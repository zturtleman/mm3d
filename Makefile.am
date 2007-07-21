EXTRA_DIST = Makefile.mingw \
	config.h.mingw \
	cleanup.sh \
	mm3d-win32-installer.nsi \
	TRANSLATORS \
	INSTALL.WIN32 \
	install.bat \
	mm3d.reg 

SUBDIRS = src plugins i18n doc desktop

DESTDIR = src

MAC_APP = $(PACKAGE).app

$(MAC_APP): $(MAC_APP)/Contents/PkgInfo $(MAC_APP)/Contents/Info.plist
	@cp $(DESTDIR)/$(PACKAGE) $(MAC_APP)/Contents/MacOS/$(PACKAGE)
	
$(MAC_APP)/Contents/PkgInfo:
	@test -d $(MAC_APP)/Contents/MacOS/ || mkdir -p $(MAC_APP)/Contents/MacOS/
	@rm -f $(MAC_APP)/Contents/PkgInfo
	@echo "APPL????" > $(MAC_APP)/Contents/PkgInfo
$(MAC_APP)/Contents/Info.plist:
	@test -d $(MAC_APP)/Contents/MacOS/ || mkdir -p $(MAC_APP)/Contents/MacOS/
	@rm -f $(MAC_APP)/Contents/Info.plist
	@echo "" > $(MAC_APP)/Contents/Info.plist
	@defaults write $(PWD)/$(MAC_APP)/Contents/Info '{"CFBundlePackageType"=APPL;}';
	@defaults write $(PWD)/$(MAC_APP)/Contents/Info "CFBundleSignature" "????"
	@defaults write $(PWD)/$(MAC_APP)/Contents/Info "CFBundleExecutable" $(PACKAGE)
