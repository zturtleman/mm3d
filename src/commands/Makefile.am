noinst_LIBRARIES = libcommands.a

libcommands_HFILES = \
	aligncmd.h \
	capcmd.h \
	copycmd.h \
	deletecmd.h \
	dupcmd.h \
	edgedivcmd.h \
	edgeturncmd.h \
	extrudecmd.h \
	faceoutcmd.h \
	flattencmd.h \
	flipcmd.h \
	hidecmd.h \
	invertcmd.h \
	invnormalcmd.h \
	makefacecmd.h \
	offsetcmd.h \
	pastecmd.h \
	rotatetexcmd.h \
	selectfreecmd.h \
	simplifycmd.h \
	snapcmd.h \
	spherifycmd.h \
	subdividecmd.h \
	unweldcmd.h \
	weldcmd.h


libcommands_a_SOURCES = \
	aligncmd.cc \
	capcmd.cc \
	copycmd.cc \
	deletecmd.cc \
	dupcmd.cc \
	edgedivcmd.cc \
	edgeturncmd.cc \
	extrudecmd.cc \
	faceoutcmd.cc \
	flattencmd.cc \
	flipcmd.cc \
	hidecmd.cc \
	invertcmd.cc \
	invnormalcmd.cc \
	makefacecmd.cc \
	offsetcmd.cc \
	pastecmd.cc \
	rotatetexcmd.cc \
	selectfreecmd.cc \
	simplifycmd.cc \
	snapcmd.cc \
	spherifycmd.cc \
	subdividecmd.cc \
	unweldcmd.cc \
	weldcmd.cc \
	$(libcommands_HFILES) 

AM_CPPFLAGS = $(CORE_PROFILE) $(COVFLAGS) -Wall -I$(srcdir)/../libmm3d -I$(srcdir)/../mm3dcore -I$(srcdir)/../depui -I$(builddir)/../qtui -I$(srcdir)/../implui -I$(srcdir)/../ -DMM3D_EDIT $(all_includes) $(QT_CXXFLAGS) $(LUALIB_CCFLAGS) $(GL_CFLAGS)


CLEANFILES = *.gcno *.gcda
