noinst_LIBRARIES = libdepui.a

libdepui_HFILES = \
   errorobj.h \
   modelviewport.h \
   textureframe.h \
	texwidget.h 

libdepui_MOC = \
	errorobj.moc.cc \
	modelviewport.moc.cc \
   textureframe.moc.cc \
   texwidget.moc.cc 

libdepui_a_SOURCES = \
	errorobj.cc \
	modelviewport.cc \
	textureframe.cc \
   texwidget.cc \
	$(libdepui_MOC) \
	$(libdepui_HFILES)

%.moc.cc: %.h
		$(QT_MOC) -o $@ $<

AM_CPPFLAGS = $(CORE_PROFILE) $(COVFLAGS) -Wall -I$(srcdir)/../libmm3d -I$(srcdir)/../mm3dcore -I$(srcdir)/../ -DMM3D_EDIT $(all_includes) $(QT_CXXFLAGS) $(LUALIB_CCFLAGS) $(GL_CFLAGS)

CLEANFILES = $(libdepui_MOC) *.gcno *.gcda

