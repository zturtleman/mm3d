include ../Makefile.common

all: libtest_common.a

clean:
	rm -rf libtest_common.a test_common.o testfilefactory.o *.gcno *.gcda

libtest_common.a: test_common.o testfilefactory.o 
	ar cru libtest_common.a test_common.o testfilefactory.o
