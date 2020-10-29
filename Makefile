MIX = mix
CFLAGS = -g -O3 -ansi -pedantic -Wall -Wextra -Wno-unused-parameter

ERLANG_PATH = $(shell erl -eval 'io:format("~s", [lists:concat([code:root_dir(), "/erts-", erlang:system_info(version), "/include"])])' -s init stop -noshell)
CFLAGS += -I$(ERLANG_PATH)

EXTRACTOR_PATH = /usr/lib/x86_64-linux-gnu

ifeq ($(wildcard deps/hoedown),)
	HOEDOWN_PATH = ../hoedown
else
	HOEDOWN_PATH = deps/hoedown
endif

CFLAGS += -I$(HOEDOWN_PATH)/src -ldl

ifneq ($(OS),Windows_NT)
	CFLAGS += -fPIC

	ifeq ($(shell uname),Darwin)
		LDFLAGS += -dynamiclib -undefined dynamic_lookup
	endif
endif

.PHONY: all extractor clean

all: extractor

extractor:
	$(MIX) compile

priv/extractor.so: src/extractor.c
	$(MAKE) -C $(HOEDOWN_PATH) libhoedown.a
	$(CC) $(CFLAGS) -shared $(LDFLAGS) -o $@ src/extractor.c $(HOEDOWN_PATH)/libhoedown.a $(EXTRACTOR_PATH)/libextractor.so

clean:
	$(MIX) clean
	$(MAKE) -C $(HOEDOWN_PATH) clean
	$(RM) priv/extractor.so
