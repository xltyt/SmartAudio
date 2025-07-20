CURRENT_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CURRENT_DIR := $(shell dirname $(CURRENT_PATH))

.PHONY: all
all: cache/smart.c

DEP_CONTENT := $(shell find contents -name '*.esp')
DEP_CONTROLLER := $(shell find controllers -name '*.c')

cache/smart.c: $(DEP_CONTENT) $(DEP_CONTROLLER)
	@echo "\033[32m=====> Build Html\033[0m"
	expansive clean
	expansive render
	esp --combine compile
	@echo "\033[32m=====> Deploy Html\033[0m"
	cp -r js/ dist/
	cp -r css/* dist/css/
	rm -rf deploy
	expansive deploy
	rm -f deploy/package.json
	rm -rf deploy/db
	rm -rf deploy/dist/*.esp
	rm -rf deploy/dist/*.html
	rm -rf deploy/dist/global
	rm -rf deploy/dist/system
	bash -x adjust_html_cache.sh
	@echo "\033[32m=====> Finish Html\033[0m"
	
clean:
	@echo -e "\033[32m=====> Clean Html\033[0m"
	esp clean
	expansive clean
	rm -rf deploy
