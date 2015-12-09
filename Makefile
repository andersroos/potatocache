
default: build 

build:
	(cd py && ./setup.py build)
	(cd py && ./setup.py build_ext -i)

clean:
	rm -rf py/build
	rm -f py/potatocache/*.so 

# This will need some improvement. :)
test: clean build
	./main.py

.PHONY: default build clean
