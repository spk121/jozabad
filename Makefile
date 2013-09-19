# The basic outline for this came from "Learning C the Hard Way"

CFLAGS=-std=c11 -g -O0 -Wall -Wextra -Wtype-limits -Wstrict-overflow=5 -fstrict-overflow -Wsign-compare -Isrc -rdynamic -DNDEBUG $(OPTFLAGS)
LIBS=-ldl -lczmq -lzmq -lm $(OPTLIBS)
PREFIX?=/usr/local

HEADERS=$(wildcard src/**/*.h src/*.h)
GCH=$(patsubst %.h,%.gch,$(HEADERS))
SOURCES=$(wildcard src/**/*.c src/*.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))
MAINS=src/main.o

TEST_SRC=$(wildcard tests/*_tests.c)
TESTS=$(patsubst %.c,%,$(TEST_SRC))

EXE_TARGET=build/jozabad
LIB_TARGET=build/libjoza.a
SO_TARGET=$(patsubst %.a,%.so,$(LIB_TARGET))

# The Target Build
all: $(EXE_TARGET) $(LIB_TARGET) $(SO_TARGET) tests

release: CFLAGS=-std=c11 -O2 $(OPTFLAGS)
release: all

$(LIB_TARGET): CFLAGS += -fPIC
$(LIB_TARGET): build $(filter-out $(MAINS),$(OBJECTS))
	ar rcs $@ $(filter-out $(MAINS),$(OBJECTS))
	ranlib $@

$(SO_TARGET): $(LIB_TARGET) $(filter-out $(MAINS),$(OBJECTS))
	$(CC) -shared -o $@ $(filter-out $(MAINS),$(OBJECTS))

$(EXE_TARGET): $(LIB_TARGET) $(MAINS)
	$(CC) -o $@ $(MAINS) $(LIB_TARGET) $(LIBS)

build:
	@mkdir -p build
	@mkdir -p bin

# The Unit Tests
.PHONY: tests
$(TESTS): %: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS) $(LIB_TARGET)
tests: $(TESTS)
	sh ./tests/runtests.sh

valgrind:
	VALGRIND="valgrind --log-file=/tmp/valgrind-%p.log" $(MAKE)

# The Cleaner
clean:
	rm -rf build $(OBJECTS) $(TESTS)
	rm -f tests/tests.log
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`

# The Install
install: all
	install -d $(DESTDIR)/$(PREFIX)/lib/
	install $(TARGET) $(DESTDIR)/$(PREFIX)/lib/

# The Checker
BADFUNCS='[^_.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)|stpn?cpy|a?sn?printf|byte_)'
check:
	@echo Files with potentially dangerous functions.
	@egrep $(BADFUNCS) $(SOURCES) || true

# Enforce some style
pretty:
	astyle --style=stroustrup --lineend=linux --convert-tabs $(SOURCES) $(HEADERS)

%.gch: %.h
	$(CC) $(CFLAGS) -o $@ $<

headercheck: $(GCH)

.PHONY: check-syntax

check-syntax:
	$(CC) -Wall -Wextra -pedantic -fsyntax-only $(SOURCES)

