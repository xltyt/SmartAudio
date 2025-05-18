CURRENT_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(shell dirname $(CURRENT_PATH))

THIRD_DIR := $(CURRENT_DIR)/../../local

all : prep

.PHONY: prep

PLATFORM ?= amd64

prep:
	@mkdir -p ../../$(PLATFORM)/bin
	@mkdir -p ../../$(PLATFORM)/lib
	@if [ ! -f ../../$(PLATFORM)/local/bin/cmake ]; then                  cd cmake && PLATFORM=$(PLATFORM) sh build.sh; fi
	@if [ ! -d ../../$(PLATFORM)/local/include/openssl ]; then            cd openssl && PLATFORM=$(PLATFORM) sh build.sh; fi
	@if [ ! -d ../../$(PLATFORM)/local/include/rapidjson ]; then          cd rapidjson && PLATFORM=$(PLATFORM) sh build.sh; fi
	@if [ ! -f ../../$(PLATFORM)/local/include/zlib.h ]; then             cd zlib && PLATFORM=$(PLATFORM) sh build.sh; fi
	@if [ ! -d ../../$(PLATFORM)/local/include/unicode ]; then            cd icu && PLATFORM=$(PLATFORM) sh build.sh; fi
	@if [ ! -d ../../$(PLATFORM)/local/include/curl ]; then               cd curl && PLATFORM=$(PLATFORM) sh build.sh; fi
	@if [ ! -f ../../$(PLATFORM)/local/include/pcre2.h ]; then            cd pcre && PLATFORM=$(PLATFORM) sh build.sh; fi
	@if [ ! -d ../../$(PLATFORM)/local/include/boost ]; then              cd boost && PLATFORM=$(PLATFORM) sh build.sh; fi
	@if [ ! -d ../../$(PLATFORM)/local/include/gtest ]; then              cd gtest && PLATFORM=$(PLATFORM) sh build.sh; fi
	@if [ ! -d ../../$(PLATFORM)/local/include/glog ]; then               cd glog && PLATFORM=$(PLATFORM) sh build.sh; fi
	@if [ ! -d ../../$(PLATFORM)/local/include/google/protobuf ]; then    cd protobuf && PLATFORM=$(PLATFORM) sh build.sh; fi
	@if [ ! -d ../../$(PLATFORM)/local/include/brpc ]; then               cd brpc && PLATFORM=$(PLATFORM) bash build.sh; fi
	@if [ ! -d ../../$(PLATFORM)/local/include/openblas ]; then           cd blas && PLATFORM=$(PLATFORM) bash build.sh; fi
	@if [ ! -d ../../$(PLATFORM)/local/torch/cpu/include ]; then          cd torch && PLATFORM=$(PLATFORM) bash build.sh; fi
	@if [ ! -d ../../$(PLATFORM)/local/torchaudio/cpu/include ]; then     cd torchaudio && PLATFORM=$(PLATFORM) bash build.sh; fi
	@if [ ! -f ../../$(PLATFORM)/local/bin/ffmpeg ]; then                 cd ffmpeg && PLATFORM=$(PLATFORM) bash build.sh; fi
	@if [ ! -f ../../$(PLATFORM)/local/include/sndfile.h ]; then          cd audio && PLATFORM=$(PLATFORM) bash build.sh; fi
	@if [ ! -d ../../$(PLATFORM)/local/include/eigen3 ]; then             cd eigen && PLATFORM=$(PLATFORM) bash build.sh; fi
	@if [ ! -d ../../$(PLATFORM)/local/onnx/cpu/include ]; then           cd onnx && PLATFORM=$(PLATFORM) bash build.sh; fi
	##@rm -rf ../../build

clean:
	rm -rf ../../$(PLATFORM)/local
