#!/bin/sh
cd "${MESON_SOURCE_ROOT}"
doxygen 'Doxyfile'
cd doc/latex
make
