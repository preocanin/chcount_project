# chcount_project

chcount_project is an example project for character counting service.

Project consists of multiple parts:

1. [CLI Application](./cli/) which counts occurencies of specific character in a given file.
2. [Server Application](./backend/) which servers frontend page and provides API for requesting the character counting
3. [Frontend SPA](./frontend/) which provides simple interface for the server usage
4. [Utility scripts](./scripts/)

## Install dependencies

```bash
cd path/to/repository/clone/chcount_project
./scripts/install_dependencies.sh
```

`NOTE:` Script currently only support installing dependencies with `pacman` and `apt` package manager.

## Deploy

```bash
cd path/to/repository/clone/chcount_project
./scripts/deploy.sh
cp -r build/* to/custom/location
```

## Future improvements

- Add tests
- Add support for uploading file
- Add support for uploading multiple files
- Make frontend prettier
