# Points 2D compression

## Contents

*    [Introduction](#introduction)
     *    [Varint details](#varint-details)
*    [Formal task](#formal-task)
*    [Algorithm description](#algorithm-description)
*    [Limitations](#limitations)
*    [Implementation notes](#implementation-notes)
*    [Implementation results](#implementation-results)
*    [Project build and run](#project-build-and-run)
     *    [Prerequisites](#prerequisites)
     *    [How to build project](#how-to-build-project)
     *    [Run project tests after build](#run-project-tests-after-build)
     *    [C++ syntax check](#c-syntax-check)
     

## Introduction
This repository is a c++ simple implementation of
Bits interleaving + Delta encoding + Variable-Length Integer (Varint) encoding.
It can be used for large-scale points 2D cloud compact 
storage (```pack``` and ```unpack``` operations).

Bit interleaving idea can be represented:

```
ABCD + EFGH -> AEBFCGDH
```

This simple bit interleaving scheme allows us to convert 2D, 3D, ... dimensions
into 1D dimension. More advanced schemes (like zigzag) can preserve
better locality, but decrease performance.

Varints represent integers using a dynamic number of bytes. 
The most common format is LEB128 (Little Endian Base 128), 
used in Google Protocol Buffers and LLVM. 

Mechanism: Each byte uses the first 7 bits for data 
and the 8th bit (MSB) as a "continuation bit".
Continuation Bit: If set (1), another byte follows; 
if clear (0), this is the last byte. 

### Varint details
The [Little Endian Base-128](https://en.wikipedia.org/wiki/LEB128) encoding
is used in many projects where you need to compress the 
relatively large set of integer values with a very wide range of
bit lengths: it can be a lot of very short integers and very
large integers in opposition. To keep this data in the most compact
way, the idea of a "continuation bit" is used here.

## Formal task

```std::vector<std::pair<int, int>>``` vector on input: vector of 2d points
with integer coordinates.

The Compressor can pack this input data into compact storage
and restore the original points vector.

class Compressor should have methods:
```bool pack(const std::vector<std:pair<int, int>>& vec)```
into internal compressed presentation.

```std::vector<std:pair<int, int>> unpack() const```
should restore original points vector.

## Algorithm description

1. Convert each (x,y) coordinate pair into a single 32-bit code value
using a bit interleaving technique.
2. Sort codes array.
3. Create an array with delta of code values.
4. Encode delta values using the variable bit length technique.

## Limitations

1. Input point coordinates should be non-negative
2. There is no check on possible coordinate values range:
they should be in [0..65535]

## Implementation notes
The first bit is used as a "continuation bit".
Data layout in this implementation can be
represented in the following diagram:

```
LSB-----------------------------MSB
1xxxxxxx                             7  bits in 1 byte
0xxxxxxx 1xxxxxxx                    14 bits in 2 bytes
0xxxxxxx 0xxxxxxx 1xxxxxxx           21 bits in 3 bytes
0xxxxxxx 0xxxxxxx 0xxxxxxx 1xxxxxxx  28 bits in 4 bytes
```

Here, LSB - least significant bit,
MSB - most significant bit.
This data layout allows us to implement any integer value
compression.
For the practical task, we can limit source data with 32 bits
or 64 bits and use a more compact
[Binary encoding layout](https://news.ycombinator.com/item?id=11263378).
Alternative bit layout schemes can increase decoding performance.
Other possible performance improvement idea:
split source integer value into (6+6+6+...) or (5+5+5+...) chunks
unlike (7+7+7+...) chunks like implemented here.

Due to merged coordinate value sorting, the original order of points
is not preserved after ```unpack```. This algorithm should not be
used in the scenarios where constant points order is important.

Please note, this implementation is not **part of production code**
and can be used only as a **reference** or **starting point** for
your own projects.

## Implementation results

Random tests show 
- 12% (values in range[0..1023]) 
- 35% (values
in range [0..32767]) 
- 36% (values in range[0..65535]) 
data size compression ratio for dense point clouds depending on values range,
density, test data size, etc.

Important: a smaller size of the test points vector 
leads to a worse compression ratio.

There is no bunch of tests covering different point distrubution
scenarios, like: several clusters with pseudo-random points placed
around cluster centers.

There is no temporary memory optimization on the ```pack``` stage.
There is no low-level instruction optimization for known
target hardware platform.

Performance results, measured on Windows 11, 
cpu 12th Gen Intel(R) Core(TM) i7-12700H (2.30 GHz)
with 16 gb RAM

Average performance for (1 * 1000 * 1000) points (GNU compiler, Release build):
- pack: 63 milliseconds
- unpack: 20 milliseconds

## Project build and run

This project was developed under Windows with
MSVC and GNU compiler. Hopefully, it can be easily 
ported under Linux/other platforms.

### Prerequisites
1. cmake version 4.2.3 (required for build)
2. Microsoft (R) C/C++ Optimizing Compiler Version 19.16.27054 (or later)
3. GNU g++.exe (Rev11, Built by MSYS2 project) 15.2.0 (alternative to msvc)
4. clang version 22.1.0 (alternative to msvc), here clang-tidy utility.

### How to build project

Build project:
```
rmdir build
mkdir build
cd build
cmake ..
cmake --build . 
cd ..
```

### Run project tests after build
The following commands allow to run test 
application under Windows using MSVC/GNU compiler
```
echo off
cd build/Debug
echo on

varint.exe

echo off
cd ../..
echo on
```

After succseefull build & run you should see in output stream:
```
minimal test completed.
pack time for million points =  61.333 milliseconds
unpack time for million points =  19.417 milliseconds
compression ratio =  36.9%
random test completed.
```

### C++ syntax check

run:
```
clang-tidy src/main.cpp
clang-tidy src/compressor.cpp
clang-tidy src/duration_meter.cpp
```
