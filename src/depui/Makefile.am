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

AM_CPPFLAGS = -DQT_COMPAT -Wall -I../libmm3d -I../mm3dcore -I../ -DMM3D_EDIT $(all_includes) $(QT_CXXFLAGS) $(LUALIB_CCFLAGS) $(GL_CFLAGS)

CLEANFILES = $(libdepui_MOC)




