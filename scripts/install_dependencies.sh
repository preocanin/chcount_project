#!/usr/bin/bash

function log_info() {
    echo "INFO: $1"
}

function log_error() {
    echo "ERROR: $1"
    exit 1
}

if command -v pacman &> /dev/null
then
    PREREQUISITES=("cmake" "boost")

    log_info "Using 'pacman' to install prerequisites"

    pacman -Qi ${PREREQUISITES[@]} &> /dev/null

    if [ $? -eq 0 ]; then
        log_info "Prerequisites are already installed"
    else
        sudo pacman -S --needed ${PREREQUISITES[@]}
    fi
elif command -v apt &> /dev/null
then
    PREREQUISITES=("g++" "cmake" "libboost-all-dev" "clang-format")

    log_info "Using 'apt' to install prerequisites"

    sudo apt upgrade
    sudo apt update 
    suto apt install ${PREREQUISITES[@]}
else
    log_error "Cannot install prerequisites"
fi
