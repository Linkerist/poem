#!/bin/sh

postfix=$1
for i in `ls *.$postfix`; do
	if test -x $i; then
		echo $i
		chmod a-x $i
	fi
done
