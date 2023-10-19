NOTPARALLEL:

.DEFAULT_GOAL := build

SHELL := /bin/bash

MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
MKFILE_DIR := $(dir $(MKFILE_PATH))
REPO_PATH=$(MKFILE_DIR)/..
PARALLEL_COUNT=$(shell nproc)

RLBOX_WASM2C_PATH=$(REPO_PATH)/../rlbox_wasm2c_sandbox

WASM2C_SRC_PATH=$(REPO_PATH)/../wabt

CC=clang
CXX=clang++ -std=c++17

WASI_SDK_PATH=$(REPO_PATH)/../wasi-sdk
WASI_CC=$(WASI_SDK_PATH)/bin/clang --sysroot $(WASI_SDK_PATH)/share/wasi-sysroot/
WASI_CXX=$(WASI_SDK_PATH)/bin/clang++ --sysroot $(WASI_SDK_PATH)/share/wasi-sysroot/ -std=c++17
WASI_AR=$(WASI_SDK_PATH)/bin/ar
WASI_LD=$(WASI_SDK_PATH)/bin/wasm-ld
WASI_CFLAGS=-I $(MKFILE_DIR)/wasmcompat -D_WASI_EMULATED_PROCESS_CLOCKS
WASI_LDFLAGS=-Wl,--stack-first -Wl,-z,stack-size=1048576 -lwasi-emulated-process-clocks

WASM2C_BUILD_INCLUDES=-I $(WASM2C_SRC_PATH)/wasm2c -I $(REPO_PATH)/benchmark -I $(WASM2C_SRC_PATH)/third_party/uvwasi/include/ -DWASM_RT_MEMCHECK_GUARD_PAGES=1 -DWASM_RT_USE_MMAP=1 -DWASM_RT_SKIP_SIGNAL_RECOVERY=1 -DWASM_RT_USE_STACK_DEPTH_COUNT=0
WASM2C_RUNTIME=$(WASM2C_SRC_PATH)/wasm2c/wasm-rt-impl.c $(REPO_PATH)/benchmark/uvwasi-rt.c
UVWASI_LIB=$(WASM2C_SRC_PATH)/build/third_party/uvwasi/libuvwasi_a.a $(WASM2C_SRC_PATH)/build/_deps/libuv-build/libuv_a.a

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

$(WASM2C_SRC_PATH)/build/wasm2c:
	cmake -S $(WASM2C_SRC_PATH)/ -B $(WASM2C_SRC_PATH)/build -DCMAKE_BUILD_TYPE=Release -DWITH_WASI=ON
	cd $(WASM2C_SRC_PATH)/build && make -j$(PARALLEL_COUNT)

#####################################

../build_wasm_$(BUILD_TYPE_DIR)/lib/libzstd.a: $(WASM2C_SRC_PATH)/build/wasm2c
	CC="$(WASI_CC)" CFLAGS="$(WASI_CFLAGS)" LDFLAGS="$(WASI_LDFLAGS)" \
		CXX="$(WASI_CXX)" CXXFLAGS="$(WASI_CFLAGS)" LDFLAGS="$(WASI_LDFLAGS)" \
		AR="$(WASI_AR)" LD="$(WASI_LD)" \
	cmake -S ../build/cmake -B ../build_wasm_$(BUILD_TYPE_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE_CONFIG) -DZSTD_FORCE_DISABLE_ASM=ON -DZSTD_MULTITHREAD_SUPPORT=OFF
	cd $(shell dirname "$@") && make -j$(PARALLEL_COUNT) libzstd_static

#####################################

../build_wasm_$(BUILD_TYPE_DIR)/lib/simple_compression.wasm.c: simple_compression.c
	$(WASI_CC) $(BUILD_TYPE_FLAGS) $(WASI_CFLAGS) $(WASI_LDFLAGS) -o ../build_wasm_$(BUILD_TYPE_DIR)/lib/simple_compression.wasm simple_compression.c $(EXAMPLE_INCLUDES) -Wl,--whole-archive ../build_wasm_$(BUILD_TYPE_DIR)/lib/libzstd.a -Wl,--no-whole-archive
	$(WASM2C_SRC_PATH)/build/wasm2c ../build_wasm_$(BUILD_TYPE_DIR)/lib/simple_compression.wasm -o ../build_wasm_$(BUILD_TYPE_DIR)/lib/simple_compression.wasm.c

