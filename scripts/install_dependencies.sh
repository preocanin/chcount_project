#!/usr/bin/bash

function log_info() {
    echo "INFO: $1"
}

function log_error() {
    echo "ERROR: $1"
    exit 1
}

function install_if_not_exists() {
    pacman -Qi $2 &>/dev/null

    PACKAGE_EXISTS=$?

    if (! command -v systemctl status $2 &> /dev/null) &&\
       (! command -v $2 &> /dev/null) &&\
       ($PACKAGE_EXISTS -neq 0)
    then
       sudo $1 $2
    else
       log_info "'$2' already installed"
    fi
}

function pacman_install_if_not_exists() {
    install_if_not_exists "pacman -S" $1
}

PREREQUISITES=("cmake" "boost")

if command -v pacman &> /dev/null
then
    log_info "Using 'pacman' to install prerequisites"

    pacman -Qi ${PREREQUISITES[@]} &> /dev/null

    if [ $? -eq 0 ]; then
        log_info "Prerequisites are already installed"
    else
        sudo pacman -S --needed ${PREREQUISITES[@]}
    fi
else
    log_error "Cannot install prerequisites"
fi
