MIX = mix
CFLAGS = -g -O3 -ansi -pedantic -Wall -Wextra -Wno-unused-parameter -fPIC

ERLANG_PATH = $(shell erl -eval 'io:format("~s", [lists:concat([code:root_dir(), "/erts-", erlang:system_info(version), "/include"])])' -s init stop -noshell)
CFLAGS += -I$(ERLANG_PATH)

ifeq ($(EXTRACTOR_PATH),)
	EXTRACTOR_PATH := /usr/lib/x86_64-linux-gnu
endif

ifeq ($(wildcard deps/extractor),)
	SELF_PATH = ../extractor
else
	SELF_PATH = deps/extractor
endif

CFLAGS += -I$(EXTRACTOR_PATH)/src -ldl

.PHONY: all extractor clean

all: extractor

extractor:
	$(MIX) compile

priv/extractor.so: src/extractor.c
	$(CC) $(CFLAGS) -shared $(LDFLAGS) -o $@ src/extractor.c $(EXTRACTOR_PATH)/libextractor.so

clean:
	$(MIX) clean
	$(MAKE) -C $(SELF_PATH) clean
	$(RM) priv/extractor.so
