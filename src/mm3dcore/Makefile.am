
noinst_LIBRARIES = libmm3dcore.a

libmm3dcore_HFILES = \
	3dmprefs.h \
	align.h \
	allocstats.h \
	bounding.h \
	cmdline.h \
	cmdmgr.h \
	command.h \
	contextpanelobserver.h \
	contextwidget.h \
	decal.h \
	decalmgr.h \
	luaif.h \
	luascript.h \
	menuconf.h \
	modeltest.h \
	pluginapi.h \
	pluginmgr.h \
	prefparse.h \
	prefs.h \
	rotatepoint.h \
	scriptif.h \
	suborigin.h \
	sysconf.h \
	texturetest.h \
	tool.h \
	toolbox.h \
	toolpoly.h \
	version.h 

libmm3dcore_a_SOURCES = \
	3dmprefs.cc \
	align.cc \
	allocstats.cc \
	bounding.cc \
	cmdline.cc \
	cmdmgr.cc \
	command.cc \
	contextpanelobserver.cc \
	contextwidget.cc \
	decal.cc \
	decalmgr.cc \
	luaif.cc \
	luascript.cc \
	modeltest.cc \
	pluginmgr.cc \
	prefparse.cc \
	prefs.cc \
	rotatepoint.cc \
	scriptif.cc \
	sysconf.cc \
	texturetest.cc \
	tool.cc \
	toolbox.cc \
	toolpoly.cc \
	$(libmm3dcore_HFILES)

#AM_CPPFLAGS = -Wall -I../libmm3d -I../ -DMM3D_EDIT $(all_includes) $(QT_CXXFLAGS) $(LUALIB_CCFLAGS) $(GL_CFLAGS)
AM_CPPFLAGS = $(CORE_PROFILE) -Wall -I../libmm3d -I../ -DMM3D_EDIT $(all_includes) $(LUALIB_CCFLAGS) $(GL_CFLAGS)


