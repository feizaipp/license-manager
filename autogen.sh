#! /bin/sh
touch NEWS README ChangeLog AUTHORS
aclocal
autoconf
autoheader
automake --add-missing
