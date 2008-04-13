noinst_LIBRARIES = libmm3d.a

libmm3d_HFILES = \
	binutil.h \
	bsptree.h \
	cal3dfilter.h \
	cmdlinemgr.h \
	cobfilter.h \
	datadest.h \
	filedatadest.h \
	memdatadest.h \
	datasource.h \
	filedatasource.h \
	memdatasource.h \
	drawcontext.h \
	dxffilter.h \
	endianconfig.h \
	file_closer.h \
	filefactory.h \
	filtermgr.h \
	glheaders.h \
	glmath.h \
	local_array.h \
	local_ptr.h \
	log.h \
	lwofilter.h \
	md2filter-anorms.h \
	md2filter.h \
	md3filter.h \
	mesh.h \
	misc.h \
	mlocale.h \
	mm3dconfig.h \
	mm3dfilter.h \
	mm3dfilter_ref.h \
	mm3dport.h \
	mm3dreg.h \
	mm3dtypes.h \
	model.h \
	modelfilter.h \
	modelstatus.h \
	modelundo.h \
	modelutil.h \
	ms3dfilter.h \
	msg.h \
	objfilter.h \
	pcxtex.h \
	rawtex.h \
	release_ptr.h \
	sorted_list.h \
	texmgr.h \
	texscale.h \
	texture.h \
	tgatex.h \
	triprim.h \
	translate.h \
	txtfilter.h \
	undo.h \
	undomgr.h \
	util.h \
	weld.h 

libmm3d_a_SOURCES = \
	bsptree.cc \
	cal3dfilter.cc \
	cmdlinemgr.cc \
	cobfilter.cc \
	datadest.cc \
	filedatadest.cc \
	memdatadest.cc \
	datasource.cc \
	filedatasource.cc \
	memdatasource.cc \
	dxffilter.cc \
	filefactory.cc \
	filtermgr.cc \
	glmath.cc \
	log.cc \
	lwofilter.cc \
	mesh.cc \
	misc.cc \
	mlocale.cc \
	model.cc model_anim.cc model_bool.cc model_copy.cc model_draw.cc model_group.cc model_influence.cc model_inner.cc model_insert.cc model_meta.cc model_ops.cc model_print.cc model_proj.cc model_select.cc model_texture.cc \
	modelfilter.cc \
	modelstatus.cc \
	modelundo.cc \
	modelutil.cc \
	md2filter.cc \
	md3filter.cc \
	mm3dfilter.cc \
	mm3dfilter_ref.cc \
	mm3dreg.cc \
	mm3dport.cc \
	ms3dfilter.cc \
	msg.cc \
	objfilter.cc \
	texmgr.cc \
	texscale.cc \
	texture.cc \
	triprim.cc \
	translate.cc \
	undo.cc \
	undomgr.cc \
	weld.cc \
	pcxtex.cc \
	rawtex.cc \
	tgatex.cc \
	txtfilter.cc \
	$(libmm3d_HFILES)

#mm3d_LDADD = $(QGL_LIBS) $(QT_LIBS) $(LUALIB_LIBS) $(GL_LIBS) $(DLOPEN_LIBS)
#mm3d_LDFLAGS = $(all_libraries) $(QT_CXXFLAGS)

# FIXME Add -pg support to ./configure
AM_CPPFLAGS = $(CORE_PROFILE) -Wall -I.. -DMM3D_EDIT $(all_includes) $(GL_CFLAGS)

wc:
	wc `ls *.h *.cpp *.cc *.y *.l 2> /dev/null | grep -v moc.cc | grep -v "\.base\." ` | sort -n
