
noinst_LIBRARIES = libqtui.a

libqtui_UIFILES = alignwin.ui \
	animconvertwin.ui \
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
	helpwin.ui \
	jointwin.ui \
	pointwin.ui \
	painttexturewin.ui \
	projectionwin.ui \
	keyvaluewin.ui \
	mapdirection.ui \
	mergewin.ui \
	metawin.ui \
	modelview.ui \
	ms3dprompt.ui \
	newanim.ui \
	objprompt.ui \
	pluginwin.ui \
	rgbawin.ui \
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
	helpwin.base.h      \
	jointwin.base.h     \
	pointwin.base.h     \
	painttexturewin.base.h     \
	projectionwin.base.h     \
	keyvaluewin.base.h     \
	mapdirection.base.h \
	mergewin.base.h     \
	metawin.base.h     \
	modelview.base.h    \
	ms3dprompt.base.h \
	newanim.base.h    \
	objprompt.base.h \
	pluginwin.base.h    \
	rgbawin.base.h      \
	statusbar.base.h    \
	texturecoord.base.h \
	textwin.base.h      \
	texwin.base.h       \
	transformwin.base.h       \
	valuewin.base.h \
	viewportsettings.base.h 

BUILT_SOURCES = $(libqtui_UIHEADERS)

libqtui_a_SOURCES = $(libqtui_UIFILES)

%.base.h: %.ui
		$(QT_UIC) -o $@ $*.ui

AM_CPPFLAGS = $(PROFILE) -DQT_COMPAT -Wall -I../libmm3d -I../mm3dcore -I../ -I../depui -DMM3D_EDIT $(all_includes) $(QT_CXXFLAGS) $(LUALIB_CCFLAGS) $(GL_CFLAGS)

CLEANFILES = $(libqtui_UIHEADERS)

