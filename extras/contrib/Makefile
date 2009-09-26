ICONV_VERSION = 1.12
LIBCONFIG_VERSION = 1.3.2
BOOST_VERSION = 1_39_0
QT_VERSION = 4.5.3

PREFIX = $(shell pwd)/inst
WGET = wget -c

# XXX: Hardcoded for mingw on linux, at the moment
CC = i586-mingw32msvc-gcc
CXX = i586-mingw32msvc-g++
CPPFLAGS += -I$(PREFIX)/include
CONFIGURE = CC="$(CC)" CXX="$(CXX)" CPPFLAGS="$(CPPFLAGS)" ./configure --host=i586-mingw32msvc --build=i386-linux --prefix=$(PREFIX)

.PHONY: help all

all: .iconv .boost .qt .libconfig

help:
	@echo "Usage: make all"


$(PREFIX):
	mkdir -p $@

### iconv ###

ICONV_DIR = libiconv-$(ICONV_VERSION)
ICONV_ARCHIVE = libiconv-$(ICONV_VERSION).tar.gz

$(ICONV_ARCHIVE): $(PREFIX)
	$(WGET) http://ftp.gnu.org/pub/gnu/libiconv/$(ICONV_ARCHIVE)

$(ICONV_DIR): $(ICONV_ARCHIVE)
	tar xzf $<

.iconv: $(ICONV_DIR)
	(cd $< && $(CONFIGURE) --enable-static --disable-shared && make && make install)
	touch $@


### libconfig ###

LIBCONFIG_DIR = libconfig-$(LIBCONFIG_VERSION)
LIBCONFIG_ARCHIVE = libconfig-$(LIBCONFIG_VERSION).tar.gz

$(LIBCONFIG_ARCHIVE): $(PREFIX)
	$(WGET) http://www.hyperrealm.com/libconfig/$(LIBCONFIG_ARCHIVE)

$(LIBCONFIG_DIR): $(LIBCONFIG_ARCHIVE)
	tar xvf $<

.libconfig: $(LIBCONFIG_DIR)
	(cd $< && CFLAGS=-DLIBCONFIG_STATIC CXXFLAGS=-DLIBCONFIG_STATIC $(CONFIGURE) --enable-static --disable-shared && make && make install)
	touch $@


### Boost ###

BOOST_DIR = boost_$(BOOST_VERSION)
BOOST_ARCHIVE = boost_$(BOOST_VERSION).tar.bz2

$(BOOST_ARCHIVE): $(PREFIX)
	$(WGET) http://garr.dl.sourceforge.net/sourceforge/boost/$(BOOST_ARCHIVE)

$(BOOST_DIR): $(BOOST_ARCHIVE)
	tar xjf $<

# We don't build any library, because we don't need them (and it is really
# hard to cross-compile with their crappy build system)
.boost: $(BOOST_DIR)
	#(cd $< && ./configure --prefix=$(PREFIX) && ./tools/jam/src/bin.linuxx86/bjam --toolset=gcc --prefix=$(PREFIX) --without-date_time --without-filesystem --without-graph --without-iostreams --without-program_options --without-python --without-regex --without-serialization --without-signals --without-test --without-thread --without-wave install)
	cp -r $</boost $(PREFIX)/include
	touch $@


### Qt ###

# FIXME: No automated way to build it at the moment :-(
QT_ARCHIVE = qt4-$(QT_VERSION)-win32-bin.tar.bz2
QT_DIR = qt4-$(QT_VERSION)-win32-bin

$(QT_ARCHIVE): $(PREFIX)
	$(WGET) http://dl.sv.nongnu.org/releases-noredirect/eliot/other/$(QT_ARCHIVE)

$(QT_DIR): $(QT_ARCHIVE)
	tar xjf $<

.qt: $(QT_DIR)
	#(cd $<; mkdir -p $(PREFIX)/bin; mkdir -p $(PREFIX)/include; mkdir -p $(PREFIX)/lib/pkgconfig; rm -f $(PREFIX)/lib/pkgconfig/Qt*; sed 's,@@PREFIX@@,$(PREFIX),' lib/pkgconfig/QtCore.pc.in > $(PREFIX)/lib/pkgconfig/QtCore.pc; sed 's,@@PREFIX@@,$(PREFIX),' lib/pkgconfig/QtGui.pc.in > $(PREFIX)/lib/pkgconfig/QtGui.pc; cp -r include/* $(PREFIX)/include; cp lib/*a $(PREFIX)/lib; mkdir -p $(PREFIX)/share/qt4/translations; cp -r share/translations/* $(PREFIX)/share/qt4/translations)
	(cd $<; cp -r bin include lib share $(PREFIX); sed 's,@@PREFIX@@,$(PREFIX),' $(PREFIX)/lib/pkgconfig/QtCore.pc.in > $(PREFIX)/lib/pkgconfig/QtCore.pc; sed 's,@@PREFIX@@,$(PREFIX),' lib/pkgconfig/QtGui.pc.in > $(PREFIX)/lib/pkgconfig/QtGui.pc; rm $(PREFIX)/lib/pkgconfig/*.in)
	touch $@

