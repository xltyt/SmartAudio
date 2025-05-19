CURRENT_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(shell dirname $(CURRENT_PATH))
DATE = $(shell date "+%Y%m%d%H")

MODE ?= cpu
CUDA_VERSION ?= 12.2
PLATFORM ?= amd64

VERSION=$(shell awk -F'|' '{print $$2}' ../../README.md | sed '/^ *$$/d' | awk -F'`' '{print $$2}' | tail -1)
ifeq ($(PLATFORM),arm)
  BUILD_MODULE_NAME=voice_service_arm
else
  ifeq ($(MODE),gpu)
	BUILD_MODULE_NAME=voice_service_gpu_$(CUDA_VERSION)
  else
    BUILD_MODULE_NAME=voice_service
  endif
endif

ifeq ($(PLATFORM),arm)
	CC=aarch64-linux-gnu-gcc
	CXX=aarch64-linux-gnu-g++
	STRIP=aarch64-linux-gnu-strip
else
	STRIP=strip
endif

MAJOR=2
MINOR=3

APP_DIR := $(CURRENT_DIR)/../../$(PLATFORM)/bin
OBJS_DIR := $(CURRENT_DIR)/../../$(PLATFORM)/build/$(BUILD_MODULE_NAME)
THIRD_DIR := $(CURRENT_DIR)/../../$(PLATFORM)/local
TEST_DIR := $(CURRENT_DIR)/../../$(PLATFORM)/test
ifeq ($(PLATFORM),arm)
  THIRD_HOST_DIR := $(THIRD_DIR)/../../amd64/local
else
  THIRD_HOST_DIR := $(THIRD_DIR)
endif

ifeq ($(MODE),gpu)
  TORCH_ROOT = $(THIRD_DIR)/torch/gpu/$(CUDA_VERSION)
else
  TORCH_ROOT = $(THIRD_DIR)/torch/cpu
endif
TORCH_AUDIO_ROOT = $(THIRD_DIR)/torchaudio/cpu

ifeq ($(MODE),gpu)
  ONNX_ROOT = $(THIRD_DIR)/onnx/gpu/$(CUDA_VERSION)
else
  ONNX_ROOT = $(THIRD_DIR)/onnx/cpu
endif

CUDA_INCLUDE ?= 
ifeq ($(MODE),gpu)
	CUDA_INCLUDE = -I/usr/local/cuda-$(CUDA_VERSION)/include
endif
CUDA_LIB ?= 
ifeq ($(MODE),gpu)
	CUDA_LIB = -L/usr/local/cuda-$(CUDA_VERSION)/lib64/ -lcudart_static -lcublas_static -lcublasLt_static
	ifeq ($(CUDA_VERSION),11.3)
		CUDA_LIB += -lculibos
	else ifeq ($(CUDA_VERSION),11.4)
		CUDA_LIB += -lculibos
	else ifeq ($(CUDA_VERSION),12.4)
		CUDA_LIB += -lculibos
    endif
	CUDA_LIB += -Wl,-rpath=/usr/local/cuda-$(CUDA_VERSION)/lib64/ 
	#CUDA_LIB += /usr/local/cuda-$(CUDA_VERSION)/lib64/stubs/libnvidia-ml.so
endif
INCLUDES += \
  -I. \
  -I../common \
  -I$(OBJS_DIR) \
  -I$(TORCH_ROOT)/include/ \
  -I$(TORCH_ROOT)/include/torch/csrc/api/include/ \
  -I$(TORCH_AUDIO_ROOT)/include/ \
  -I$(ONNX_ROOT)/include/ \
  -I$(ONNX_ROOT)/include/onnxruntime \
  -I$(ONNX_ROOT)/include/onnxruntime/core/session \
  $(CUDA_INCLUDE) \
  -I$(THIRD_DIR)/include/eigen3/ \
  -I$(THIRD_DIR)/include/

