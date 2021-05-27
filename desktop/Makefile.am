EXTRA_DIST = \
	mm3d.desktop \
	mm3d.xml \
	mm3d.png \
	mm3d-32.xpm \
	mm3d-16.xpm

all:

install:
	$(INSTALL) -d $(DESTDIR)$(datadir)/applications/
	$(INSTALL) -d $(DESTDIR)$(datadir)/metainfo/
	$(INSTALL) -d $(DESTDIR)$(datadir)/mime/packages/
	$(INSTALL) -d $(DESTDIR)$(datadir)/icons/hicolor/128x128/apps/
	$(INSTALL) -d $(DESTDIR)$(datadir)/icons/hicolor/64x64/apps/
	$(INSTALL) -d $(DESTDIR)$(datadir)/icons/hicolor/32x32/apps/
	$(INSTALL) -d $(DESTDIR)$(datadir)/icons/hicolor/16x16/apps/
	${INSTALL} -m 0644 $(srcdir)/moe.clover.mm3d.desktop $(DESTDIR)$(datadir)/applications/
	${INSTALL} -m 0644 $(srcdir)/moe.clover.mm3d.metainfo.xml $(DESTDIR)$(datadir)/metainfo/
	${INSTALL} -m 0644 $(srcdir)/moe.clover.mm3d.mm3dmodel.xml $(DESTDIR)$(datadir)/mime/packages/
	${INSTALL} -m 0644 $(srcdir)/moe.clover.mm3d.png $(DESTDIR)$(datadir)/icons/hicolor/128x128/apps/moe.clover.mm3d.png
	${INSTALL} -m 0644 $(srcdir)/moe.clover.mm3d-64.png $(DESTDIR)$(datadir)/icons/hicolor/64x64/apps/moe.clover.mm3d.png
	${INSTALL} -m 0644 $(srcdir)/moe.clover.mm3d-32.png $(DESTDIR)$(datadir)/icons/hicolor/32x32/apps/moe.clover.mm3d.png
	${INSTALL} -m 0644 $(srcdir)/moe.clover.mm3d-16.png $(DESTDIR)$(datadir)/icons/hicolor/16x16/apps/moe.clover.mm3d.png
