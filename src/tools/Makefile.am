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
	projtoolwidget.h \
	rectangletool.h \
	rotatetool.h \
	rotatetoolwidget.h \
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
	toolwidget.h \
	torustool.h \
	torustoolwidget.h \
	vertextool.h 

libtools_MOC = \
	cubetoolwidget.moc.cc \
	cylindertoolwidget.moc.cc \
	ellipsetoolwidget.moc.cc \
	polytoolwidget.moc.cc \
	projtoolwidget.moc.cc \
	rotatetoolwidget.moc.cc \
	scaletoolwidget.moc.cc \
	toolwidget.moc.cc \
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
	projtoolwidget.cc \
	movetool.cc \
	rotatetool.cc \
	rotatetoolwidget.cc \
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
	toolwidget.cc \
	torustool.cc \
	$(libtools_MOC) \
	$(libtools_HFILES) 

AM_CPPFLAGS = $(CORE_PROFILE) $(COVFLAGS) -Wall -I$(srcdir)/../libmm3d -I$(srcdir)/../mm3dcore -I$(srcdir)/../depui -I$(builddir)/../qtui -I$(srcdir)/../implui -I$(srcdir)/../ -DMM3D_EDIT $(all_includes) $(QT_CXXFLAGS) $(LUALIB_CCFLAGS) $(GL_CFLAGS)

%.moc.cc: %.h
		$(QT_MOC) -o $@ $<

CLEANFILES = $(libtools_MOC) *.gcno *.gcda
