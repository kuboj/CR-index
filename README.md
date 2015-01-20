# CR-index
Compressed read index - bioinf data structure for fast querying substring in reads.

## Installation

### Dependencies

  * [SGA](https://github.com/jts/sga)
    * [Google sparse hash library](https://code.google.com/p/sparsehash/)
    * [Bamtools](https://github.com/pezmaster31/bamtools)
    * [Zlib](http://www.zlib.net/) (widely available as system package)
    * [Jemalloc](http://www.canonware.com/jemalloc/download.html) (widely available as system package)
  * [FM-index](https://github.com/mpetri/FM-Index) - implementation of FM-index by Matthias Petri, although this library is bundled in RM-index
  * [Boost](http://www.boost.org/) (widely available as system package)

### Installation

`make` builds all dependencies and compiles *RM_example.cpp*
