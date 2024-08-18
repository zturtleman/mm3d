EXTRA_DIST = Makefile.generic \
	config.h.generic \
	cleanup.sh \
	mm3d-win32-installer.nsi \
	TRANSLATORS \
	INSTALL.WIN32 \
	mm3d.ico \
	mm3d.icns

# util is before doc because doc uses tool built in util.
SUBDIRS = src plugins translations util doc desktop man

DESTDIR = src

MAC_APP = Maverick Model 3D.app

define INFOPLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
<key>CFBundleDevelopmentRegion</key>
<string>en</string>
<key>UTExportedTypeDeclarations</key>
<array>
    <dict>
        <key>UTTypeConformsTo</key>
        <array>
            <string>public.data</string>
        </array>
        <key>UTTypeIdentifier</key>
        <string>moe.clover.mm3dmodel</string>
        <key>UTTypeDescription</key>
        <string>MM3D model</string>
        <key>UTTypeIconFile</key>
        <string>mm3d</string>
        <key>UTTypeReferenceURL</key>
        <string>https://clover.moe/mm3d_manual/olh_mm3dformat.html</string>
        <key>UTTypeTagSpecification</key>
        <dict>
            <key>public.filename-extension</key>
            <string>mm3d</string>
        </dict>
    </dict>
</array>
<key>CFBundleDocumentTypes</key>
<array>
    <dict>
        <key>CFBundleTypeName</key>
        <string>MM3D model</string>
        <key>LSItemContentTypes</key>
        <array>
            <string>moe.clover.mm3dmodel</string>
        </array>
        <key>CFBundleTypeRole</key>
        <string>Editor</string>
        <key>LSHandlerRank</key>
        <string>Owner</string>
    </dict>
    <dict>
        <key>CFBundleTypeName</key>
        <string>Quake 2 model</string>
        <key>CFBundleTypeExtensions</key>
        <array>
            <string>md2</string>
        </array>
        <key>CFBundleTypeRole</key>
        <string>Editor</string>
        <key>LSHandlerRank</key>
        <string>Alternate</string>
    </dict>
    <dict>
        <key>CFBundleTypeName</key>
        <string>Quake 3 model</string>
        <key>CFBundleTypeExtensions</key>
        <array>
            <string>md3</string>
        </array>
        <key>CFBundleTypeRole</key>
        <string>Editor</string>
        <key>LSHandlerRank</key>
        <string>Alternate</string>
    </dict>
    <dict>
        <key>CFBundleTypeName</key>
        <string>Milkshape 3D model</string>
        <key>CFBundleTypeExtensions</key>
        <array>
            <string>ms3d</string>
        </array>
        <key>CFBundleTypeRole</key>
        <string>Editor</string>
        <key>LSHandlerRank</key>
        <string>Alternate</string>
    </dict>
    <dict>
        <key>CFBundleTypeName</key>
        <string>Wavefront OBJ model</string>
        <key>CFBundleTypeExtensions</key>
        <array>
            <string>obj</string>
        </array>
        <key>CFBundleTypeRole</key>
        <string>Editor</string>
        <key>LSHandlerRank</key>
        <string>Alternate</string>
    </dict>
</array>
<key>CFBundleExecutable</key>
<string>${PACKAGE}</string>
<key>CFBundleIconFile</key>
<string>${PACKAGE}</string>
<key>CFBundleIdentifier</key>
<string>moe.clover.${PACKAGE}</string>
<key>CFBundleInfoDictionaryVersion</key>
<string>6.0</string>
<key>CFBundleName</key>
<string>Maverick Model 3D</string>
<key>CFBundlePackageType</key>
<string>APPL</string>
<key>CFBundleShortVersionString</key>
<string>${VERSION}</string>
<key>CFBundleSignature</key>
<string>????</string>
<key>CFBundleVersion</key>
<string>${VERSION}</string>
<key>CGDisableCoalescedUpdates</key>
<true/>
<key>LSMinimumSystemVersion</key>
<string>${MACOSX_DEPLOYMENT_TARGET}</string>
<key>NSHumanReadableCopyright</key>
<string>Copyright © 2004-2008 Kevin Worcester, Copyright © 2009-2024 Zack Middleton.</string>
<key>NSPrincipalClass</key>
<string>NSApplication</string>
</dict>
</plist>
endef
export INFOPLIST

define LOCVERSIONPLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>LprojCompatibleVersion</key>
    <string>123</string>
    <key>LprojLocale</key>
    <string>XXLANGXX</string>
    <key>LprojRevisionLevel</key>
    <string>1</string>
    <key>LprojVersion</key>
    <string>123</string>
</dict>
</plist>
endef
export LOCVERSIONPLIST

appbundle:
	${INSTALL} -d      "$(MAC_APP)/Contents/MacOS/"
	${INSTALL} -d      "$(MAC_APP)/Contents/Resources/"
	${INSTALL} -d      "$(MAC_APP)/Contents/PlugIns/mm3d/1.3"
	${INSTALL} -d      "$(MAC_APP)/Contents/SharedSupport/mm3d/doc/html/olh_images/screencaps/"
	${INSTALL} -d      "$(MAC_APP)/Contents/SharedSupport/mm3d/doc/html/olh_images/tools/"
	${INSTALL} -d      "$(MAC_APP)/Contents/SharedSupport/mm3d/translations/"
	${INSTALL} -m 0755 "$(DESTDIR)/$(PACKAGE)" "$(MAC_APP)/Contents/MacOS/$(PACKAGE)"
	${INSTALL} -m 0644 $(top_srcdir)/mm3d.icns "$(MAC_APP)/Contents/Resources/${PACKAGE}.icns"
	${INSTALL} -m 0644 doc/html/*.html "$(MAC_APP)/Contents/SharedSupport/mm3d/doc/html/"
	${INSTALL} -m 0644 $(top_srcdir)/doc/html/olh_images/screencaps/*.png "$(MAC_APP)/Contents/SharedSupport/mm3d/doc/html/olh_images/screencaps/"
	${INSTALL} -m 0644 $(top_srcdir)/doc/html/olh_images/tools/*.png "$(MAC_APP)/Contents/SharedSupport/mm3d/doc/html/olh_images/tools/"
	${INSTALL} -m 0644 $(top_srcdir)/doc/html/olh_images/tools/*.jpg "$(MAC_APP)/Contents/SharedSupport/mm3d/doc/html/olh_images/tools/"
	${INSTALL} -m 0644 translations/*.qm "$(MAC_APP)/Contents/SharedSupport/mm3d/translations/"
	echo "APPL????" > "$(MAC_APP)/Contents/PkgInfo"
	@echo "$$INFOPLIST" > "$(MAC_APP)/Contents/Info.plist"
	@echo "Wrote \"$(MAC_APP)/Contents/Info.plist\""
	$(QT_MACDEPLOYQT) "$(MAC_APP)"
	@# Add translations of Qt itself
	$(QT_LCONVERT) -o "$(MAC_APP)/Contents/SharedSupport/mm3d/translations/qt_de.qm" "$(QT_TRANSLATIONS_DIR)/qtbase_de.qm"
	$(QT_LCONVERT) -o "$(MAC_APP)/Contents/SharedSupport/mm3d/translations/qt_fr.qm" "$(QT_TRANSLATIONS_DIR)/qtbase_fr.qm"
	$(QT_LCONVERT) -o "$(MAC_APP)/Contents/SharedSupport/mm3d/translations/qt_sk.qm" "$(QT_TRANSLATIONS_DIR)/qtbase_sk.qm"
	@# Tell macOS to translate builtin dialogs (i.e., file chooser)
	${INSTALL} -d      "$(MAC_APP)/Contents/Resources/en.lproj"
	${INSTALL} -d      "$(MAC_APP)/Contents/Resources/de.lproj"
	${INSTALL} -d      "$(MAC_APP)/Contents/Resources/fr.lproj"
	${INSTALL} -d      "$(MAC_APP)/Contents/Resources/sk.lproj"
	@echo "$$LOCVERSIONPLIST" | sed "s/XXLANGXX/en/g" > "$(MAC_APP)/Contents/Resources/en.lproj/locversion.plist"
	@echo "$$LOCVERSIONPLIST" | sed "s/XXLANGXX/de/g" > "$(MAC_APP)/Contents/Resources/de.lproj/locversion.plist"
	@echo "$$LOCVERSIONPLIST" | sed "s/XXLANGXX/fr/g" > "$(MAC_APP)/Contents/Resources/fr.lproj/locversion.plist"
	@echo "$$LOCVERSIONPLIST" | sed "s/XXLANGXX/sk/g" > "$(MAC_APP)/Contents/Resources/sk.lproj/locversion.plist"

