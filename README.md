HashStore Benchmark
===================
Benchmark for different solutions to search for binary hashes in hamming distance.

Features
--------

- Sequential CPU search
- Sequential GPU search
- Multi Index Hashing

Installation
------------

You first need to clone the project on your computer, and build it with one of the following commands. You might need to change the GPU architecture in CMakeLists.txt, change the value of the option `--gpu-architecture=sm_50`.

```bash
# Create a directory for binaries
$ mkdir build && cd build

# Build the project with CMake
$ cmake -DCMAKE_BUILD_TYPE=Release ..
$ make
```
The executables are then located in the same directory.
Usage
-----

```bash
# Generate a random hash file (16384 hashes)
$ ./HashGenerator 16384 > hashes

# Sequential CPU search
$ ./HashSearch cpu hashes > search_cpu

# Sequential GPU search
$ ./HashSearch gpu hashes > search_gpu

# MIH search
$ ./HashSearch mih hashes > search_mih

# Check the difference between two executions
diff search_cpu search_mih
```

License
-------
See the LICENSE file.