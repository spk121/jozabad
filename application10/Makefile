# CC = clang
CC = gcc
PKGCONFIG = $(shell which pkg-config)
CFLAGS = -std=gnu11 `$(PKGCONFIG) --cflags glib-2.0 gio-2.0 gio-unix-2.0` -g -O0 -Wall -Wno-unknown-attributes
#LIBS = `$(PKGCONFIG) --libs gtk+-3.0 libczmq`
LIBS = `$(PKGCONFIG) --libs glib-2.0 gio-2.0 gio-unix-2.0`
GLIB_COMPILE_RESOURCES = `$(PKGCONFIG) --variable=glib_compile_resources gio-2.0`
GLIB_COMPILE_SCHEMAS = `$(PKGCONFIG) --variable=glib_compile_schemas gio-2.0`

SRC = main.c service.c client.c _gsocketconnection.c msg.c \
	iodir.c tput.c seq.c cause.c diag.c q.c packet.c \
        channel.c crc.c \

#resources.c exampleapp.c exampleappwin.c	exampleappprefs.c

OBJS = $(SRC:.c=.o)

all: exampleapp test1a test_restart

com.lonelycactus.exampleapp.gschema.valid: com.lonelycactus.exampleapp.gschema.xml
	$(GLIB_COMPILE_SCHEMAS) --strict --dry-run --schema-file=$< && mkdir -p $(@D) && touch $@

gschemas.compiled: com.lonelycactus.exampleapp.gschema.valid
	$(GLIB_COMPILE_SCHEMAS) .

resources.c: exampleapp.gresource.xml $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=. --generate-dependencies exampleapp.gresource.xml)
	$(GLIB_COMPILE_RESOURCES) exampleapp.gresource.xml --target=$@ --sourcedir=. --generate-source

%.o: %.c
	$(CC) -c -o $(@F) $(CFLAGS) $<

exampleapp: $(OBJS) gschemas.compiled
	$(CC) -o $(@F) $(LIBS) $(OBJS)

test1a: test1a.o msg.o diag.o iodir.o crc.o
	$(CC) -o $(@F) test1a.o msg.o diag.o iodir.o crc.o $(LIBS)

test_restart: test_restart.o msg.o diag.o iodir.o crc.o
	$(CC) -o $(@F) test_restart.o msg.o diag.o iodir.o crc.o $(LIBS)

clean:
	rm -f com.lonelycactus.exampleapp.gschema.valid
	rm -f gschemas.compiled
	rm -f resources.c
	rm -f $(OBJS)
	rm -f exampleapp

check-syntax:
	$(CC) -Wall -Wextra -pedantic -fsyntax-only $(SRC) test1a.c
