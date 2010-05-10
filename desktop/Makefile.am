EXTRA_DIST = \
	mm3d.desktop \
	mm3d.xml \
	mm3d.png \
	mm3d-32.xpm \
	mm3d-16.xpm

all:

install:
	$(INSTALL) -d $(DESTDIR)$(datadir)/applications/
	$(INSTALL) -d $(DESTDIR)$(datadir)/mime/packages/
	$(INSTALL) -d $(DESTDIR)$(datadir)/pixmaps/
	${INSTALL} -m 0644 $(srcdir)/mm3d.desktop $(DESTDIR)$(datadir)/applications/
	${INSTALL} -m 0644 $(srcdir)/mm3d.xml $(DESTDIR)$(datadir)/mime/packages/
	${INSTALL} -m 0644 $(srcdir)/mm3d.png $(DESTDIR)$(datadir)/pixmaps/
	${INSTALL} -m 0644 $(srcdir)/mm3d-32.xpm $(DESTDIR)$(datadir)/pixmaps/
	${INSTALL} -m 0644 $(srcdir)/mm3d-16.xpm $(DESTDIR)$(datadir)/pixmaps/
