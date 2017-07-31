/* libaddr2line.h

   Copyright (C) 2016
        403fd4d072f534ee5bd7da6efc9462f3995bb456bad644cd9bb7bcaad314b02d source@appudo.com
        d3c2e0357cde0e67a0649c55dda800615d5be0c8ea9845aeda270d0fe57c1363 source@appudo.com

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, 51 Franklin Street - Fifth Floor, Boston,
   MA 02110-1301, USA.  */

#ifndef LIBADDR2LINE_H
#define LIBADDR2LINE_H

extern "C" int   __attribute__((visibility("default"))) addr2line(char* attr, char* outfile, int outSize, int fd = -1);

#endif // LIBADDR2LINE_H
