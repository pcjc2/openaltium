lib_LTLIBRARIES = libopenaltium.la

libopenaltium_la_SOURCES = \
	content-parser.c \
	content-parser.h \
	parameters.c \
	parameters.h \
	models.c \
	models.h \
	pcblib.c \
	pcblib.h \
	pcblib-data.c \
	pcblib-data.h \
	schlib.c \
	schlib.h \
	schlib-data.c \
	schlib-data.h \
	main.c

libopenaltium_la_CFLAGS = \
	$(GLIB_CFLAGS) \
	$(GIO_CFLAGS) \
	$(GSF_CFLAGS) \
	-Wall

libopenaltium_la_LDFLAGS = \
	$(GLIB_LIBS) \
	$(GIO_LIBS) \
	$(GSF_LIBS) \
	-lm \
	-Wall

bin_PROGRAMS = read_data

read_data_SOURCES = \
	$(libopenaltium_la_SOURCES)

read_data_CFLAGS = \
	$(GLIB_CFLAGS) \
	$(GIO_CFLAGS) \
	$(GSF_CFLAGS)

read_data_LDFLAGS = \
	$(GLIB_LIBS) \
	$(GIO_LIBS) \
	$(GSF_LIBS) \
	-lm

.PHONY: test

test: read_data
	./read_data -f Data
