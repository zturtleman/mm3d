EXTRA_DIST = \
	mm3d.desktop \
	x-mm3d.desktop \
	mm3d.xpm \
	mm3d-16.xpm

all:

install:
	$(INSTALL) -d $(DESTDIR)$(datadir)/applications/
	$(INSTALL) -d $(DESTDIR)$(datadir)/mimelnk/application/
	$(INSTALL) -d $(DESTDIR)$(datadir)/pixmaps/
	${INSTALL} -m 0644 mm3d.desktop $(DESTDIR)$(datadir)/applications/
	${INSTALL} -m 0644 x-mm3d.desktop $(DESTDIR)$(datadir)/mimelnk/application/
	${INSTALL} -m 0644 *.xpm $(DESTDIR)$(datadir)/pixmaps/

