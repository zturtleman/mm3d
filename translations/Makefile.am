QMFILES = \
	mm3d_de.qm \
	mm3d_fr.qm \
	mm3d_sk.qm \
	mm3d_ref.qm \
	mm3d_bork.qm

TSFILES = \
	$(srcdir)/mm3d_ref.ts \
	$(srcdir)/mm3d_de.ts \
	$(srcdir)/mm3d_fr.ts \
	$(srcdir)/mm3d_sk.ts \
	$(srcdir)/mm3d_bork.ts

EXTRA_DIST = \
	$(QMFILES) \
	$(TSFILES)

BUILT_SOURCES = $(QMFILES)
	
all:

tsfiles:
	lupdate $(srcdir)/../src/*.cc $(srcdir)/../src/*/*.cc $(srcdir)/../src/*.h $(srcdir)/../src/*/*.h $(srcdir)/../src/qtui/*.ui -ts $(TSFILES)

qmfiles: $(QMFILES)

%.qm: %.ts
	$(QT_LRELEASE) $< -qm $@

install:
	$(INSTALL) -d $(DESTDIR)$(datadir)/mm3d/translations
	${INSTALL} -m 0644 $(QMFILES) $(DESTDIR)$(datadir)/mm3d/translations

CLEANFILES = $(QMFILES)
