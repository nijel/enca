#! /bin/sh
# @(#) $Id: test-guess-utf8.sh,v 1.4 2003/11/17 12:27:40 yeti Exp $
# Purpose: various UTF-8 variant recognition.
. $srcdir/setup.sh
for l in $ALL_TEST_LANGUAGES; do
  $ENCA -p -f -L $l $srcdir/$l-utf8.* | sed -e "s#^$srcdir/##" >>$TESTNAME.actual || DIE=1
done
. $srcdir/finish.sh