ABSL_LIBS= \
  -labsl_civil_time \
  -labsl_cord \
  -labsl_cordz_functions \
  -labsl_cordz_info \
  -labsl_cordz_sample_token \
  -labsl_cordz_handle \
  -labsl_cord_internal \
  -labsl_crc_cord_state \
  -labsl_crc_cpu_detect \
  -labsl_crc32c \
  -labsl_crc_internal \
  -labsl_die_if_null \
  -labsl_exponential_biased \
  -labsl_failure_signal_handler \
  -labsl_flags_commandlineflag \
  -labsl_flags_commandlineflag_internal \
  -labsl_flags_config \
  -labsl_flags_internal \
  -labsl_flags_marshalling \
  -labsl_flags_parse \
  -labsl_flags_private_handle_accessor \
  -labsl_flags_program_name \
  -labsl_flags_reflection \
  -labsl_flags_usage \
  -labsl_flags_usage_internal \
  -labsl_graphcycles_internal \
  -labsl_leak_check \
  -labsl_log_initialize \
  -labsl_log_internal_message \
  -labsl_log_internal_check_op \
  -labsl_log_internal_conditions \
  -labsl_log_internal_format \
  -labsl_log_internal_globals \
  -labsl_log_internal_log_sink_set \
  -labsl_log_internal_nullguard \
  -labsl_log_internal_proto \
  -labsl_log_severity \
  -labsl_log_sink \
  -labsl_log_entry \
  -labsl_log_flags \
  -labsl_log_globals \
  -labsl_examine_stack \
  -labsl_periodic_sampler \
  -labsl_random_distributions \
  -labsl_random_internal_distribution_test_util \
  -labsl_random_internal_platform \
  -labsl_random_internal_randen \
  -labsl_random_internal_randen_hwaes \
  -labsl_random_internal_randen_hwaes_impl \
  -labsl_random_internal_randen_slow \
  -labsl_random_internal_seed_material \
  -labsl_random_seed_gen_exception \
  -labsl_random_seed_sequences \
  -labsl_hash \
  -labsl_raw_hash_set \
  -labsl_raw_logging_internal \
  -labsl_low_level_hash \
  -labsl_hashtablez_sampler \
  -labsl_city \
  -labsl_scoped_set_env \
  -labsl_spinlock_wait \
  -labsl_stacktrace \
  -labsl_symbolize \
  -labsl_debugging_internal \
  -labsl_status \
  -labsl_statusor \
  -labsl_strerror \
  -labsl_str_format_internal \
  -labsl_strings \
  -labsl_strings_internal \
  -labsl_string_view \
  -labsl_demangle_internal \
  -labsl_malloc_internal \
  -labsl_synchronization \
  -labsl_kernel_timeout_internal \
  -labsl_throw_delegate \
  -labsl_int128 \
  -labsl_time \
  -labsl_time_zone \
  -labsl_base
  
#-L$(TORCH_AUDIO_ROOT)/lib -ltorchaudio
TORCH_STATIC_LIB=\
  -Wl,--whole-archive \
  $(TORCH_ROOT)/lib/libtorch.a \
  $(TORCH_ROOT)/lib/libtorch_cpu.a \
  $(TORCH_ROOT)/lib/libc10.a \
  $(TORCH_ROOT)/lib/libfbgemm.a \
  $(TORCH_ROOT)/lib/libdnnl.a \
  $(TORCH_ROOT)/lib/libpytorch_qnnpack.a \
  $(TORCH_ROOT)/lib/libXNNPACK.a \
  $(TORCH_ROOT)/lib/libkineto.a \
  $(TORCH_ROOT)/lib/libsleef.a \
  $(TORCH_ROOT)/lib/libtensorpipe.a \
  $(TORCH_ROOT)/lib/libtensorpipe_uv.a \
  $(TORCH_ROOT)/lib/libasmjit.a \
  $(TORCH_ROOT)/lib/libnnpack.a \
  $(TORCH_ROOT)/lib/libcpuinfo.a \
  $(TORCH_ROOT)/lib/libpthreadpool.a \
  $(TORCH_ROOT)/lib/libgloo.a \
  $(TORCH_ROOT)/lib/libonnx.a \
  $(TORCH_ROOT)/lib/libonnx_proto.a \
  $(TORCH_ROOT)/lib/libclog.a \
  -Wl,--no-whole-archive \
  -lopenblas

