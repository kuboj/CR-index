# CR-index
Compressed read index - bioinf data structure for fast querying substring in reads.

## Installation

### Dependencies

  * [SDSL](https://github.com/simongog/sdsl-lite)
  * [Boost](http://www.boost.org/) (widely available as system package)
  * [PStreams](http://pstreams.sourceforge.net/) (widely available as system package)

### Installation

`make examples` compiles examples and all dependencies

# TODO

  * how to measure RAM used?
    * [http://people.ksp.sk/~johnny64/bordel/referenced_memory_size.cpp]()
  * write benchmark scripts
  * implement bloom filter?
    * [https://github.com/mavam/libbf]()
    * [https://code.google.com/p/bloom/]()

# Benchmark

  * 6M reads (generated from yarrowia.fasta), read length 100, 1% of errors
    * 352K critical reads
    * superstring compress ratio 2.59
    * superstring size 231M (1.85M reads were missing)
    * RSS ~ 300MB    
