EXTRA_DIST = Makefile.generic \
	config.h.generic \
	cleanup.sh \
	mm3d-win32-installer.nsi \
	TRANSLATORS \
	INSTALL.WIN32 \
	install.bat \
	mm3d.reg 

SUBDIRS = src plugins i18n doc desktop man

DESTDIR = src

MAC_APP = Misfit Model 3D.app

define INFOPLIST
<?xml version=\"1.0\" encoding=\"UTF-8\"?>
<!DOCTYPE plist PUBLIC \"-//Apple//DTD PLIST 1.0//EN\" \"http://www.apple.com/DTDs/PropertyList-1.0.dtd\">
<plist version=\"1.0\">
<dict>
<key>CFBundleDevelopmentRegion</key>
<string>en</string>
<key>CFBundleExecutable</key>
<string>${PACKAGE}</string>
<key>CFBundleIconFile</key>
<string>${PACKAGE}</string>
<key>CFBundleIdentifier</key>
<string>com.misfitcode.${PACKAGE}</string>
<key>CFBundleInfoDictionaryVersion</key>
<string>6.0</string>
<key>CFBundleName</key>
<string>Misfit Model 3D</string>
<key>CFBundlePackageType</key>
<string>APPL</string>
<key>CFBundleShortVersionString</key>
<string>1.3.9</string>
<key>CFBundleSignature</key>
<string>????</string>
<key>CFBundleVersion</key>
<string>1.3.9</string>
<key>CGDisableCoalescedUpdates</key>
<true/>
<key>LSMinimumSystemVersion</key>
<string>${MACOSX_DEPLOYMENT_TARGET}</string>
<key>NSHumanReadableCopyright</key>
<string>Copyright © 2004-2008 Kevin Worcester.</string>
<key>NSPrincipalClass</key>
<string>NSApplication</string>
</dict>
</plist>
endef
export INFOPLIST

$(PACKAGE).app:
	@test -d "$(MAC_APP)/Contents/MacOS/" || mkdir -p "$(MAC_APP)/Contents/MacOS/"
	@test -d "$(MAC_APP)/Contents/Resources/" || mkdir -p "$(MAC_APP)/Contents/Resources/"
	@cp "$(DESTDIR)/$(PACKAGE)" "$(MAC_APP)/Contents/MacOS/$(PACKAGE)"
	@echo "APPL????" > "$(MAC_APP)/Contents/PkgInfo"
	@echo "$$INFOPLIST" > "$(MAC_APP)/Contents/Info.plist"
	@cp mm3d.icns "$(MAC_APP)/Contents/Resources/${PACKAGE}.icns"

