INCLUDES = -I$(top_srcdir)/lib -I$(top_srcdir)/xpm -I$(top_srcdir)/src

bin_PROGRAMS=fcitx
fcitx_SOURCES= IC.c IC.h\
			   KeyList.c \
			   MyErrorsHandlers.c MyErrorHandlers.h\
			   ime.c ime.h \
			   internalVersion.c \
			   xim.c xim.h \
			   main.c main.h \
			   fcitx.h

fcitx_LDADD=../../lib/libXimd.a \
			../im/pinyin/libimpinyin.a \
			../im/table/libimtable.a \
			../im/qw/libimqw.a \
			../im/extra/libimextra.a \
			../im/special/libimspecial.a \
			../tools/libtools.a \
			../ui/libui.a \
			../interface/libinterface.a

fcitx_LDFLAGS = -lXpm -lXtst -lpthread
