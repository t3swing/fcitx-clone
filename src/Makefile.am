bin_PROGRAMS = fcitx fcitx.xft
AM_CFLAGS = -O2 -fno-strength-reduce
INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)/xpm
fcitx_SOURCES = \
  IC.c IC.h ime.c ime.h InputWindow.c InputWindow.h KeyList.c \
  KeyList.h main.c main.h MainWindow.c MainWindow.h \
  MyErrorsHandlers.c MyErrorsHandlers.h punc.c punc.h py.c PYFA.c \
  PYFA.h py.h pyMapTable.c pyMapTable.h pyParser.c pyParser.h \
  SetLocale.c SetLocale.h sp.c sp.h tools.c tools.h ui.c ui.h \
  version.h wbx.c wbx.h xim.c xim.h
fcitx_LDADD = ../lib/libXimd.a

fcitx_xft_SOURCES = ${fcitx_SOURCES}
fcitx_xft_OBJECTS = \
  IC.o ime.o KeyList.o main.o MainWindow.o MyErrorsHandlers.o \
  punc.o PYFA.o pyMapTable.o pyParser.o SetLocale.o sp.o xim.o \
  InputWindow.xft.o py.xft.o tools.xft.o ui.xft.o wbx.xft.o
fcitx_xft_LDADD = ../lib/libXimd.a
fcitx_xft_AM_CFLAGS = ${CFLAGS} ${XFT_CFLAGS}
fcitx_xft_LDFLAGS = ${LIBS} ${XFT_LIBS}

%.xft.o: %.c
	$(COMPILE) ${fcitx_xft_AM_CFLAGS} -c `test -f '$<' || echo '$(srcdir)/'`$< -o $@
