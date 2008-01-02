include ../Makefile.common

LIB = ../../libmm3d/libmm3d.a

# If you add a test, be sure to add a moc dependency, and a run line below

TESTS = \
		  model_init_test.unit \
		  model_equal_test.unit \
		  model_test.unit \
		  util_test.unit \

# Add moc dependencies here

model_init_test.unit: model_init_test.moc $(LIB)

model_equal_test.unit: model_equal_test.moc $(LIB)

model_test.unit: model_test.moc $(LIB)

util_test.unit: util_test.moc $(LIB)

all:

test: $(TESTS)

# Add run line here

check: test
	./model_init_test.unit -silent
	./model_equal_test.unit -silent
	./model_test.unit -silent
	./util_test.unit -silent

clean:
	rm -rf *.unit *.moc core core.*
