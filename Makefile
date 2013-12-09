# The basic outline for this came from "Learning C the Hard Way"

CFLAGS = -Isrc -std=c11 -g -O0 -Wall -Wextra \
	-rdynamic $(OPTFLAGS) \
	`pkg-config glib-2.0 libczmq libzmq --cflags` \
	-fdata-sections -ffunction-sections
LIBS = -ldl -lm $(OPTLIBS) \
	`pkg-config glib-2.0 libczmq libzmq --libs` \
	--verbose -Wl,-L/usr/local/lib -Wl,--gc-sections -Wl,--print-gc-sections
PREFIX ?=/usr/local

HEADERS=$(wildcard src/*.h)
GCH=$(patsubst %.h,%.gch,$(HEADERS))
SOURCES=$(wildcard src/*.c)
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))
MAINS=src/main.o

# Unit tests check single functions
UTEST_SRC=$(wildcard tests/*_utest.c)
UTESTS=$(patsubst %.c,%,$(UTEST_SRC))

# Integration tests send messages to a broker
ITEST_SRC=$(wildcard tests/*_itest.c)
ITESTS=$(patsubst %.c,%,$(ITEST_SRC))

# Demo workers
DEMO_SRC=$(wildcard tests/demo*.c)
DEMOS=$(patsubst %.c,%,$(DEMO_SRC))

EXE_TARGET=build/jozabad
LIB_TARGET=build/libjoza.a
SO_TARGET=$(patsubst %.a,%.so,$(LIB_TARGET))

# The Target Build
all: $(EXE_TARGET) $(LIB_TARGET) $(SO_TARGET) tests demos

release: CFLAGS=-std=c11 -O2 -DNDEBUG $(OPTFLAGS)
release: all

$(LIB_TARGET): CFLAGS += -fPIC
$(LIB_TARGET): build $(filter-out $(MAINS),$(OBJECTS))
	ar rcs $@ $(filter-out $(MAINS),$(OBJECTS))
	ranlib $@

$(SO_TARGET): $(LIB_TARGET) $(filter-out $(MAINS),$(OBJECTS))
	$(CC) -shared -o $@ $(filter-out $(MAINS),$(OBJECTS))

$(EXE_TARGET): $(OBJECTS)
	$(CC) -o $@ $(OBJECTS) $(LIBS)

build:
	@mkdir -p build
	@mkdir -p bin

# The Unit Tests
.PHONY: tests
$(UTESTS): %: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS) $(LIB_TARGET)
$(ITESTS): %: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS) $(LIB_TARGET)
tests: $(UTESTS) $(ITESTS)
	sh ./tests/runtests.sh

# Demos
.phony: demos
$(DEMOS): %: %.c
	$(CC) $(CFLAGS) -o $@ $< $(LIBS) $(LIB_TARGET)
demos: $(DEMOS)

valgrind:
	VALGRIND="valgrind --log-file=/tmp/valgrind-%p.log" $(MAKE) tests

# The Cleaner
clean:
	rm -rf build $(OBJECTS) $(UTESTS) $(ITESTS)
	rm -f tests/tests.log
	find . -name "*.gc*" -exec rm {} \;
	rm -rf `find . -name "*.dSYM" -print`

# The Install
install: all
	install -d $(DESTDIR)/$(PREFIX)/lib/
	install $(TARGET) $(DESTDIR)/$(PREFIX)/lib/

# The Checker
BADFUNCS='[^_.>a-zA-Z0-9](str(n?cpy|n?cat|xfrm|n?dup|str|pbrk|tok|_)|stpn?cpy|a?sn?printf|byte_|alloc)'
check:
	@echo Files with potentially dangerous functions.
	@egrep $(BADFUNCS) $(SOURCES) || true
	@echo
	@echo Files where the file name doesn\'t appear in the second line of the file
	@ls -1 src/*.[ch] tests/*.[ch] | xargs -I {} awk -v fname={} -f gnu/fname.awk {} || true
	@echo
	@echo Files that don\'t contain the word Copyright
	@grep -L Copyright src/*.[ch] tests/*.[ch] || true
	@echo
	@echo Files that don\'t contain the phrase "GNU General Public License"
	@grep -L "GNU General Public License" src/*.[ch] tests/*.[ch] || true
	@echo

# Enforce some style
pretty:
	astyle --style=stroustrup --lineend=linux --convert-tabs $(SOURCES) $(HEADERS) $(ITEST_SRC) $(UTEST_SRC)

%.gch: %.h
	$(CC) $(CFLAGS) -o $@ $<

headercheck: $(GCH)

.PHONY: check-syntax

check-syntax:
	$(CC) -Isrc -std=c11 -Wall -Wextra -pedantic -fsyntax-only $(SOURCES)

