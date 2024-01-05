# Chcount - CLI

Command line interface application for counting the occurencies of character in a file.
Uses concurrency to speedup the counting process.

## Install dependencies

To install dependencies look [here](../README.md#install-dependencies)

## How to build

Open terminal and follow steps

```bash
cd path/to/chcount_project/cli
mkdir build
cd build
cmake ..
make -j
```

## Usage

```bash
chcount -c 'I' -f path/to/counting/file
```

All options

```bash
Options:
  --help                  Help message
  -c [ --character ] arg  Character which we count
  -f [ --input-file ] arg Path to an input file
```
