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
    PREREQUISITES=("g++" "cmake" "clang-format" "build-essential" "python3-dev" "autotools-dev" "libicu-dev" "libbz2-dev")

    log_info "Using 'apt' to install prerequisites"

    sudo apt upgrade
    sudo apt update 
    sudo apt install ${PREREQUISITES[@]}

    BOOST_VERSION="boost_1_84_0"

    wget https://archives.boost.io/release/1.84.0/source/${BOOST_VERSION}.tar.gz
    tar -xvf ${BOOST_VERSION}.tar.gz
    cd ${BOOST_VERSION}/
    sudo ./bootstrap.sh --prefix=/usr/
    ./b2
    sudo ./b2 install
    cd ..
    rm ${BOOST_VERSION}.tar.gz
    sudo rm -r ${BOOST_VERSION}/

else
    log_error "Cannot install prerequisites"
fi
