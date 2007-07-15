
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

libqtui_UI = alignwin.base.cc \
   animconvertwin.base.cc \
   animexportwin.base.cc \
	animsetwin.base.cc \
	animwidget.base.cc \
	autoassignjointwin.base.cc \
	backgroundselect.base.cc \
	backgroundwin.base.cc \
	boolwin.base.cc \
	contextgroup.base.cc \
	contextinfluences.base.cc \
	contextname.base.cc \
	contextposition.base.cc \
	contextprojection.base.cc \
	contextrotation.base.cc \
	extrudewin.base.cc \
	groupwin.base.cc \
	helpwin.base.cc \
	jointwin.base.cc \
	pointwin.base.cc \
	painttexturewin.base.cc \
	projectionwin.base.cc \
	keyvaluewin.base.cc \
	mapdirection.base.cc \
	mergewin.base.cc \
	metawin.base.cc \
	modelview.base.cc \
	newanim.base.cc \
	objprompt.base.cc \
	pluginwin.base.cc \
	rgbawin.base.cc \
	statusbar.base.cc \
	texturecoord.base.cc \
	textwin.base.cc \
	texwin.base.cc \
	transformwin.base.cc \
	valuewin.base.cc \
	viewportsettings.base.cc 

libqtui_MOC = alignwin.base.moc.cc \
   animconvertwin.base.moc.cc \
   animexportwin.base.moc.cc \
	animsetwin.base.moc.cc \
	animwidget.base.moc.cc \
	autoassignjointwin.base.moc.cc \
	backgroundselect.base.moc.cc \
	backgroundwin.base.moc.cc \
	boolwin.base.moc.cc \
	contextgroup.base.moc.cc \
	contextinfluences.base.moc.cc \
	contextname.base.moc.cc \
	contextposition.base.moc.cc \
	contextprojection.base.moc.cc \
	contextrotation.base.moc.cc \
	extrudewin.base.moc.cc \
	groupwin.base.moc.cc \
	helpwin.base.moc.cc \
	jointwin.base.moc.cc \
	pointwin.base.moc.cc \
	painttexturewin.base.moc.cc \
	projectionwin.base.moc.cc \
	keyvaluewin.base.moc.cc \
	mapdirection.base.moc.cc \
	mergewin.base.moc.cc \
	metawin.base.moc.cc \
	modelview.base.moc.cc \
	newanim.base.moc.cc \
	objprompt.base.moc.cc \
	pluginwin.base.moc.cc \
	rgbawin.base.moc.cc \
	statusbar.base.moc.cc \
	texturecoord.base.moc.cc \
	textwin.base.moc.cc \
	texwin.base.moc.cc \
	transformwin.base.moc.cc \
	valuewin.base.moc.cc \
	viewportsettings.base.moc.cc 

BUILT_SOURCES = $(libqtui_UIHEADERS) $(libqtui_UI) $(libqtui_MOC)

libqtui_a_SOURCES = $(libqtui_UIFILES) $(libqtui_UI) $(libqtui_MOC)

%.moc.cc: %.h
		$(QT_MOC) -o $@ $<

%.base.h: %.ui
		$(QT_UIC) -o $@ $*.ui

%.base.cc: $(mm3d_UIHEADERS) %.ui
		$(QT_UIC) -o $@ -impl $*.base.h $*.ui

AM_CPPFLAGS = -DQT_COMPAT -Wall -I../libmm3d -I../mm3dcore -I../ -I../depui -DMM3D_EDIT $(all_includes) $(QT_CXXFLAGS) $(LUALIB_CCFLAGS) $(GL_CFLAGS)

CLEANFILES = $(libqtui_UI) $(libqtui_MOC) $(libqtui_UIHEADERS)

