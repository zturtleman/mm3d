SUBDIRS=olh_images

SPAGES =  \
	olh_detailsindex.htm \
	olh_vertexdetails.htm \
	olh_facedetails.htm \
	olh_meshdetails.htm \
	olh_normaldetails.htm \
	olh_groupdetails.htm \
	olh_materialdetails.htm \
	olh_texturecoorddetails.htm \
	olh_projectiondetails.htm \
	olh_jointdetails.htm \
	olh_pointdetails.htm \
	olh_animationdetails.htm \
	olh_backgroundimagedetails.htm \
	olh_alignwin.htm \
	olh_animconvertwin.htm \
	olh_animexportwin.htm \
	olh_animsetwin.htm \
	olh_animwin.htm \
	olh_autoassignjointwin.htm \
	olh_backgroundwin.htm \
	olh_boolwin.htm \
	olh_cmdline.htm \
	olh_commands.htm \
	olh_develop.htm \
	olh_quakemd2.htm \
	olh_quakemd3.htm \
	olh_cal3d.htm \
	olh_groupwin.htm \
	olh_index.htm \
	olh_influences.htm \
	olh_jointwin.htm \
	olh_license.htm \
	olh_mergewin.htm \
	olh_metawin.htm \
	olh_mm3dformat.htm \
	olh_cal3dprompt.htm \
	olh_ms3dprompt.htm \
	olh_objprompt.htm \
	olh_iqeprompt.htm \
	olh_smdprompt.htm \
	olh_painttexturewin.htm \
	olh_pointwin.htm \
	olh_pluginwin.htm \
	olh_projectionwin.htm \
	olh_properties.htm \
	olh_spherifywin.htm \
	olh_texturecoordwin.htm \
	olh_texturewin.htm \
	olh_tips.htm \
	olh_tools.htm \
	olh_transformwin.htm \
	olh_tutorial_index.htm \
	olh_viewportsettings.htm \
	olh_whatsnew.htm \
	olh_mainwin.htm

PAGES =  \
	olh_detailsindex.html \
	olh_vertexdetails.html \
	olh_facedetails.html \
	olh_meshdetails.html \
	olh_normaldetails.html \
	olh_groupdetails.html \
	olh_materialdetails.html \
	olh_texturecoorddetails.html \
	olh_projectiondetails.html \
	olh_jointdetails.html \
	olh_pointdetails.html \
	olh_animationdetails.html \
	olh_backgroundimagedetails.html \
	olh_alignwin.html \
	olh_animconvertwin.html \
	olh_animexportwin.html \
	olh_animsetwin.html \
	olh_animwin.html \
	olh_autoassignjointwin.html \
	olh_backgroundwin.html \
	olh_boolwin.html \
	olh_cmdline.html \
	olh_commands.html \
	olh_develop.html \
	olh_quakemd2.html \
	olh_quakemd3.html \
	olh_cal3d.html \
	olh_groupwin.html \
	olh_index.html \
	olh_influences.html \
	olh_jointwin.html \
	olh_license.html \
	olh_mergewin.html \
	olh_metawin.html \
	olh_mm3dformat.html \
	olh_cal3dprompt.html \
	olh_ms3dprompt.html \
	olh_objprompt.html \
	olh_iqeprompt.html \
	olh_smdprompt.html \
	olh_painttexturewin.html \
	olh_pointwin.html \
	olh_pluginwin.html \
	olh_projectionwin.html \
	olh_properties.html \
	olh_spherifywin.html \
	olh_texturecoordwin.html \
	olh_texturewin.html \
	olh_tips.html \
	olh_tools.html \
	olh_tutorial_index.html \
	olh_transformwin.html \
	olh_viewportsettings.html \
	olh_whatsnew.html \
	olh_mainwin.html

EXTRA_DIST = ${SPAGES} ${PAGES}

COMMON = template.htm
DOCDIR = ${datadir}/doc/mm3d/html

all: docs

docs: ${PAGES}

%.html: %.page %.htm ${COMMON}
	$(top_srcdir)/util/hpagemake.pl $(srcdir)/$*.page > $*.html

install: all
	${INSTALL} -d      $(DESTDIR)${DOCDIR}/olh_images/screencaps
	${INSTALL} -d      $(DESTDIR)${DOCDIR}/olh_images/tools
	${INSTALL} -m 0644 $(PAGES) $(DESTDIR)${DOCDIR}
	${INSTALL} -m 0644 $(srcdir)/olh_images/screencaps/*.png $(DESTDIR)${DOCDIR}/olh_images/screencaps
	${INSTALL} -m 0644 $(srcdir)/olh_images/tools/*.png $(DESTDIR)${DOCDIR}/olh_images/tools
	${INSTALL} -m 0644 $(srcdir)/olh_images/tools/*.jpg $(DESTDIR)${DOCDIR}/olh_images/tools

