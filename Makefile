# TODO Find some way to keep those in sync or maybe it does not matter. Just build with both py and c.
CXXFLAGS = -pthread -DNDEBUG -g -fwrapv -O2 -Wall -Wstrict-prototypes -g -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -D_FORTIFY_SOURCE=2 -fPIC -std=c++11

OBJS = core/os.o core/potatocache.o core/utils.o

LIBS = -lrt

TEST_OBJS = core/tester.o core/test.cpp core/os_test.cpp

TEST_LIBS = -lboost_unit_test_framework


default: build 

build: $(OBJS)

test: $(OBJS) $(TEST_OBJS) Makefile
	$(CXX) -o ./tester $(OBJS) $(TEST_OBJS) $(TEST_LIBS) $(LIBS) 
	./tester

py-build:
	(cd py && ./setup.py build)
	(cd py && ./setup.py build_ext -i)

# This will need some improvement. :)
py-test: clean
	./main.py

clean:
	rm -rf py/build
	rm -f py/potatocache/*.so
	rm -f tester Makefile.bak
	rm -f core/*.o

all-test: test py-test

all-build: build py-build

depend:
	makedepend -Y core/*.cpp core/*.hpp

.PHONY: depend default build clean py-build py-test all-build all-test

# DO NOT DELETE

core/os.o: core/os.hpp core/exceptions.hpp core/utils.hpp core/shared.hpp
core/os_test.o: core/os.hpp core/exceptions.hpp
core/potatocache.o: core/potatocache.hpp core/os.hpp core/exceptions.hpp
core/potatocache.o: core/shared.hpp core/utils.hpp
core/utils.o: core/exceptions.hpp core/utils.hpp
core/os.o: core/exceptions.hpp
core/potatocache.o: core/os.hpp core/exceptions.hpp
