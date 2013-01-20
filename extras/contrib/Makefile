ICONV_VERSION = 1.14
LIBCONFIG_VERSION = 1.4.8
BOOST_VERSION = 1_49_0
QT_VERSION = 4.8.2
EXPAT_VERSION = 2.0.1
ARABICA_VERSION = 2010-November
LIBXML2_VERSION = 2.7.8
XERCES_VERSION = 3.1.1
LOG4CXX_VERSION = 0.10.0


PREFIX = $(shell pwd)/inst
PREFIX_FLAG = $(PREFIX)/.flag
WGET = wget -c
SVN = svn

# XXX: Hardcoded for mingw on linux, at the moment
CC = i586-mingw32msvc-gcc
CXX = i586-mingw32msvc-g++
CPPFLAGS += -I$(PREFIX)/include
CONFIGURE = CC="$(CC)" CXX="$(CXX)" CPPFLAGS="$(CPPFLAGS)" ./configure --host=i586-mingw32msvc --build=i386-linux --prefix=$(PREFIX)

.PHONY: help all

all: .iconv .boost .expat .arabica .libconfig .apr2-svn .log4cxx-svn .qt

help:
	@echo "Usage: make all"

$(PREFIX_FLAG):
	mkdir -p $(PREFIX)
	touch $@


### iconv ###

ICONV_DIR = libiconv-$(ICONV_VERSION)
ICONV_DIR_FLAG = $(ICONV_DIR)/.flag
ICONV_ARCHIVE = libiconv-$(ICONV_VERSION).tar.gz

$(ICONV_ARCHIVE): $(PREFIX_FLAG)
	$(WGET) http://ftp.gnu.org/pub/gnu/libiconv/$@

$(ICONV_DIR_FLAG): $(ICONV_ARCHIVE)
	tar xzf $<
	touch $@

.iconv: $(ICONV_DIR_FLAG)
	(cd $(ICONV_DIR) && $(CONFIGURE) --enable-static --disable-shared && make && make install)
	touch $@


### Expat ###

EXPAT_DIR = expat-$(EXPAT_VERSION)
EXPAT_DIR_FLAG = $(EXPAT_DIR)/.flag
EXPAT_ARCHIVE = expat-$(EXPAT_VERSION).tar.gz

$(EXPAT_ARCHIVE): $(PREFIX_FLAG)
	$(WGET) http://kent.dl.sf.net/project/expat/expat/$(EXPAT_VERSION)/$@

$(EXPAT_DIR_FLAG): $(EXPAT_ARCHIVE)
	tar xzf $<
	touch $@

.expat: $(EXPAT_DIR_FLAG)
	(cd $(EXPAT_DIR) && $(CONFIGURE) --enable-static --disable-shared && make install)
	touch $@


### Arabica ###

ARABICA_DIR = arabica-$(ARABICA_VERSION)
ARABICA_DIR_FLAG = $(ARABICA_DIR)/.flag
ARABICA_ARCHIVE = arabica-$(ARABICA_VERSION).tar.bz2

$(ARABICA_ARCHIVE): $(PREFIX_FLAG)
	$(WGET) http://downloads.sf.net/project/arabica/arabica/November-10/$@

$(ARABICA_DIR_FLAG): $(ARABICA_ARCHIVE)
	tar xjf $<
	touch $@

.arabica: $(ARABICA_DIR_FLAG)
	(cd $(ARABICA_DIR) && patch -p0 < ../arabica-no-example.patch && $(CONFIGURE) --enable-static --disable-shared --with-parser=expat --with-expat=$(PREFIX) --with-boost=$(PREFIX) --with-tests=no --with-dom=no && make install)
	touch $@


### LibXML 2 ###

LIBXML2_DIR = libxml2-$(LIBXML2_VERSION)
LIBXML2_DIR_FLAG = $(LIBXML2_DIR)/.flag
LIBXML2_ARCHIVE = libxml2-$(LIBXML2_VERSION).tar.gz

$(LIBXML2_ARCHIVE): $(PREFIX_FLAG)
	$(WGET) ftp://xmlsoft.org/libxml2/$@

$(LIBXML2_DIR_FLAG): $(LIBXML2_ARCHIVE)
	tar xzf $<
	touch $@

# TODO: add more --without-xxx flags (apparently needed for build: pattern, debug)
# --with-legacy and --with-html are needed for arabica
.libxml2: $(LIBXML2_DIR_FLAG)
	(cd $(LIBXML2_DIR) && $(CONFIGURE) --enable-static --disable-shared --with-legacy --without-schematron --without-tree --without-zlib --without-http --without-ftp --without-html --without-python --without-docbook --without-regexps && make install)
	touch $@


### Xerces ###

XERCES_DIR = xerces-c-$(XERCES_VERSION)
XERCES_DIR_FLAG = $(XERCES_DIR)/.flag
XERCES_ARCHIVE = xerces-c-$(XERCES_VERSION).tar.gz

$(XERCES_ARCHIVE): $(PREFIX_FLAG)
	$(WGET) http://mirrors.24-7-solutions.net/pub/apache/xerces/c/3/sources/$@

$(XERCES_DIR_FLAG): $(XERCES_ARCHIVE)
	tar xzf $<
	touch $@

.xerces: $(XERCES_DIR_FLAG)
	(cd $(XERCES_DIR) && $(CONFIGURE) --enable-static --disable-shared --disable-network && make install)
	touch $@


### APR 2 SVN (needed by log4cxx) ###
# APR 2 is not yet released, so we have to use the SVN version (last known to work: 28/04/2012)

APR_DIR2 = apr
APR_DIR_FLAG2 = $(APR_DIR2)/.flag

