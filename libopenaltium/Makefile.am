lib_LTLIBRARIES = libopenaltium.la

libopenaltium_la_SOURCES = \
	read_data.c

libopenaltium_la_CFLAGS = \
	$(GLIB_CFLAGS)

libopenaltium_la_LDFLAGS = \
	$(GLIB_LIBS)

.PHONY: test

test: read_data
	./read_data -f Data
