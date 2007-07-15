#!/bin/sh

# Using autoconf 2.50 and automake 1.9
# If you are trying this with any other version it may not work

aclocal && autoconf && automake && touch config.h.in
