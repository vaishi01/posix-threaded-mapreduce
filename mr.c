#define _POSIX_C_SOURCE 200809L
#include "mr.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash.h"
#include "kvlist.h"

size_t kvlist_length(kvlist_t *list);

pthread_mutex_t *mutexes;

typedef struct {
  size_t start_index;
  size_t end_index;
  kvlist_t *input;
  kvlist_t **intermediates;
  mapper_t mapper;
  size_t num_reducers;
} MapperThreadArg;

typedef struct {
  kvlist_t *input;
  kvlist_t *output;
  reducer_t reducer;
} ReducerThreadArg;

void *mapper_thread(void *arg) {
  MapperThreadArg *data = (MapperThreadArg *)arg;
  kvlist_iterator_t *it = kvlist_iterator_new(data->input);
  kvpair_t *pair;
  size_t index = 0;

  kvlist_t *local_output = kvlist_new();

  while ((pair = kvlist_iterator_next(it)) != NULL) {
    if (index >= data->start_index && index < data->end_index) {
      data->mapper(pair, local_output);
    }
    index++;
  }
  kvlist_iterator_free(&it);

  for (size_t i = 0; i < data->num_reducers; i++) {
    pthread_mutex_lock(&mutexes[i]);
    kvlist_extend(data->intermediates[i], local_output);
    pthread_mutex_unlock(&mutexes[i]);
  }

  kvlist_free(&local_output);
  return NULL;
}

void *reducer_thread(void *arg) {
  ReducerThreadArg *data = (ReducerThreadArg *)arg;
  kvlist_sort(data->input);

  kvlist_iterator_t *itor = kvlist_iterator_new(data->input);
  kvpair_t *pair;
  kvlist_t *temp_list = kvlist_new();
  char *current_key = NULL;

  while ((pair = kvlist_iterator_next(itor)) != NULL) {
    if (current_key == NULL || strcmp(pair->key, current_key) != 0) {
      if (current_key != NULL) {
        data->reducer(current_key, temp_list, data->output);
        kvlist_free(&temp_list);
        temp_list = kvlist_new();
      }
      free(current_key);
      current_key = strdup(pair->key);
    }
    kvlist_append(temp_list, kvpair_clone(pair));
  }

  if (current_key != NULL) {
    data->reducer(current_key, temp_list, data->output);
    free(current_key);
    // kvlist_free(&temp_list);
  }
  kvlist_free(&temp_list);
  temp_list = NULL;
  kvlist_iterator_free(&itor);
  return NULL;
}

void map_reduce(mapper_t mapper, size_t num_mapper, reducer_t reducer,
                size_t num_reducer, kvlist_t *input, kvlist_t *output) {
  pthread_t mapper_threads[num_mapper];
  pthread_t reducer_threads[num_reducer];
  MapperThreadArg mapper_args[num_mapper];
  ReducerThreadArg reducer_args[num_reducer];
  kvlist_t *intermediates[num_reducer];
  mutexes = malloc(num_reducer * sizeof(pthread_mutex_t));

  for (size_t i = 0; i < num_reducer; i++) {
    pthread_mutex_init(&mutexes[i], NULL);
    intermediates[i] = kvlist_new();
  }

  size_t total_items = kvlist_length(input);
  size_t slice_size = total_items / num_mapper;
  size_t remaining_items = total_items % num_mapper;

  size_t current_start_index = 0;
  for (size_t i = 0; i < num_mapper; i++) {
    size_t items_to_process = slice_size + (remaining_items > 0 ? 1 : 0);
    if (remaining_items > 0) remaining_items--;

    mapper_args[i].start_index = current_start_index;
    mapper_args[i].end_index = current_start_index + items_to_process;
    mapper_args[i].input = input;
    mapper_args[i].intermediates = intermediates;
    mapper_args[i].mapper = mapper;
    mapper_args[i].num_reducers = num_reducer;
    pthread_create(&mapper_threads[i], NULL, mapper_thread, &mapper_args[i]);
    current_start_index += items_to_process;
  }

  for (size_t i = 0; i < num_mapper; i++) {
    pthread_join(mapper_threads[i], NULL);
  }

  for (size_t i = 0; i < num_reducer; i++) {
    reducer_args[i].input = intermediates[i];
    reducer_args[i].output = output;
    reducer_args[i].reducer = reducer;
    pthread_create(&reducer_threads[i], NULL, reducer_thread, &reducer_args[i]);
  }

  for (size_t i = 0; i < num_reducer; i++) {
    pthread_join(reducer_threads[i], NULL);
    kvlist_free(&intermediates[i]);
    pthread_mutex_destroy(&mutexes[i]);
  }

  free(mutexes);
}

size_t kvlist_length(kvlist_t *list) {
  size_t length = 0;
  kvlist_iterator_t *it = kvlist_iterator_new(list);
  while (kvlist_iterator_next(it) != NULL) {
    length++;
  }
  kvlist_iterator_free(&it);
  return length;
}
