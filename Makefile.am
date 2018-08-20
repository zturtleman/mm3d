EXTRA_DIST = Makefile.generic \
	config.h.generic \
	cleanup.sh \
	mm3d-win32-installer.nsi \
	TRANSLATORS \
	INSTALL.WIN32 \
	mm3d.ico \
	mm3d.icns

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
<string>Misfit Model 3D</string>
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
	$(QT_MACDEPLOYQT) "$(MAC_APP)"

