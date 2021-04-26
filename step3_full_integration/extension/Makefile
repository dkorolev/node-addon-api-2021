.PHONY: all clean

all: build/extension_using_current.a

clean:
	rm -rf build

build/extension_using_current.a: extension_using_current.cc extension_using_current.h
	mkdir -p build
	g++ -std=c++17 -o $@ -c $< -fPIC
