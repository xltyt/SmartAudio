CURRENT_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(shell dirname $(CURRENT_PATH))

all : prep

.PHONY: prep

# apt install lrzsz git vim git-lfs build-essential cmake perl autoconf libtool pip python-is-python3 gcc-aarch64-linux-gnu g++-aarch64-linux-gnu gfortran-aarch64-linux-gnu libssl-dev tcl
# pip install setuptools PyYAML typing_extensions --index-url https://mirrors.aliyun.com/pypi/simple/
# GPU
#   cuda_12.2.0_535.54.03_linux.run
#   cudnn-linux-x86_64-8.9.7.29_cuda12-archive.tar.xz
#   libcusparse_lt-linux-x86_64-0.4.0.7-archive.tar.xz
#
prep:
	@bash ../data/model_prepare.sh
	@echo "\033[32m=====> Build amd64...\033[0m"
	make -C 3rd_party
	make -j16 -C common
	make -j16 -C search
	MODE=gpu make -j16 -C search
	make -j16 -C manager
	@echo "\033[32m=====> Build arm...\033[0m"
	PLATFORM=arm make -C 3rd_party
	PLATFORM=arm make -j16 -C common
	PLATFORM=arm make -j16 -C search
	PLATFORM=arm make -j16 -C manager
	@echo "\033[32m=====> Build Done\033[0m"

dist:
	make dist -C search

clean:
	make clean -C common
	make clean -C search
	MODE=gpu make clean -C search
	make clean -C manager
	PLATFORM=arm make clean -C common
	PLATFORM=arm make clean -C search
	PLATFORM=arm make clean -C manager

distclean:
	make clean
	make clean -C 3rd_party
	PLATFORM=arm make clean -C 3rd_party
