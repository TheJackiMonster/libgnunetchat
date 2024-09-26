#!/bin/sh
cd "${MESON_SOURCE_ROOT}"
sed -i 's/PROJECT_NUMBER\(\s\+=\s\+\)\([0-9\.a-z\-]\+\)/PROJECT_NUMBER\1'"$(sh contrib/get_version.sh)/" 'Doxyfile'
doxygen 'Doxyfile'
cd doc/latex
make