$(APR_DIR_FLAG2):
	$(SVN) co http://svn.apache.org/repos/asf/apr/apr/trunk/ apr
	touch $@

.apr2-svn: $(APR_DIR_FLAG2)
	(cd $(APR_DIR2) && patch -p0 < ../apr-force-large-files-support.patch && ./buildconf && $(CONFIGURE) --enable-static --disable-shared --with-expat=$(PREFIX) && make && make install)
	touch $@


### log4cxx SVN ###

LOG4CXX_DIR2 = apache-log4cxx
LOG4CXX_DIR_FLAG2 = $(LOG4CXX_DIR2)/.flag

$(LOG4CXX_DIR_FLAG2):
	$(SVN) checkout http://svn.apache.org/repos/asf/logging/log4cxx/trunk apache-log4cxx
	touch $@

.log4cxx-svn: $(LOG4CXX_DIR_FLAG2)
	(cd $(LOG4CXX_DIR2) && ./autogen.sh && patch -p0 < ../log4cxx-no-example.patch && patch -p0 < ../log4cxx-build-fixes.patch && patch -p0 < ../log4cxx-pkg-config.patch && $(CONFIGURE) --with-apr=$(PREFIX)/bin/apr-2-config --disable-doxygen --disable-dot --enable-static --disable-shared && make && make -i install)
	touch $@


### log4cxx ###

LOG4CXX_DIR = apache-log4cxx-$(LOG4CXX_VERSION)
LOG4CXX_DIR_FLAG = $(LOG4CXX_DIR)/.flag
LOG4CXX_ARCHIVE = apache-log4cxx-$(LOG4CXX_VERSION).tar.gz

$(LOG4CXX_ARCHIVE): $(PREFIX_FLAG)
	$(WGET) http://apache.cyberuse.com/logging/log4cxx/$(LOG4CXX_VERSION)/$@

$(LOG4CXX_DIR_FLAG): $(LOG4CXX_ARCHIVE)
	tar xzf $<
	touch $@

.log4cxx: $(LOG4CXX_DIR_FLAG)
	(cd $(LOG4CXX_DIR) && patch -p0 < ../log4cxx-no-example.patch && patch -p0 < ../log4cxx-build-fixes.patch && patch -p0 < ../log4cxx-pkg-config.patch && $(CONFIGURE) --with-apr=$(PREFIX)/bin/apr-2-config --disable-doxygen --disable-dot --enable-static --disable-shared && make && make install)
	touch $@


### libconfig ###

LIBCONFIG_DIR = libconfig-$(LIBCONFIG_VERSION)
LIBCONFIG_DIR_FLAG = $(LIBCONFIG_DIR)/.flag
LIBCONFIG_ARCHIVE = libconfig-$(LIBCONFIG_VERSION).tar.gz

$(LIBCONFIG_ARCHIVE): $(PREFIX_FLAG)
	$(WGET) http://www.hyperrealm.com/libconfig/$@

$(LIBCONFIG_DIR_FLAG): $(LIBCONFIG_ARCHIVE)
	tar xvf $<
	touch $@

.libconfig: $(LIBCONFIG_DIR_FLAG)
	(cd $(LIBCONFIG_DIR) && patch -p0 < ../libconfig++-static-build.patch && CFLAGS=-DLIBCONFIG_STATIC CXXFLAGS=-DLIBCONFIGXX_STATIC $(CONFIGURE) --disable-examples --enable-static --disable-shared && make && make install)
	touch $@


### Boost ###

BOOST_DIR = boost_$(BOOST_VERSION)
BOOST_DIR_FLAG = $(BOOST_DIR)/.flag
BOOST_ARCHIVE = boost_$(BOOST_VERSION).tar.bz2

$(BOOST_ARCHIVE): $(PREFIX_FLAG)
	$(WGET) http://garr.dl.sourceforge.net/sourceforge/boost/$@

$(BOOST_DIR_FLAG): $(BOOST_ARCHIVE)
	tar xjf $<
	touch $@

# We don't build any library, because we don't need them (and it is really
# hard to cross-compile with their crappy build system)
.boost: $(BOOST_DIR_FLAG)
	#(cd $(BOOST_DIR) && ./configure --prefix=$(PREFIX) && ./tools/jam/src/bin.linuxx86/bjam --toolset=gcc --prefix=$(PREFIX) --without-date_time --without-filesystem --without-graph --without-iostreams --without-program_options --without-python --without-regex --without-serialization --without-signals --without-test --without-thread --without-wave install)
	cp -r $(BOOST_DIR)/boost $(PREFIX)/include
	touch $@


### Qt ###

# FIXME: No automated way to build it at the moment :-(
QT_DIR = qt4-$(QT_VERSION)-win32-bin
QT_DIR_FLAG = $(QT_DIR)/.flag
QT_ARCHIVE = qt4-$(QT_VERSION)-win32-bin.tar.bz2

$(QT_ARCHIVE): $(PREFIX_FLAG)
	$(WGET) http://dl.sv.nongnu.org/releases-noredirect/eliot/other/$@

$(QT_DIR_FLAG): $(QT_ARCHIVE)
	tar xjf $<
	touch $@

.qt: $(QT_DIR_FLAG)
	(cd $(QT_DIR); \
		cp -r bin include lib share $(PREFIX); \
		for i in QtCore QtGui QtNetwork; do \
			sed 's,@@PREFIX@@,$(PREFIX),' $(PREFIX)/lib/pkgconfig/$$i.pc.in > $(PREFIX)/lib/pkgconfig/$$i.pc; \
		done; \
		rm $(PREFIX)/lib/pkgconfig/*.in)
	touch $@

