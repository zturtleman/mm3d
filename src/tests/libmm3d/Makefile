include ../Makefile.common

LIB = ../common/libtest_common.a ../../libmm3d/libmm3d.a

# If you add a test, be sure to add a moc dependency, and a run line below

TESTS = \
		  memutil_test.unit \
		  glmath_test.unit \
		  big_endian_test.unit \
		  datasource_test.unit \
		  datadest_test.unit \
		  filedatasource_test.unit \
		  undomgr_test.unit \
		  texcompare_test.unit \
		  texscale_test.unit \
		  model_init_test.unit \
		  model_equal_test.unit \
		  model_equiv_test.unit \
		  model_add_test.unit \
		  model_vertex_test.unit \
		  model_triangle_test.unit \
		  model_group_test.unit \
		  model_material_test.unit \
		  model_joint_test.unit \
		  model_select_test.unit \
		  model_weld_test.unit \
		  model_test.unit \
		  new_mm3d_test.unit \
		  filter_mm3d_test.unit \
		  filter_ms3d_test.unit \
		  filter_cal3d_test.unit \
		  filter_cob_test.unit \
		  filter_dxf_test.unit \
		  filter_obj_test.unit \
		  filter_md2_test.unit \
		  filter_md3_test.unit \
		  filtermgr_test.unit \
		  cmdlinemgr_test.unit \
		  util_test.unit \
		  misc_test.unit \
		  translation_test.unit \
		  msg_test.unit

# Add moc dependencies here

all:

cmdlinemgr_test.unit: cmdlinemgr_test.moc $(LIB)

filtermgr_test.unit: filtermgr_test.moc $(LIB)

big_endian_test.unit: big_endian_test.moc $(LIB)

datasource_test.unit: datasource_test.moc $(LIB)

datadest_test.unit: datadest_test.moc $(LIB)

filedatasource_test.unit: filedatasource_test.moc $(LIB)

undomgr_test.unit: undomgr_test.moc $(LIB)

memutil_test.unit: memutil_test.moc $(LIB)

glmath_test.unit: glmath_test.moc $(LIB)

texcompare_test.unit: texcompare_test.moc $(LIB)

texscale_test.unit: texscale_test.moc $(LIB)

model_init_test.unit: model_init_test.moc $(LIB)

model_equal_test.unit: model_equal_test.moc $(LIB)

model_equiv_test.unit: model_equiv_test.moc $(LIB)

model_add_test.unit: model_add_test.moc $(LIB)

model_vertex_test.unit: model_vertex_test.moc $(LIB)

model_triangle_test.unit: model_triangle_test.moc $(LIB)

model_group_test.unit: model_group_test.moc $(LIB)

model_material_test.unit: model_material_test.moc $(LIB)

model_joint_test.unit: model_joint_test.moc $(LIB)

model_select_test.unit: model_select_test.moc $(LIB)

model_weld_test.unit: model_weld_test.moc $(LIB)

model_test.unit: model_test.moc $(LIB)

new_mm3d_test.unit: new_mm3d_test.moc $(LIB)

filter_mm3d_test.unit: filter_mm3d_test.moc $(LIB)

filter_ms3d_test.unit: filter_ms3d_test.moc $(LIB)

filter_cal3d_test.unit: filter_cal3d_test.moc $(LIB)

filter_cob_test.unit: filter_cob_test.moc $(LIB)

filter_dxf_test.unit: filter_dxf_test.moc $(LIB)

filter_obj_test.unit: filter_obj_test.moc $(LIB)

filter_md2_test.unit: filter_md2_test.moc $(LIB)

filter_md3_test.unit: filter_md3_test.moc $(LIB)

util_test.unit: util_test.moc $(LIB)

misc_test.unit: misc_test.moc $(LIB)

msg_test.unit: msg_test.moc $(LIB)

translation_test.unit: translation_test.moc $(LIB)

test: $(TESTS)

# Add run line here

check: test
	./memutil_test.unit -silent
	./glmath_test.unit -silent
	./big_endian_test.unit -silent
	./datasource_test.unit -silent
	./filedatasource_test.unit -silent
	./undomgr_test.unit -silent
	./datadest_test.unit -silent
	./texcompare_test.unit -silent
	./texscale_test.unit -silent
	./model_init_test.unit -silent
	./model_equal_test.unit -silent
	./model_equiv_test.unit -silent
	./model_add_test.unit -silent
	./model_vertex_test.unit -silent
	./model_triangle_test.unit -silent
	./model_group_test.unit -silent
	./model_material_test.unit -silent
	./model_joint_test.unit -silent
	./model_select_test.unit -silent
	./model_weld_test.unit -silent
	./model_test.unit -silent
	./util_test.unit -silent
	./misc_test.unit -silent
	./translation_test.unit -silent
	./msg_test.unit -silent
	./filtermgr_test.unit -silent
	./cmdlinemgr_test.unit -silent

filtercheck:
	./new_mm3d_test.unit -silent
	./filter_mm3d_test.unit -silent
	./filter_ms3d_test.unit -silent
	./filter_cal3d_test.unit -silent
	./filter_md2_test.unit -silent
	./filter_md3_test.unit -silent
	./filter_obj_test.unit -silent
	./filter_cob_test.unit -silent
	./filter_dxf_test.unit -silent

fullcheck: check filtercheck

cov:
	mkdir -p cov_html
	lcov -c -d ../../ -o cov_html/coverage.dat
	./genhtml -o cov_html/ cov_html/coverage.dat

clean:
	rm -rf *.unit *.moc core core.* *.gcno

.PHONY: check fullcheck filtercheck cov clean
