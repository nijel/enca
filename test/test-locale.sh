#! /bin/sh
# @(#) $Id: test-iconv.sh,v 1.4 2003/11/17 12:27:40 yeti Exp $
# Purpose: test whether libiconv interface works.
# FIXME: this may fail when the interface works but libiconv is broken!
. $srcdir/setup.sh
if $ENCA --version | grep ' +language-detection ' >/dev/null; then
  TEST_TEXT=$srcdir/cs-s.iso88592
  OPTS=""
  export LC_CTYPE=cs_CZ
  cp $TEST_TEXT $TESTNAME.actual
  $ENCA $OPTS -x UTF-8 $TESTNAME.actual || DIE=1
  # Failures
  export LC_CTYPE=INVALID
  export LC_COLLATE=INVALID
  export LC_MESSAGES=INVALID
  cp $TEST_TEXT $TESTNAME.actual
  $ENCA $OPTS -x UTF-8 $TESTNAME.actual 2>/dev/null && DIE=1
else
  E77=1
fi
. $srcdir/finish.sh
