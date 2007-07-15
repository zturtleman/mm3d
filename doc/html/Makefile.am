SUBDIRS=olh_images

PAGES =  \
	olh_alignwin.html \
	olh_animconvertwin.html \
	olh_animexportwin.html \
	olh_animsetwin.html \
	olh_animwin.html \
	olh_autoassignjointwin.html \
	olh_backgroundwin.html \
	olh_cmdline.html \
	olh_commands.html \
	olh_develop.html \
	olh_quakemd2.html \
	olh_quakemd3.html \
	olh_groupwin.html \
	olh_index.html \
	olh_jointwin.html \
	olh_license.html \
	olh_mergewin.html \
	olh_metawin.html \
	olh_mm3dformat.html \
	olh_pluginwin.html \
	olh_projectionwin.html \
	olh_rgbawin.html \
	olh_scriptguide.html \
	olh_scriptref.html \
	olh_spherifywin.html \
	olh_texturecoordwin.html \
	olh_texturewin.html \
	olh_tools.html \
	olh_use.html \
	olh_valuewin.html \
	olh_viewportsettings.html \
	olh_viewwin.html

EXTRA_DIST = ${PAGES}

COMMON = template.htm
DOCDIR = ${datadir}/doc/mm3d/html

docs: ${PAGES}

%.html: %.page %.htm ${COMMON}
	hpagemake.pl $*.page > $*.html

install: all
	${INSTALL} -d      $(DESTDIR)${DOCDIR}/olh_images/tools
	${INSTALL} -m 0644 *.html $(DESTDIR)${DOCDIR}
	${INSTALL} -m 0644 olh_images/tools/*.png $(DESTDIR)${DOCDIR}/olh_images/tools
	${INSTALL} -m 0644 olh_images/tools/*.jpg $(DESTDIR)${DOCDIR}/olh_images/tools

