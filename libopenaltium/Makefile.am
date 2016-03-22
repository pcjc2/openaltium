lib_LTLIBRARIES = libopenaltium.la

libopenaltium_la_SOURCES = \
	content-parser.c \
	pcblib-data.c \
	pcblib.c

libopenaltium_la_CFLAGS = \
	$(GLIB_CFLAGS) \
	$(GSF_CFLAGS)

libopenaltium_la_LDFLAGS = \
	$(GLIB_LIBS) \
	$(GSF_LIBS)

bin_PROGRAMS = read_data

read_data_SOURCES = \
	$(libopenaltium_la_SOURCES)

read_data_CFLAGS = \
	$(GLIB_CFLAGS) \
	$(GSF_CFLAGS)

read_data_LDFLAGS = \
	$(GLIB_LIBS) \
	$(GSF_LIBS)

.PHONY: test

test: read_data
	./read_data -f Data
