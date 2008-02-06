noinst_LIBRARIES = libcommands.a

libcommands_HFILES = \
	aligncmd.h \
	assignjointcmd.h \
	backgroundcmd.h \
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
	jointcmd.h \
	makefacecmd.h \
	pastecmd.h \
	pointcmd.h \
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
	assignjointcmd.cc \
	backgroundcmd.cc \
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
	jointcmd.cc \
	makefacecmd.cc \
	pastecmd.cc \
	pointcmd.cc \
	rotatetexcmd.cc \
	selectfreecmd.cc \
	simplifycmd.cc \
	snapcmd.cc \
	spherifycmd.cc \
	subdividecmd.cc \
	unweldcmd.cc \
	weldcmd.cc \
	$(libcommands_HFILES) 

AM_CPPFLAGS = $(CORE_PROFILE) -coverage -Wall -I../libmm3d -I../mm3dcore -I../depui -I../qtui -I../implui -I../ -DMM3D_EDIT $(all_includes) $(QT_CXXFLAGS) $(LUALIB_CCFLAGS) $(GL_CFLAGS)


CLEANFILES = *.gcno *.gcda
