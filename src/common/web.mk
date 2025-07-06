APPWEB_DIR            ?= /

ME_COM_CGI            ?= 0
ME_COM_COMPILER       ?= 1
ME_COM_DIR            ?= 0
ME_COM_EJS            ?= 0
ME_COM_ESP            ?= 1
ME_COM_HTTP           ?= 1
ME_COM_LIB            ?= 1
ME_COM_MATRIXSSL      ?= 0
ME_COM_MBEDTLS        ?= 1
ME_COM_MDB            ?= 1
ME_COM_MPR            ?= 1
ME_COM_NANOSSL        ?= 0
ME_COM_OPENSSL        ?= 0
ME_COM_OSDEP          ?= 1
ME_COM_PCRE           ?= 1
ME_COM_PHP            ?= 0
ME_COM_SSL            ?= 1
ME_COM_VXWORKS        ?= 0
ME_COM_WATCHDOG       ?= 1

ME_COM_CGI            = 1
ME_COM_OPENSSL        = 1
ME_COM_MBEDTLS        = 0
ME_COM_EJS            = 0
ME_COM_ESP            = 1

ifeq ($(ME_COM_LIB),1)
    ME_COM_COMPILER := 1
endif
ifeq ($(ME_COM_MBEDTLS),1)
    ME_COM_SSL := 1
endif
ifeq ($(ME_COM_OPENSSL),1)
    ME_COM_SSL := 1
endif
ifeq ($(ME_COM_ESP),1)
    ME_COM_MDB := 1
endif

#APPWEB_PROFILE = default
APPWEB_PROFILE = static

WEB_CFLAGS = -DME_DEBUG=1 $(patsubst %,-D%,$(filter ME_%,$(MAKEFLAGS))) -DME_COM_CGI=$(ME_COM_CGI) -DME_COM_COMPILER=$(ME_COM_COMPILER) -DME_COM_DIR=$(ME_COM_DIR) -DME_COM_EJS=$(ME_COM_EJS) -DME_COM_ESP=$(ME_COM_ESP) -DME_COM_HTTP=$(ME_COM_HTTP) -DME_COM_LIB=$(ME_COM_LIB) -DME_COM_MATRIXSSL=$(ME_COM_MATRIXSSL) -DME_COM_MBEDTLS=$(ME_COM_MBEDTLS) -DME_COM_MDB=$(ME_COM_MDB) -DME_COM_MPR=$(ME_COM_MPR) -DME_COM_NANOSSL=$(ME_COM_NANOSSL) -DME_COM_OPENSSL=$(ME_COM_OPENSSL) -DME_COM_OSDEP=$(ME_COM_OSDEP) -DME_COM_PCRE=$(ME_COM_PCRE) -DME_COM_PHP=$(ME_COM_PHP) -DME_COM_SSL=$(ME_COM_SSL) -DME_COM_VXWORKS=$(ME_COM_VXWORKS) -DME_COM_WATCHDOG=$(ME_COM_WATCHDOG) 

WEB_CFLAGS += -I$(APPWEB_DIR)/include/appweb

WEB_LIBPATHS = -L$(APPWEB_DIR)/lib/appweb/latest/bin/

WEB_LIBS += -lappweb
WEB_LIBS += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
    WEB_LIBS += -lmbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    WEB_LIBS += -lmpr-mbedtls
endif
ifeq ($(ME_COM_MBEDTLS),1)
    WEB_LIBS += -lmbedtls
endif
ifeq ($(ME_COM_OPENSSL),1)
    WEB_LIBS += -lmpr-openssl
endif
WEB_LIBS += -lmpr
ifeq ($(ME_COM_MBEDTLS),1)
    WEB_LIBS += -lmpr-mbedtls
endif
ifeq ($(ME_COM_PCRE),1)
    WEB_LIBS += -lpcre
endif
ifeq ($(ME_COM_HTTP),1)
    WEB_LIBS += -lhttp
endif
ifeq ($(ME_COM_PCRE),1)
    WEB_LIBS += -lpcre
endif
WEB_LIBS += -lmpr-version
ifeq ($(ME_COM_ESP),1)
    WEB_LIBS += -lesp
endif
WEB_LIBS += -lmpr-version
ifeq ($(ME_COM_HTTP),1)
    WEB_LIBS += -lhttp
endif
#WEB_LIBS += -lappweb
ifeq ($(ME_COM_ESP),1)
    WEB_LIBS += -lesp
endif
