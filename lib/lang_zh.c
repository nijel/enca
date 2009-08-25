/*
  language info: chinese

  Copyright (C) 2005 Meng Jie (Zuxy) <zuxy.meng@gmail.com>

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as published
  by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
*/
#ifdef HAVE_CONFIG_H
# include "config.h"
#endif /* HAVE_CONFIG_H */

#include "enca.h"
#include "internal.h"

static const char *const CHARSET_NAMES[] = {
  "gbk",
  "big5"
};

/**
 * ENCA_LANGUAGE_ZH:
 *
 * Chinese language.
 *
 * Everything the world out there needs to know about this language.
 **/
const EncaLanguageInfo ENCA_LANGUAGE_ZH = {
  "zh",
  "chinese",
  2,
  CHARSET_NAMES,
  0,
  0,
  0,
  0,
  0,
  NULL,
  NULL,
  NULL,
};

/* vim: ts=2
 */
