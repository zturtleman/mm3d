include ../Makefile.common

LIB = ../common/libtest_common.a ../../libmm3d/libmm3d.a

# If you add a test, be sure to add a moc dependency, and a run line below

TESTS = \
		  memutil_test.unit \
		  datasource_test.unit \
		  datadest_test.unit \
		  filedatasource_test.unit \
		  texcompare_test.unit \
		  model_init_test.unit \
		  model_equal_test.unit \
		  model_add_test.unit \
		  model_vertex_test.unit \
		  model_triangle_test.unit \
		  model_select_test.unit \
		  model_test.unit \
		  new_mm3d_test.unit \
		  cmdlinemgr_test.unit \
		  util_test.unit \
		  misc_test.unit \

# Add moc dependencies here

all:

cmdlinemgr_test.unit: cmdlinemgr_test.moc $(LIB)

datasource_test.unit: datasource_test.moc $(LIB)

datadest_test.unit: datadest_test.moc $(LIB)

filedatasource_test.unit: filedatasource_test.moc $(LIB)

memutil_test.unit: memutil_test.moc $(LIB)

texcompare_test.unit: texcompare_test.moc $(LIB)

model_init_test.unit: model_init_test.moc $(LIB)

model_equal_test.unit: model_equal_test.moc $(LIB)

model_add_test.unit: model_add_test.moc $(LIB)

model_vertex_test.unit: model_vertex_test.moc $(LIB)

model_triangle_test.unit: model_triangle_test.moc $(LIB)

model_select_test.unit: model_select_test.moc $(LIB)

model_test.unit: model_test.moc $(LIB)

new_mm3d_test.unit: new_mm3d_test.moc $(LIB)

util_test.unit: util_test.moc $(LIB)

misc_test.unit: misc_test.moc $(LIB)

test: $(TESTS)

# Add run line here

check: test
	./memutil_test.unit -silent
	./datasource_test.unit -silent
	./filedatasource_test.unit -silent
	./datadest_test.unit -silent
	./texcompare_test.unit -silent
	./model_init_test.unit -silent
	./model_equal_test.unit -silent
	./model_add_test.unit -silent
	./model_vertex_test.unit -silent
	./model_triangle_test.unit -silent
	./model_select_test.unit -silent
	./model_test.unit -silent
	./util_test.unit -silent
	./misc_test.unit -silent
	./cmdlinemgr_test.unit -silent

fullcheck: check
	./new_mm3d_test.unit -silent

clean:
	rm -rf *.unit *.moc core core.*
