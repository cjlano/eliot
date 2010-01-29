SUBDIRS = intl dic game utils qt po extras

ACLOCAL_AMFLAGS = -I m4

WIN32_PACKAGE_DIR = $(top_builddir)/eliot-$(VERSION)
MACOSX_PACKAGE_DIR = $(top_builddir)/Eliot.app

.PHONY: package-win32-zip package-macosx

EXTRA_DIST = COPYING.arabica

# Prepare the win32 package
package-win32-zip:
# Check that the working directory is not already there
	@if test -e $(WIN32_PACKAGE_DIR); then \
		echo '$(WIN32_PACKAGE_DIR) is in the way'; \
		false; \
	fi

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
		unix2dos $(WIN32_PACKAGE_DIR)/$$i.txt; \
	done

# Copy the mingw DLL
	gunzip -c /usr/share/doc/mingw32-runtime/mingwm10.dll.gz > $(WIN32_PACKAGE_DIR)/mingwm10.dll

# Zip the temporary dir and remove it
	zip -r -9 eliot-$(VERSION)-win32.zip $(WIN32_PACKAGE_DIR)
	rm -rf $(WIN32_PACKAGE_DIR)

# Prepare the macosx package
package-macosx:
# Check that the working directory is not already there
	@if test -e $(MACOSX_PACKAGE_DIR); then \
		echo '$(MACOSX_PACKAGE_DIR) is in the way'; \
		false; \
	fi

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
	cp $(QT4LOCALEDIR)/*.qm $(MACOSX_PACKAGE_DIR)/locale/qt4/ || true
