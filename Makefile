
.PHONY: default all

default: all

include vars.mk

include rules.mk

all: $(TARGETS)

