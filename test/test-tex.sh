#! /bin/sh
# @(#) $Id: test-tex.sh,v 1.4 2003/11/17 12:27:40 yeti Exp $
# Purpose: test whether TeX decoding works.
. $srcdir/setup.sh
TEST_TEXT=$srcdir/cs-s.tex
OPTS="-L cs"
# File
cp $TEST_TEXT $TESTNAME.actual
$ENCA $OPTS -x UTF-8 $TESTNAME.actual || DIE=1
$ENCA -L none $TESTNAME.actual | grep UTF-8 >/dev/null || DIE=1
$ENCA $OPTS -x TEX $TESTNAME.actual || DIE=1
diff $TEST_TEXT $TESTNAME.actual || DIE=1
. $srcdir/finish.sh
