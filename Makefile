
MAKEFILE_DIR := $(dir $(lastword $(MAKEFILE_LIST)))
BASE_DIR := $(realpath $(CURDIR)/$(MAKEFILE_DIR))
COVERAGE_DIR := $(BASE_DIR)/.cov

EXTRA_CXXFLAGS ?=
EXTRA_LIBS ?= 

# TODO Find some way to keep those in sync or maybe it does not matter. Just build with both py and c.
CXXFLAGS = -pthread -DNDEBUG -g -fwrapv -O2 -Wall -Wno-unused-result -g -fstack-protector --param=ssp-buffer-size=4 -Wformat -Werror=format-security -D_FORTIFY_SOURCE=2 -fPIC -std=c++11 $(EXTRA_CXXFLAGS)

OBJS = core/os.o core/impl.o core/utils.o

LIBS = -lrt -lpthread $(EXTRA_LIBS)

TEST_OBJS = \
	core/run-tests.o \
	core/test.cpp \
	core/os-test.cpp \
	core/impl-test.cpp \

TEST_LIBS = -lboost_unit_test_framework


default: build 

build: $(OBJS)

test: $(OBJS) $(TEST_OBJS) Makefile
	$(CXX) -std=c++11 -o ./run-tests $(OBJS) $(TEST_OBJS) $(TEST_LIBS) $(LIBS) 
	./run-tests

main: $(OBJS) core/main.o
	$(CXX) -o ./main $^ $(LIBS)

py-build:
	(cd py && ./setup.py build)
	(cd py && ./setup.py build_ext -i)

# This will need some improvement. :)
py-test: clean py-build
	./main.py

clean:
	rm -rf py/build .cov
	rm -f py/potatocache/*.so
	rm -f run-tests main Makefile.bak
	rm -f core/*.o core/*.gcda core/*.gcno
	rm -f /run/shm/potato_test_shm_*

all-test: test py-test

all-build: build py-build

depend:
	makedepend -Y core/*.cpp core/*.hpp

todo:
	grep -rIn TODO . | grep -v -e "-rIn" -e .idea

coverage: clean
	mkdir $(COVERAGE_DIR)
	EXTRA_CXXFLAGS="--coverage" EXTRA_LIBS="-lgcov"	make -f $(BASE_DIR)/Makefile --no-print-directory test
	lcov -q --capture --no-external --directory $(BASE_DIR)/core --output-file $(COVERAGE_DIR)/coverage.info
	genhtml -q --prefix $(BASE_DIR) $(COVERAGE_DIR)/coverage.info --output-directory $(COVERAGE_DIR)/html
	firefox $(COVERAGE_DIR)/html/core/index.html

.PHONY: depend default build clean py-build py-test all-build all-test todo

# DO NOT DELETE

core/impl-test.o: core/test.hpp core/impl.hpp core/config.hpp core/os.hpp
core/impl-test.o: core/shared.hpp
core/impl.o: core/shared.hpp core/utils.hpp core/os.hpp core/impl.hpp
core/impl.o: core/config.hpp
core/main.o: core/utils.hpp core/os.hpp
core/os-test.o: core/test.hpp core/os.hpp
core/os.o: core/os.hpp core/utils.hpp
core/test.o: core/test.hpp core/utils.hpp
core/utils.o: core/utils.hpp
core/impl.o: core/config.hpp core/os.hpp core/shared.hpp
core/potatocache.o: core/config.hpp core/impl.hpp core/os.hpp core/shared.hpp
