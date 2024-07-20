SUBDIRS = images

noinst_LIBRARIES = libqtui.a

libqtui_UIFILES = alignwin.ui \
	animconvertwin.ui \
	animeditwin.ui \
	animexportwin.ui \
	animsetwin.ui \
	animwidget.ui \
	autoassignjointwin.ui \
	backgroundselect.ui \
	backgroundwin.ui \
	boolwin.ui \
	cal3dprompt.ui \
	contextgroup.ui \
	contextinfluences.ui \
	contextname.ui \
	contextposition.ui \
	contextprojection.ui \
	contextrotation.ui \
	extrudewin.ui \
	groupwin.ui \
	groupclean.ui \
	helpwin.ui \
	iqeprompt.ui \
	jointwin.ui \
	offsetwin.ui \
	pointwin.ui \
	painttexturewin.ui \
	projectionwin.ui \
	keyvaluewin.ui \
	mapdirection.ui \
	md3prompt.ui \
	mergewin.ui \
	metawin.ui \
	modelview.ui \
	ms3dprompt.ui \
	newanim.ui \
	objprompt.ui \
	pluginwin.ui \
	smdprompt.ui \
	statusbar.ui \
	texturecoord.ui \
	textwin.ui \
	texwin.ui \
	transformwin.ui \
	valuewin.ui \
	viewportsettings.ui 

libqtui_UIHEADERS = \
   alignwin.base.h \
   animconvertwin.base.h \
   animeditwin.base.h \
   animexportwin.base.h \
	animsetwin.base.h   \
	animwidget.base.h      \
	autoassignjointwin.base.h      \
	backgroundselect.base.h \
	backgroundwin.base.h \
	boolwin.base.h      \
	cal3dprompt.base.h \
	contextgroup.base.h \
	contextinfluences.base.h \
	contextname.base.h \
	contextposition.base.h \
	contextprojection.base.h \
	contextrotation.base.h \
	extrudewin.base.h   \
	groupwin.base.h     \
	groupclean.base.h     \
	helpwin.base.h      \
	iqeprompt.base.h    \
	jointwin.base.h     \
	pointwin.base.h     \
	painttexturewin.base.h     \
	projectionwin.base.h     \
	keyvaluewin.base.h     \
	mapdirection.base.h \
	md3prompt.base.h    \
	mergewin.base.h     \
	metawin.base.h     \
	modelview.base.h    \
	ms3dprompt.base.h \
	newanim.base.h    \
	objprompt.base.h \
	offsetwin.base.h \
	pluginwin.base.h    \
	smdprompt.base.h    \
	statusbar.base.h    \
	texturecoord.base.h \
	textwin.base.h      \
	texwin.base.h       \
	transformwin.base.h       \
	valuewin.base.h \
	viewportsettings.base.h 

BUILT_SOURCES = $(libqtui_UIHEADERS) resources.cpp

libqtui_a_SOURCES = dummy.c resources.cpp

EXTRA_DIST = $(libqtui_UIFILES) resources.qrc

%.base.h: $(srcdir)/%.ui
		$(QT_UIC) -o $@ $(srcdir)/$*.ui

resources.cpp: $(srcdir)/resources.qrc $(srcdir)/images/zoomin.xpm $(srcdir)/images/zoomout.xpm
		$(QT_RCC) --name qtuiResources -o $@ $(srcdir)/resources.qrc

AM_CPPFLAGS = $(PROFILE) -Wall -I$(srcdir)/../libmm3d -I$(srcdir)/../mm3dcore -I$(srcdir)/../ -I$(srcdir)/../depui -DMM3D_EDIT $(all_includes) $(QT_CXXFLAGS) $(LUALIB_CCFLAGS) $(GL_CFLAGS)

CLEANFILES = $(libqtui_UIHEADERS) resources.cpp *.gcno *.gcda

