CFLAGS=-O3 -Isrc/libtommath/headers -Isrc/libtomcrypt/headers -DLTC_SOURCE
CXXFLAGS=$(CFLAGS)

CFILES=$(shell find src/ -type f -name '*.c')
HFILES=$(shell find src/ -type f -name '*.h')
HPPFILES=$(wildcard src/*.hpp)

all: bin/garble bin/garble.html

define c-to-obj

obj/gcc/$(basename $(FILE)).o: src/$(FILE) $$(HFILES)
	mkdir -p $$(shell dirname $$@)
	gcc -c --std=c99 -o $$@ $$(CFLAGS) $$<

obj/gcc/lib.a: obj/gcc/$(basename $(FILE)).o

obj/emcc/$(basename $(FILE)).o: src/$(FILE) $$(HFILES)
	mkdir -p $$(shell dirname $$@)
	emcc -c --std=c99 -o $$@ $$(CFLAGS) $$<

obj/emcc/lib.a: obj/emcc/$(basename $(FILE)).o

endef

$(foreach FILE,$(patsubst src/%,%,$(CFILES)),$(eval $(call c-to-obj, $(FILE))))

obj/gcc/lib.a:
	mkdir -p $(shell dirname $@)
	ar -r $@ $^

obj/emcc/lib.a:
	mkdir -p $(shell dirname $@)
	emar -r $@ $^

obj/gcc/main.o: src/main.gcc.cpp $(HFILES) $(HPPFILES)
	mkdir -p $(shell dirname $@)
	g++ -c --std=c++17 -o $@ $(CXXFLAGS) $<

obj/emcc/main.o: src/main.emcc.cpp $(HFILES) $(HPPFILES)
	mkdir -p $(shell dirname $@)
	em++ -c --std=c++17 -o $@ $(CXXFLAGS) $<

bin/garble: obj/gcc/main.o obj/gcc/lib.a
	mkdir -p $(shell dirname $@)
	g++ -o $@ $^

bin/garble.html: src/main.post.js obj/emcc/main.o obj/emcc/lib.a
	mkdir -p $(shell dirname $@)
	em++ -o $@ --post-js $^ \
		-s EXTRA_EXPORTED_RUNTIME_METHODS='["ccall"]' \
		-s DEMANGLE_SUPPORT=1

clean:
	rm -rf bin/ obj/
