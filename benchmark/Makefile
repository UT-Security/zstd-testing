NOTPARALLEL:

.DEFAULT_GOAL := build

SHELL := /bin/bash

MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
MKFILE_DIR := $(dir $(MKFILE_PATH))
REPO_PATH=$(MKFILE_DIR)/..
PARALLEL_COUNT=$(shell nproc)

CC=clang

EXAMPLE_INCLUDES=-I$(REPO_PATH)/examples -I$(REPO_PATH)/lib/

ifdef DEBUG
BUILD_TYPE_DIR=debug
BUILD_TYPE_CONFIG=Debug
BUILD_TYPE_FLAGS=-O0 -g -DWASM_RT_NOINLINE
BUILD_BINARY_FLAGS=-fsanitize=address
else
BUILD_TYPE_DIR=release
BUILD_TYPE_CONFIG=Release
BUILD_TYPE_FLAGS=-O3
BUILD_BINARY_FLAGS=
endif

../build_$(BUILD_TYPE_DIR)/lib/libzstd.a:
	cmake -S ../build/cmake -B ../build_$(BUILD_TYPE_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE_CONFIG)
	cd $(shell dirname "$@") && make -j$(PARALLEL_COUNT) libzstd_static

../build_$(BUILD_TYPE_DIR)/simple_compression: ./simple_compression.c ../build_$(BUILD_TYPE_DIR)/lib/libzstd.a
	$(CC) $(BUILD_TYPE_FLAGS) -o $@ ./simple_compression.c $(EXAMPLE_INCLUDES) ../build_$(BUILD_TYPE_DIR)/lib/libzstd.a

../build_$(BUILD_TYPE_DIR)/simple_decompression: ./simple_decompression.c ../build_$(BUILD_TYPE_DIR)/lib/libzstd.a
	$(CC) $(BUILD_TYPE_FLAGS) -o $@ ./simple_decompression.c $(EXAMPLE_INCLUDES) ../build_$(BUILD_TYPE_DIR)/lib/libzstd.a

#####################################

../build_conly_$(BUILD_TYPE_DIR)/lib/libzstd.a:
	cmake -S ../build/cmake -B ../build_conly_$(BUILD_TYPE_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE_CONFIG) -DZSTD_FORCE_DISABLE_ASM=ON
	cd $(shell dirname "$@") && make -j$(PARALLEL_COUNT) libzstd_static

../build_conly_$(BUILD_TYPE_DIR)/simple_compression: ./simple_compression.c ../build_conly_$(BUILD_TYPE_DIR)/lib/libzstd.a
	$(CC) $(BUILD_TYPE_FLAGS) -o $@ ./simple_compression.c $(EXAMPLE_INCLUDES) ../build_conly_$(BUILD_TYPE_DIR)/lib/libzstd.a

../build_conly_$(BUILD_TYPE_DIR)/simple_decompression: ./simple_decompression.c ../build_conly_$(BUILD_TYPE_DIR)/lib/libzstd.a
	$(CC) $(BUILD_TYPE_FLAGS) -o $@ ./simple_decompression.c $(EXAMPLE_INCLUDES) ../build_conly_$(BUILD_TYPE_DIR)/lib/libzstd.a

#####################################

ubuntu-22.04.3-desktop-amd64.iso:
	wget https://www.releases.ubuntu.com/jammy/ubuntu-22.04.3-desktop-amd64.iso

build: ubuntu-22.04.3-desktop-amd64.iso
build: ../build_$(BUILD_TYPE_DIR)/simple_compression
build: ../build_$(BUILD_TYPE_DIR)/simple_decompression
build: ../build_conly_$(BUILD_TYPE_DIR)/simple_compression
build: ../build_conly_$(BUILD_TYPE_DIR)/simple_decompression

test:
	../build_$(BUILD_TYPE_DIR)/simple_compression ./ubuntu-22.04.3-desktop-amd64.iso
	../build_$(BUILD_TYPE_DIR)/simple_decompression ./ubuntu-22.04.3-desktop-amd64.iso.zst
	../build_conly_$(BUILD_TYPE_DIR)/simple_compression ./ubuntu-22.04.3-desktop-amd64.iso
	../build_conly_$(BUILD_TYPE_DIR)/simple_decompression ./ubuntu-22.04.3-desktop-amd64.iso.zst
