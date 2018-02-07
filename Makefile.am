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
<string>moe.clover.${PACKAGE}</string>
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
<string>Copyright © 2004-2008 Kevin Worcester, clover.moe fork © 2009-2018 Zack Middleton.</string>
<key>NSPrincipalClass</key>
<string>NSApplication</string>
</dict>
</plist>
endef
export INFOPLIST

$(PACKAGE).app:
	${INSTALL} -d      "$(MAC_APP)/Contents/MacOS/"
	${INSTALL} -d      "$(MAC_APP)/Contents/Resources/"
	${INSTALL} -d      "$(MAC_APP)/Contents/PlugIns/mm3d/1.3"
	${INSTALL} -d      "$(MAC_APP)/Contents/SharedSupport/mm3d/doc/html/olh_images/screencaps/"
	${INSTALL} -d      "$(MAC_APP)/Contents/SharedSupport/mm3d/doc/html/olh_images/tools/"
	${INSTALL} -d      "$(MAC_APP)/Contents/SharedSupport/mm3d/i18n/"
	${INSTALL} -m 0755 "$(DESTDIR)/$(PACKAGE)" "$(MAC_APP)/Contents/MacOS/$(PACKAGE)"
	${INSTALL} -m 0644 mm3d.icns "$(MAC_APP)/Contents/Resources/${PACKAGE}.icns"
	${INSTALL} -m 0644 doc/html/*.html "$(MAC_APP)/Contents/SharedSupport/mm3d/doc/html/"
	${INSTALL} -m 0644 doc/html/olh_images/screencaps/*.png "$(MAC_APP)/Contents/SharedSupport/mm3d/doc/html/olh_images/screencaps/"
	${INSTALL} -m 0644 doc/html/olh_images/tools/*.png "$(MAC_APP)/Contents/SharedSupport/mm3d/doc/html/olh_images/tools/"
	${INSTALL} -m 0644 doc/html/olh_images/tools/*.jpg "$(MAC_APP)/Contents/SharedSupport/mm3d/doc/html/olh_images/tools/"
	${INSTALL} -m 0644 i18n/*.qm "$(MAC_APP)/Contents/SharedSupport/mm3d/i18n/"
	echo "APPL????" > "$(MAC_APP)/Contents/PkgInfo"
	@echo "$$INFOPLIST" > "$(MAC_APP)/Contents/Info.plist"
	@echo "Wrote \"$(MAC_APP)/Contents/Info.plist\""

