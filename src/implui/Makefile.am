
noinst_LIBRARIES = libimplui.a

libimplui_HFILES = \
	aboutwin.h \
	alignwin.h \
	animconvertwin.h \
	animeditwin.h \
	animexportwin.h \
	animsetwin.h \
	animwidget.h \
	animwin.h \
	autoassignjointwin.h \
	backgroundselect.h \
	backgroundwin.h \
	boolpanel.h \
	boolwin.h \
	cal3dprompt.h \
	contextgroup.h \
	contextinfluences.h \
	contextname.h \
	contextpanel.h \
	contextposition.h \
	contextprojection.h \
	contextrotation.h \
	extrudewin.h \
	groupclean.h \
	groupwin.h \
	helpwin.h \
	iqeprompt.h \
	jointwin.h \
	keycfg.h \
	keyvaluewin.h \
	licensewin.h \
	mapdirection.h \
	md3prompt.h \
	mergewin.h \
	metawin.h \
	ms3dprompt.h \
	msgqt.h \
	mview.h \
	newanim.h \
	objprompt.h \
	offsetwin.h \
	painttexturewin.h \
	paintwidget.h \
	pluginwin.h \
	pointwin.h \
	projectionwin.h \
	qtmain.h \
	qttex.h \
	smdprompt.h \
	spherifywin.h \
	statusbar.h \
	texturecoord.h \
	texwin.h \
	transformwin.h \
	transimp.h \
	valuewin.h \
	viewpanel.h \
	viewportsettings.h \
	viewwin.h 

libimplui_MOC = \
	aboutwin.moc.cc \
	alignwin.moc.cc \
	animconvertwin.moc.cc \
	animeditwin.moc.cc \
	animexportwin.moc.cc \
	animsetwin.moc.cc \
	animwidget.moc.cc \
	animwin.moc.cc \
	autoassignjointwin.moc.cc \
	backgroundselect.moc.cc \
	backgroundwin.moc.cc \
	boolpanel.moc.cc \
	boolwin.moc.cc \
	cal3dprompt.moc.cc \
	contextgroup.moc.cc \
	contextinfluences.moc.cc \
	contextname.moc.cc \
	contextpanel.moc.cc \
	contextposition.moc.cc \
	contextprojection.moc.cc \
	contextrotation.moc.cc \
	extrudewin.moc.cc \
	groupclean.moc.cc \
	groupwin.moc.cc \
	helpwin.moc.cc \
	iqeprompt.moc.cc \
	jointwin.moc.cc \
	pointwin.moc.cc \
	painttexturewin.moc.cc \
	projectionwin.moc.cc \
	keyvaluewin.moc.cc \
	licensewin.moc.cc \
	md3prompt.moc.cc \
	mergewin.moc.cc \
	metawin.moc.cc \
	ms3dprompt.moc.cc \
	mview.moc.cc \
	newanim.moc.cc \
	objprompt.moc.cc \
	offsetwin.moc.cc \
	pluginwin.moc.cc \
	smdprompt.moc.cc \
	spherifywin.moc.cc \
	statusbar.moc.cc \
	texturecoord.moc.cc \
	texwin.moc.cc \
	transformwin.moc.cc \
	valuewin.moc.cc \
	viewportsettings.moc.cc \
	viewpanel.moc.cc \
	viewwin.moc.cc 


BUILT_SOURCES = $(libimplui_MOC)

libimplui_a_SOURCES = \
	aboutwin.cc \
	alignwin.cc \
	animconvertwin.cc \
	animeditwin.cc \
	animexportwin.cc \
	animsetwin.cc \
	animwidget.cc \
	animwin.cc \
	autoassignjointwin.cc \
	backgroundselect.cc \
	backgroundwin.cc \
	boolpanel.cc \
	boolwin.cc \
	cal3dprompt.cc \
	contextgroup.cc \
	contextinfluences.cc \
	contextname.cc \
	contextpanel.cc \
	contextposition.cc \
	contextprojection.cc \
	contextrotation.cc \
	extrudewin.cc \
	groupclean.cc \
	groupwin.cc \
	helpwin.cc \
	iqeprompt.cc \
	jointwin.cc \
	keycfg.cc \
	keyvaluewin.cc \
	licensewin.cc \
	mapdirection.cc \
	md3prompt.cc \
	mergewin.cc \
	metawin.cc \
	ms3dprompt.cc \
	msgqt.cc \
	mview.cc \
	newanim.cc \
	objprompt.cc \
	offsetwin.cc \
	painttexturewin.cc \
	paintwidget.cc \
	pluginwin.cc \
	pointwin.cc \
	projectionwin.cc \
	qtmain.cc \
	qttex.cc \
	smdprompt.cc \
	spherifywin.cc \
	statusbar.cc \
	texturecoord.cc \
	texwin.cc \
	transformwin.cc \
	transimp.cc \
	valuewin.cc \
	viewpanel.cc \
	viewportsettings.cc \
	viewwin.cc \
	viewwin_influences.cc \
	$(libimplui_MOC) \
	$(libimplui_HFILES)

%.moc.cc: %.h
		$(QT_MOC) -o $@ $<

%.base.h: %.ui
		$(QT_UIC) -o $@ $(srcdir)/$*.ui

%.base.cc: $(mm3d_UIHEADERS) $(srcdir)/%.ui
		$(QT_UIC) -o $@ -impl $(builddir)/$*.base.h $(srcdir)/$*.ui

# -DQT_NO_CAST_ASCII
AM_CPPFLAGS = $(PROFILE) $(COVFLAGS) -Wall -I$(srcdir)/../libmm3d -I$(srcdir)/../mm3dcore -I$(srcdir)/../depui -I$(builddir)/../qtui -I$(srcdir)/../ -DMM3D_EDIT $(all_includes) $(QT_CXXFLAGS) $(LUALIB_CCFLAGS) $(GL_CFLAGS)

CLEANFILES = $(libimplui_MOC) $(mm3d_UIHEADERS) *.gcno *.gcda

