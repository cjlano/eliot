SUBDIRS = intl dic game utils qt po extras

ACLOCAL_AMFLAGS = -I m4

WIN32_PACKAGE_DIR = $(top_builddir)/eliot-$(VERSION)
MACOSX_PACKAGE_DIR = $(top_builddir)/eliot-$(VERSION)/Eliot.app

DMG_FILE = $(top_builddir)/eliot-$(VERSION).dmg

.PHONY: package-win32-zip package-macosx

EXTRA_DIST = COPYING.arabica LGPL-2.0.txt

# Prepare the win32 package
package-win32-zip:
# Remove previous stuff
	rm -rf $(WIN32_PACKAGE_DIR)
	rm -f eliot-$(VERSION)-win32.zip

	mkdir -p $(WIN32_PACKAGE_DIR)

# Copy the binaries and strip them
	cp $(top_builddir)/dic/compdic$(EXEEXT) $(top_builddir)/dic/listdic$(EXEEXT) $(top_builddir)/qt/eliot$(EXEEXT) $(WIN32_PACKAGE_DIR)
	$(STRIP) $(WIN32_PACKAGE_DIR)/*

# Copy the i18n files
	cat $(top_srcdir)/po/LINGUAS | while read lang; do \
		mkdir -p $(WIN32_PACKAGE_DIR)/locale/$${lang}/LC_MESSAGES; \
		cp $(top_srcdir)/po/$${lang}.gmo $(WIN32_PACKAGE_DIR)/locale/$${lang}/LC_MESSAGES/eliot.mo; \
	done
	mkdir -p $(WIN32_PACKAGE_DIR)/locale/qt4
	cp $(QT4LOCALEDIR)/*.qm $(WIN32_PACKAGE_DIR)/locale/qt4/ || true

# Copy relevant text files
	for i in AUTHORS COPYING COPYING.arabica NEWS THANKS; do \
		cp $(top_srcdir)/$$i $(WIN32_PACKAGE_DIR)/$$i.txt; \
		todos $(WIN32_PACKAGE_DIR)/$$i.txt; \
	done

# Copy the mingw DLL
	gunzip -c /usr/share/doc/mingw32-runtime/mingwm10.dll.gz > $(WIN32_PACKAGE_DIR)/mingwm10.dll

# Zip the temporary dir and remove it
	zip -r -9 eliot-$(VERSION)-win32.zip $(WIN32_PACKAGE_DIR)
	rm -rf $(WIN32_PACKAGE_DIR)


# Prepare the macosx package
package-macosx:
# Remove previous stuff
	rm -rf $(top_builddir)/eliot-$(VERSION)

# Prepare the bundle structure
	mkdir -p $(MACOSX_PACKAGE_DIR)/Contents/MacOS
	mkdir -p $(MACOSX_PACKAGE_DIR)/Contents/Resources
	cp $(top_srcdir)/extras/macosx/Info.plist $(MACOSX_PACKAGE_DIR)/Contents
	cp $(top_srcdir)/extras/macosx/eliot-64.icns $(MACOSX_PACKAGE_DIR)/Contents/Resources

# Copy the binaries and strip them
	cp $(top_builddir)/qt/eliot$(EXEEXT) $(MACOSX_PACKAGE_DIR)/Contents/MacOS/Eliot
	strip $(MACOSX_PACKAGE_DIR)/Contents/MacOS/Eliot

# Copy the i18n files
	cat $(top_srcdir)/po/LINGUAS | while read lang; do \
		mkdir -p $(MACOSX_PACKAGE_DIR)/locale/$${lang}/LC_MESSAGES; \
		cp $(top_srcdir)/po/$${lang}.gmo $(MACOSX_PACKAGE_DIR)/locale/$${lang}/LC_MESSAGES/eliot.mo; \
	done
	mkdir -p $(MACOSX_PACKAGE_DIR)/locale/qt4
	cp $(QT4LOCALEDIR)/qt_*.qm $(MACOSX_PACKAGE_DIR)/locale/qt4/ || true

# Copy the shared libraries (up to 3 levels of recursive dependencies)
	mkdir -p $(MACOSX_PACKAGE_DIR)/Contents/Frameworks
	otool -L $(MACOSX_PACKAGE_DIR)/Contents/MacOS/Eliot | grep /opt/local/ | sed 's/ (.*//' > dylib.list1
	otool -L `cat dylib.list1` | grep /opt/local/ | grep " (" | sed 's/ (.*//' | sort -u > dylib.list2
	otool -L `cat dylib.list2` | grep /opt/local/ | grep " (" | sed 's/ (.*//' | sort -u > dylib.list3
	for lib in `cat dylib.list3`; do \
		cp $$lib $(MACOSX_PACKAGE_DIR)/Contents/Frameworks; \
	done
	rm -f dylib.list*

# Copy needed Qt resources
	mkdir -p $(MACOSX_PACKAGE_DIR)/Contents/Resources
	cp -r "`find /opt/local -name qt_menu.nib`" $(MACOSX_PACKAGE_DIR)/Contents/Resources

# Change the dependencies, so that we point to the libraries inside the bundle
	cd $(MACOSX_PACKAGE_DIR)/Contents/Frameworks; \
	for lib in `ls`; do \
		for dep in `otool -L $$lib | grep /opt/local/ | sed 's/ (.*//'`; do \
			install_name_tool -change $$dep @executable_path/../Frameworks/`basename $$dep` $$lib; \
			install_name_tool -change $$dep @executable_path/../Frameworks/`basename $$dep` ../MacOS/Eliot; \
		done; \
	done


# Build the .dmg file
package-macosx-dmg: package-macosx
	rm -f $(DMG_FILE)
	hdiutil create "$(DMG_FILE)" -verbose -scrub -srcfolder "$(top_builddir)/eliot-$(VERSION)"

