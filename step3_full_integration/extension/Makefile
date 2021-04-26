.PHONY: all clean

all: build/extension.so

clean:
	rm -rf build

build/extension.so: extension.cc
	mkdir -p build
	g++ -o build/extension.so -c $< -fPIC
