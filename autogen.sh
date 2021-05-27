#!/bin/sh

DIR=$(dirname $0)
if [ ! -z "$DIR" ]; then
	cd "$DIR"
fi

aclocal && autoconf && automake --add-missing && touch config.h.in
