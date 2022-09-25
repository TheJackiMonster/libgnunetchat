# This Makefile.am is in the public domain

EXTRA_DIST = \
  Doxyfile \
  HOWTO.md

SUBDIRS = \
  include \
  src \
  tests

ACLOCAL_AMFLAGS = -I m4

docs:
	mkdir -p doc
	doxygen