../build_wasm_$(BUILD_TYPE_DIR)/simple_compression: wasm-rt-static-runner-simple-compression.c ../build_wasm_$(BUILD_TYPE_DIR)/lib/simple_compression.wasm.c
	$(CC) $(BUILD_TYPE_FLAGS) $(BUILD_BINARY_FLAGS) -o $@ $(WASM2C_RUNTIME) wasm-rt-static-runner-simple-compression.c ../build_wasm_$(BUILD_TYPE_DIR)/lib/simple_compression.wasm.c $(WASM2C_BUILD_INCLUDES) -I  ../build_wasm_$(BUILD_TYPE_DIR)/lib/ $(UVWASI_LIB)

../build_wasm_$(BUILD_TYPE_DIR)/lib/simple_decompression.wasm.c: simple_decompression.c
	$(WASI_CC) $(BUILD_TYPE_FLAGS) $(WASI_CFLAGS) $(WASI_LDFLAGS) -o ../build_wasm_$(BUILD_TYPE_DIR)/lib/simple_decompression.wasm simple_decompression.c $(EXAMPLE_INCLUDES) -Wl,--whole-archive ../build_wasm_$(BUILD_TYPE_DIR)/lib/libzstd.a -Wl,--no-whole-archive
	$(WASM2C_SRC_PATH)/build/wasm2c ../build_wasm_$(BUILD_TYPE_DIR)/lib/simple_decompression.wasm -o ../build_wasm_$(BUILD_TYPE_DIR)/lib/simple_decompression.wasm.c

../build_wasm_$(BUILD_TYPE_DIR)/simple_decompression: wasm-rt-static-runner-simple-decompression.c ../build_wasm_$(BUILD_TYPE_DIR)/lib/simple_decompression.wasm.c
	$(CC) $(BUILD_TYPE_FLAGS) $(BUILD_BINARY_FLAGS) -o $@ $(WASM2C_RUNTIME) wasm-rt-static-runner-simple-decompression.c ../build_wasm_$(BUILD_TYPE_DIR)/lib/simple_decompression.wasm.c $(WASM2C_BUILD_INCLUDES) -I  ../build_wasm_$(BUILD_TYPE_DIR)/lib/ $(UVWASI_LIB)

#####################################

ubuntu-22.04.3-desktop-amd64.iso:
	wget https://www.releases.ubuntu.com/jammy/ubuntu-22.04.3-desktop-amd64.iso

build: ubuntu-22.04.3-desktop-amd64.iso
build: ../build_$(BUILD_TYPE_DIR)/simple_compression
build: ../build_$(BUILD_TYPE_DIR)/simple_decompression
build: ../build_conly_$(BUILD_TYPE_DIR)/simple_compression
build: ../build_conly_$(BUILD_TYPE_DIR)/simple_decompression
build: ../build_wasm_$(BUILD_TYPE_DIR)/simple_compression
build: ../build_wasm_$(BUILD_TYPE_DIR)/simple_decompression

test:
	sleep 1
	../build_$(BUILD_TYPE_DIR)/simple_compression ./ubuntu-22.04.3-desktop-amd64.iso
	sleep 1
	../build_$(BUILD_TYPE_DIR)/simple_decompression ./ubuntu-22.04.3-desktop-amd64.iso.zst
	rm ./ubuntu-22.04.3-desktop-amd64.iso.zst
	sleep 1
	../build_conly_$(BUILD_TYPE_DIR)/simple_compression ./ubuntu-22.04.3-desktop-amd64.iso
	sleep 1
	../build_conly_$(BUILD_TYPE_DIR)/simple_decompression ./ubuntu-22.04.3-desktop-amd64.iso.zst
	rm ./ubuntu-22.04.3-desktop-amd64.iso.zst
	sleep 1
	../build_wasm_$(BUILD_TYPE_DIR)/simple_compression ./ubuntu-22.04.3-desktop-amd64.iso
	sleep 1
	../build_wasm_$(BUILD_TYPE_DIR)/simple_decompression ./ubuntu-22.04.3-desktop-amd64.iso.zst