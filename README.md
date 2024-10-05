
# posix-threaded-mapreduce

The **posix-threaded-mapreduce** is a C-based implementation designed to handle large-scale data processing tasks in parallel. By leveraging POSIX threads, this library distributes computational workloads efficiently, significantly improving performance on multi-core systems. It implements the core principles of the MapReduce model—splitting, mapping, shuffling, and reducing—allowing developers to process vast datasets seamlessly.

  

##  Key Features

-  **Parallel Processing**: Utilizes multiple threads to divide tasks, maximizing CPU core usage for efficient computation.

-  **Custom Key-Value Pair Handling**: Implements data structures to handle key-value pairs, allowing for easy data manipulation.

-  **Thread-Safe Operations**: Ensures safe concurrent data access using synchronization mechanisms.

-  **Scalable Architecture**: Supports flexible configuration of mapper and reducer threads, allowing the system to scale with dataset size.

-  **Memory Management**: No memory leaks, verified using Valgrind for robust performance in long-running processes.

  

##  Architecture

The library follows the MapReduce model:

1.  **Split Phase**: The input data is divided into smaller chunks to be processed in parallel.

2.  **Map Phase**: Multiple threads run the **Map** function, processing the split data to produce intermediate key-value pairs.

3.  **Shuffle Phase**: Intermediate pairs are grouped by key for further reduction.

4.  **Reduce Phase**: The **Reduce** function aggregates data associated with each key, running across multiple threads.

5.  **Output**: Final results are consolidated into an output list.

  

##  Installation

To use this library, you will need to have `make` and `gcc` installed. You can clone the repository and compile the source code using the following commands:

  

```bash

git  clone git@github.com:vaishi01/posix-threaded-mapreduce.git

cd  posix-threaded-mapreduce
make

```

  

## Usage

The library can be tested with a provided **word-count** program, which counts the occurrences of each word in a set of text files.

  

### Example Usage:

1.  **Compile the word-count program**:

```bash

make word-count

```

  

2.  **Run the word-count program**:

```bash

./word-count <NUM_MAPPER> <NUM_REDUCER> <FILE1> <FILE2> ...

```

  

Example:

```bash

./word-count 4 2 sample.txt

```

  

This command runs the word-count program with 4 mapper threads and 2 reducer threads on `sample.txt`.

  

## Components

  

### 1. `kvlist.h`

This file implements the **key-value list** data structure used to store key-value pairs throughout the MapReduce process. It includes:

-  `kvpair_t`: Represents a key-value pair.

-  `kvlist_t`: A singly-linked list of key-value pairs.

- Functions for creating, appending, sorting, and printing key-value lists.

  

### 2. `mr.c`

This is the core of the library, where the **map_reduce** function is implemented. The main responsibilities of this function include:

- Spawning mapper and reducer threads.

- Managing the input and output data.

- Synchronizing thread operations.

  

### 3. `hash.h`

Contains a simple hash function to distribute key-value pairs among reducers.

  

### 4. `mapper_t` and `reducer_t`

-  **mapper_t**: Defines the map function, which processes each input key-value pair to produce intermediate pairs.

-  **reducer_t**: Defines the reduce function, which aggregates intermediate key-value pairs by their keys.

  

## Example: Word-Counting

The word-counting program is a classic MapReduce application that counts the number of occurrences of each word in a set of text files. The process is as follows:

-  **Mapper**: Splits the content of each file into words and emits key-value pairs (`word, 1`).

-  **Reducer**: Sums up the occurrences of each word.

  

## Performance Considerations

This library is optimized for performance:

- The use of POSIX threads allows for parallel processing of large datasets.

- The shuffle and reduce phases are designed to minimize contention and maximize throughput, ensuring scalability.

  

## License

This project is licensed under the MIT License.
