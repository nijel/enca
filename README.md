# Enca - guess and convert encoding of text files

[![build status](https://secure.travis-ci.org/nijel/enca.png)](https://travis-ci.org/nijel/enca)
[![Coverage Status](https://coveralls.io/repos/nijel/enca/badge.png?branch=master)](https://coveralls.io/r/nijel/enca?branch=master)

## Copyright 

Copyright (C) 2000-2003 David Necas (Yeti) <yeti@physics.muni.cz>

Copyright (C) 2009-2013 Michal Cihar <michal@cihar.com>

## Description

Enca (Extremely Naive Charset Analyser) consists of two main components:

  * libenca, an encoding detection library.  It currently supports
    Belarusian, Bulgarian, Croatian, Czech, Estonian, Hungarian, Latvian,
    Lithuanian, Polish, Russian, Slovak, Slovene, Ukrainian, Chinese, and
    some multibyte encodings independently on language.  The API should be
    relatively stable (to be read as `it will either change only
    marginally, or very drastically').

  * enca, a command line frontend, integrating libenca and several
    charset conversion libraries and tools (GNU recode, UNIX98 iconv,
    perl Unicode::Map, cstocs).


## Installation

Please see INSTALL for system requirements, detailed installation
instructions and also for description of optional features that can
be selected at ./configure time.

For the impatient: Run

    ./configure
    make
    make check
    make install

as usual.


## License

Enca can be copied and/or modified under the terms of version 2 of
GNU General Public License.  Please see COPYING for details.


## Web resources

Enca can be found at http://github.com/nijel/enca/, you can download 
tarballs from http://dl.cihar.com/enca/.


## Bugs

Report problems at <https://github.com/nijel/enca/issues>. Some known bugs have
been collected in BUGS section of enca manual page.


## Hacking (with) Enca

Please see TODO for list of things that should be fixed and features to
be implemented and their priority and also for list of things that
definitely shouldn't be implemented.

The file README.devel describes what needs to be done to add a new
encoding or language to Enca.

The directory devel-docs/html contains Enca library API documentation in
HTML form.
