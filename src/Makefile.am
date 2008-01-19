SUBDIRS = libmm3d mm3dcore depui qtui implui tools commands pixmap

bin_PROGRAMS = mm3d

EXTRA_DIST = icon.rc maketags.sh

mm3d_HFILES = \
	stdcmds.h \
	stdfilters.h \
	stdtexfilters.h \
	stdtools.h 


mm3d_SOURCES = \
	3dm.cc \
	stdcmds.cc \
	stdfilters.cc \
	stdtexfilters.cc \
	stdtools.cc \
	$(mm3d_HFILES)

mm3d_LDADD = $(CORE_PROFILE) $(QGL_LIBS) $(QT_LIBS) $(LUALIB_LIBS) $(GL_LIBS) $(DLOPEN_LIBS) tools/libtools.a commands/libcommands.a implui/libimplui.a qtui/libqtui.a depui/libdepui.a mm3dcore/libmm3dcore.a libmm3d/libmm3d.a
mm3d_LDFLAGS = $(all_libraries) $(QT_CXXFLAGS)

AM_CPPFLAGS = $(CORE_PROFILE) -Wall -Ilibmm3d -Imm3dcore -Idepui -Iqtui -Iimplui -Itools -Icommands -DMM3D_EDIT $(all_includes) $(QT_CXXFLAGS) $(LUALIB_CCFLAGS) $(GL_CFLAGS)

%.moc.cc: %.h
		$(QT_MOC) -o $@ $<

wc:
	wc `ls *.h *.cpp *.cc *.y *.l */*.h */*.cpp */*.cc */*.y */*.l 2> /dev/null | grep -v moc.cc | grep -v "\.base\." ` | sort -n
