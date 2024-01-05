#!/usr/bin/bash

./scripts/build.sh

mkdir build 2>/dev/null
cp cli/build/chcount build/
cp backend/build/chcount_server build/
mkdir build/www 2>/dev/null
cp frontend/*.html build/www
cp -r frontend/js build/www
