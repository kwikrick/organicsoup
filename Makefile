# Main Makefile to select configuration

# Default configuration
CONFIG ?= linux

ifeq ($(CONFIG),linux)
include Makefile_linux
else ifeq ($(CONFIG),web)
include Makefile_web
else
$(error Unknown CONFIG '$(CONFIG)'. Use 'linux' or 'web')
endif

.PHONY: config
config:
	@echo "Using configuration: $(CONFIG)"