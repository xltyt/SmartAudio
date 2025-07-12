CURRENT_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(shell dirname $(CURRENT_PATH))
DATE = $(shell date "+%Y%m%d%H")

PLATFORM ?= amd64

VERSION=$(shell awk -F'|' '{print $$2}' ../../README.md | sed '/^ *$$/d' | awk -F'`' '{print $$2}' | tail -1)
ifeq ($(PLATFORM),arm)
  BUILD_MODULE_NAME=voice_manager_arm
else
  BUILD_MODULE_NAME=voice_manager
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

APP_DIR := $(CURRENT_DIR)/../../$(PLATFORM)/manager/
OBJS_DIR := $(CURRENT_DIR)/../../$(PLATFORM)/build/$(BUILD_MODULE_NAME)
THIRD_DIR := $(CURRENT_DIR)/../../$(PLATFORM)/local
APPWEB_DIR=$(THIRD_DIR)/appweb
include ../common/web.mk

INCLUDES += \
  -I. \
  -I../common \
  -I$(OBJS_DIR) \
  -I$(THIRD_DIR)/include/eigen3/ \
  -I$(THIRD_DIR)/include/

BASE_LIBS += $(THIRD_DIR)/../lib/libmycommon.a
BASE_LIBS += \
  -L$(THIRD_DIR)/lib \
  -lboost_system -lboost_filesystem -lboost_regex \
  -lleveldb -llz4 -lsnappy \
  -lglog \
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

#CFLAGS    += -g -std=c++11 -fopenmp -static-libstdc++ -static-libgcc
#ifeq ($(PLATFORM),amd64)
#  CFLAGS += -mavx -mavx2
#endif
#CFLAGS	  += -DNDEBUG -DFORTIFY_SOURCE=2 -fPIC
#CFLAGS	  += -Wno-deprecated-declarations
##CFLAGS    += -DMKL_ILP64 -m64 -I${MKLROOT}/include
#CFLAGS    += -fstack-protector --param=ssp-buffer-size=4 -Wformat -Wformat-security -Wl,-z,relro,-z,now -Wl,--as-needed -Wl,--no-copy-dt-needed-entries -Wl,-z,noexecstatck -Wl,-z,noexecheap -pie -fPIE -w
#CFLAGS    += -DFORTIFY_SOURCE=2 -fPIC
#CFLAGS    += -g -Wall -fno-strict-aliasing -Wfloat-equal -Wno-unused-function -Wl,-Bsymbolic

LIBPATHS  +=
CPPFLAGS  = $(CFLAGS) -std=c++11
CXXFLAGS  = $(CFLAGS) -std=c++11

SOURCES += \
  global.cpp \
  flags.cpp \
  main.cpp
OBJS := $(addprefix $(OBJS_DIR)/,$(patsubst %.cpp,%.o,$(SOURCES)))
OBJS := $(patsubst %.c,%.o,$(OBJS))
OBJS += $(OBJS_DIR)/web_init.o
OBJS += $(OBJS_DIR)/web_common.o

BINS = \
	$(APP_DIR)/$(BUILD_MODULE_NAME)

all : prep $(BINS)

.PHONY: prep

prep:
	@mkdir -p $(OBJS_DIR)
	@mkdir -p $(APP_DIR)/etc

DEP_CONTENT := $(shell find documents/contents -name '*.esp')
DEP_CONTROLLER := $(shell find documents/controllers -name '*.c')

documents/cache/smart.c: $(DEP_CONTENT) $(DEP_CONTROLLER)
	PATH=$(THIRD_DIR)/bin:$(PATH) make -C documents

$(OBJS_DIR)/web_init.o: documents/cache/smart.c web_init.c
	$(CC) -c -o $(OBJS_DIR)/web_init.o $(CFLAGS) $(WEB_CFLAGS) $(INCLUDES) -Idocuments -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" web_init.c
$(OBJS_DIR)/web_common.o: web_common.c
	$(CC) -c -o $(OBJS_DIR)/web_common.o $(CFLAGS) $(WEB_CFLAGS) $(INCLUDES) -Idocuments -D_FILE_OFFSET_BITS=64 -D_FILE_OFFSET_BITS=64 -DMBEDTLS_USER_CONFIG_FILE=\"embedtls.h\" web_common.c

$(APP_DIR)/$(BUILD_MODULE_NAME) : $(OBJS) $(OBJS_DIR)/main.o
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS) $(LIBS) $(WEB_LIBPATHS) $(WEB_LIBS) -ljson-c -lsqlcipher -lpcre -Wl,-rpath=../lib -Wl,-rpath=lib
	$(STRIP) $@
	mkdir -p $(APP_DIR)/etc
	mkdir -p $(APP_DIR)/log
	mkdir -p $(APP_DIR)/logs
	mkdir -p $(APP_DIR)/tmp
	mkdir -p $(APP_DIR)/data
	cp -f etc/web.conf $(APP_DIR)/etc/web_ori.conf
	sed -i '/^#/d' $(APP_DIR)/etc/web_ori.conf
	sed -i '/^$$/d' $(APP_DIR)/etc/web_ori.conf
	rm -rf $(APP_DIR)/documents
	cp -r documents/deploy/dist $(APP_DIR)/documents
	cd $(APP_DIR)/documents && rm -rf data && ln -sf ../data .

$(OBJS_DIR)/%.o:%.cpp
	@mkdir -p $(shell dirname $@)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $? -o $@
$(OBJS_DIR)/%.o:%.cc
	@mkdir -p $(shell dirname $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $? -o $@

clean:
	rm -f $(OBJS)
	rm -f $(OBJS_DIR)/main.o
	rm -rf rpc_data
	PATH=$(THIRD_DIR)/bin:$(PATH) make clean -C documents

distclean:
	make clean
	rm -rf $(APP_DIR)/
