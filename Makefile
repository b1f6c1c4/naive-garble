CC=emcc -c -std=c99
CXX=em++ -c -std=c++17
LD=em++
CFLAGS=-O3 -Isrc/libtommath/headers -Isrc/libtomcrypt/headers -DLTC_SOURCE
CXXFLAGS=$(CFLAGS)
LDFLAGS=

CFILES=$(shell find src/ -type f -name '*.c')
CXXFILES=$(shell find src/ -type f -name '*.cpp')
HFILES=$(shell find src/ -type f -name '*.h')

all: bin/garble.js

define c

obj/$(basename $(FILE)).o: src/$(FILE) $$(HFILES)
	mkdir -p $$(shell dirname $$@)
	$$(CC) -o $$@ $$(CFLAGS) $$<

endef

$(foreach FILE,$(patsubst src/%,%,$(CFILES)),$(eval $(call c, $(FILE))))

define cxx

obj/$(basename $(FILE)).o: src/$(FILE) $$(HFILES)
	mkdir -p $$(shell dirname $$@)
	$$(CXX) -o $$@ $$(CXXFLAGS) $$<

endef

$(foreach FILE,$(patsubst src/%,%,$(CXXFILES)),$(eval $(call cxx, $(FILE))))

obj/main.o: src/garbled_table.hpp src/oblivious_transfer.hpp src/wrapper.hpp src/simple_min.hpp src/util.hpp

bin/garble.js: $(patsubst src/%,obj/%.o,$(basename $(CFILES) $(CXXFILES)))
	mkdir -p $(shell dirname $@)
	$(LD) -o $@ $(LDFLAGS) $^

clean:
	rm -rf bin/ obj/
