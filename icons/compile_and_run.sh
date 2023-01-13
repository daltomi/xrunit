#
#	Copyright © 2020 Daniel T. Borelli <danieltborelli@gmail.com>
#
#	This file is part of xrunit.
#
#	xrunit is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.
#
#	xrunit is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with xrunit.  If not, see <http://www.gnu.org/licenses/>.


#!/bin/bash
echo "[*] Compilando..."
chmod -w compile_and_run.sh png2h.c
g++ -c png2h.c $(fltk-config --cxxflags) || exit 1
g++ png2h.o -o png2h $(fltk-config --ldflags --use-images) || exit 1
rm png2h.o

echo "[*] Eliminando icons.h ..."
rm icons.h

echo "[*] Generando icons.h ..."
echo "//Generate by png2c" > icons.h
echo "//Author: danieltborelli@gmail.com" >> icons.h
echo "//Project: xrunit" >> icons.h
echo "#pragma once" >> icons.h
for i in *.png; do
	./png2h $i >> icons.h
done

rm png2h
echo "[*] Done..."
