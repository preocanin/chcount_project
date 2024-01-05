# Utility script

## install_dependencies.sh

Install needed dependencies using `pacman` or `apt` package manager.

## build.sh

Builds cli and backend applications.

### Usage

```bash
cd path/to/chcount_project
./script/build.sh
```

## deploy.sh

Builds and copies all applications in one folder `build` in the root of directory.

### Usage

```bash
cd path/to/chcount_project
./script/deploy.sh
cd build # All files are located here
```
