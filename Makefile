
default: build 

build:
	(cd py && ./setup.py build)
	(cd py && ./setup.py build_ext -i)

clean:
	rm -rf py/build
	rm -f py/potatocache/*.so 

.PHONY: default build clean

