noinst_LIBRARIES = libtools.a

libtools_HFILES = \
	atrfartool.h \
	atrneartool.h \
	bgmovetool.h \
	bgscaletool.h \
	cubetool.h \
	cubetoolwidget.h \
	cylindertool.h \
	cylindertoolwidget.h \
	dragvertextool.h \
	ellipsetool.h \
	ellipsetoolwidget.h \
	extrudetool.h \
	jointtool.h \
	pointtool.h \
	projtool.h \
	movetool.h \
	polytool.h \
	polytoolwidget.h \
	rectangletool.h \
	rotatetool.h \
	scaletool.h \
	scaletoolwidget.h \
	selectbonetool.h \
	selectpointtool.h \
	selectprojtool.h \
	selectconnectedtool.h \
	selectfacetool.h \
	selectfacetoolwidget.h \
	selectgrouptool.h \
	selectvertextool.h \
	sheartool.h \
	torustool.h \
	torustoolwidget.h \
	vertextool.h 

libtools_MOC = \
	cubetoolwidget.moc.cc \
	cylindertoolwidget.moc.cc \
	ellipsetoolwidget.moc.cc \
	polytoolwidget.moc.cc \
	scaletoolwidget.moc.cc \
	torustoolwidget.moc.cc \
	selectfacetoolwidget.moc.cc 

BUILT_SOURCES = $(libtools_MOC)

libtools_a_SOURCES = \
	vertextool.cc \
	atrfartool.cc \
	atrneartool.cc \
	bgmovetool.cc \
	bgscaletool.cc \
	cubetool.cc \
	cubetoolwidget.cc \
	cylindertool.cc \
	cylindertoolwidget.cc \
	dragvertextool.cc \
	ellipsetool.cc \
	ellipsetoolwidget.cc \
	extrudetool.cc \
	polytool.cc \
	polytoolwidget.cc \
	jointtool.cc \
	pointtool.cc \
	projtool.cc \
	movetool.cc \
	rotatetool.cc \
	scaletool.cc \
	scaletoolwidget.cc \
	torustoolwidget.cc \
	selectvertextool.cc \
	selectconnectedtool.cc \
	selectfacetool.cc \
	selectfacetoolwidget.cc \
	selectgrouptool.cc \
	selectbonetool.cc \
	selectpointtool.cc \
	selectprojtool.cc \
	sheartool.cc \
	rectangletool.cc \
	torustool.cc \
	$(libtools_MOC) \
	$(libtools_HFILES) 

AM_CPPFLAGS = -DQT_COMPAT -Wall -I../libmm3d -I../mm3dcore -I../depui -I../qtui -I../implui -I../ -DMM3D_EDIT $(all_includes) $(QT_CXXFLAGS) $(LUALIB_CCFLAGS) $(GL_CFLAGS)

%.moc.cc: %.h
		$(QT_MOC) -o $@ $<

CLEANFILES = $(libtools_MOC)