BASE_LIBS += $(THIRD_DIR)/../lib/libmycommon.a
BASE_LIBS += \
  -lkaldi-native-fbank-core -lkissfft-float \
  -L$(ONNX_ROOT)/lib -lonnxruntime \
  -L$(TORCH_ROOT)/lib -ltorch_cpu -ltorch -lc10 \
  $(CUDA_LIB) \
  -L$(THIRD_DIR)/lib \
  -lsamplerate -lsndfile \
  -lboost_system -lboost_filesystem -lboost_regex \
  -lpcre2-32 -lpcre2-16 -lpcre2-8 \
  -licuuc -licuio -licudata \
  -lprotoc -lprotobuf \
  -lleveldb -llz4 -lsnappy \
  -lglog \
  -lgtest \
  -lgflags \
  -lcurl \
  -lssl -lcrypto \
  -lz \
  -lm \
  -lrt \
  -ldl \
  -lpthread

LIBS += \
  -L. \
  $(BASE_LIBS)

ifeq ($(MODE),gpu)
  CFLAGS += -DUSE_GPU=1 -DCUDA_VERSION=\"$(CUDA_VERSION)\"
endif
CFLAGS    += -O2 -g -std=c++17 -fopenmp -static-libstdc++ -static-libgcc
ifeq ($(PLATFORM),amd64)
  CFLAGS += -mavx -mavx2
endif
CFLAGS	  += -DNDEBUG -DFORTIFY_SOURCE=2 -fPIC
CFLAGS	  += -Wno-deprecated-declarations
#CFLAGS    += -DMKL_ILP64 -m64 -I${MKLROOT}/include
LIBPATHS  +=
CPPFLAGS  = $(CFLAGS) -std=c++17
CXXFLAGS  = $(CFLAGS) -std=c++17

SOURCES += \
  frontend_utils.cpp \
  frontend_audio_utils.cpp \
  inference_zero_shot.cpp \
  frontend.cpp \
  audio_utils.cpp \
  tiktoken.cpp \
  whisper_token.cpp \
  core_bpe.cpp \
  flags.cpp \
  main.cpp
OBJS := $(addprefix $(OBJS_DIR)/,$(patsubst %.cpp,%.o,$(SOURCES)))
OBJS := $(patsubst %.c,%.o,$(OBJS))

OBJS_PB := \
    $(OBJS_DIR)/search.pb.o

BINS = \
	$(APP_DIR)/$(BUILD_MODULE_NAME)

all : prep $(BINS) test

.PHONY: prep

prep:
	@mkdir -p $(OBJS_DIR)

$(APP_DIR)/$(BUILD_MODULE_NAME) : $(OBJS_PB) $(OBJS) $(OBJS_DIR)/main.o
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(THIRD_DIR)/lib/libbrpc.a $(LIBS) $(WEB_LIBPATHS) $(WEB_LIBS) -Wl,-rpath=../lib -Wl,-rpath=lib 
	$(STRIP) $@

test: \
	$(TEST_DIR)/test_search

$(TEST_DIR)/test_search : \
	$(OBJS_DIR)/test_search.o \
	$(OBJS_DIR)/tiktoken.o \
	$(OBJS_DIR)/core_bpe.o \
	$(OBJS_DIR)/whisper_token.o \
	$(OBJS_DIR)/audio_utils.o \
	$(OBJS_DIR)/frontend_utils.o \
	$(OBJS_DIR)/frontend_audio_utils.o \
	$(OBJS_DIR)/frontend.o \
	$(OBJS_DIR)/inference_zero_shot.o
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS) $(WEB_LIBPATHS) $(WEB_LIBS) -Wl,-rpath=../lib -Wl,-rpath=lib -lgtest
	$(STRIP) $@

$(OBJS_DIR)/search.pb.o: proto/search.proto
	$(THIRD_HOST_DIR)/bin/protoc --cpp_out=$(OBJS_DIR) --proto_path=$(CURRENT_DIR)/proto $(CURRENT_DIR)/proto/search.proto
	$(CC) $(CPPFLAGS) $(INCLUDES) -c $(OBJS_DIR)/search.pb.cc -o $(OBJS_DIR)/search.pb.o

$(OBJS_DIR)/%.o:%.cpp
	@mkdir -p $(shell dirname $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $? -o $@
$(OBJS_DIR)/%.o:%.cc
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $? -o $@

clean:
	rm -f $(OBJS)
	rm -f $(OBJS_DIR)/search.pb.*
	rm -f $(OBJS_DIR)/main.o
	rm -f $(OBJS_DIR)/test_search.o
	rm -rf rpc_data

distclean:
	make clean
	rm -rf $(APP_DIR)
