#! /bin/sh

PREFIX=$1

export CROSS_COMPILE=arm-none-linux-gnueabi-
export SPEC_TARGET=linux-arm-g++
export PATH=$PREFIX/opt/FriendlyARM/toolschain/4.4.3/bin:$PATH
export ROADMASTER_TARGET=MINI2440
