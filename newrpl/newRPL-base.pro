TEMPLATE = app
CONFIG += console
CONFIG -= qt

DEFINES += "TARGET_PC"

SOURCES += main.cpp \
    runstream.c \
    hal.c \
    romlibs.c \
    lib-common.c \
    lib-4090-overloaded.c \
    sysvars.c \
    compiler.c \
    datastack.c \
    returnstack.c \
    tempob.c \
    lam.c \
    lib-eight-docol.c \
    lib-twelve-bint.c \
    lib-4080-localenv.c \
    lib-two-ident.c \
    lib-twenty-lam.c \
    directory.c \
    gc.c \
    contrib/mpdecimal-2.4.0/libmpdec/basearith.c \
    contrib/mpdecimal-2.4.0/libmpdec/constants.c \
    contrib/mpdecimal-2.4.0/libmpdec/context.c \
    contrib/mpdecimal-2.4.0/libmpdec/convolute.c \
    contrib/mpdecimal-2.4.0/libmpdec/crt.c \
    contrib/mpdecimal-2.4.0/libmpdec/difradix2.c \
    contrib/mpdecimal-2.4.0/libmpdec/fnt.c \
    contrib/mpdecimal-2.4.0/libmpdec/fourstep.c \
    contrib/mpdecimal-2.4.0/libmpdec/io.c \
    contrib/mpdecimal-2.4.0/libmpdec/memory.c \
    contrib/mpdecimal-2.4.0/libmpdec/mpdecimal.c \
    contrib/mpdecimal-2.4.0/libmpdec/mpsignal.c \
    contrib/mpdecimal-2.4.0/libmpdec/numbertheory.c \
    contrib/mpdecimal-2.4.0/libmpdec/sixstep.c \
    contrib/mpdecimal-2.4.0/libmpdec/transpose.c \
    io_substring.c \
    errors.c \
    lib-64-precision.c \
    lib-65-development.c \
    atan_1_comp.c \
    atan_2_comp.c \
    atan_5_comp.c \
    transcendentals.c \
    lib-66-transcendentals.c \
    atanh_1_comp.c \
    atanh_2_comp.c \
    atanh_5_comp.c \
    cordic_K_comp.c \
    cordic_Kh_comp.c \
    lists.c \
    lib-50-lists.c \
    symbolic.c \
    lib-68-flags.c \
    lib-70-binary.c \
    lib-32-symbolic.c \
    lib-nine-docol2.c \
    lib-ten-reals.c \
    lib-24-string.c \
    lib-72-stack.c \
    lib-28-dirs.c \
    lib-30-complex.c \
    lib-48-matrix.c \
    matrix.c \
    utf8lib.c \
    utf8data.c

HEADERS += \
    libraries.h \
    newrpl.h \
    hal.h \
    sysvars.h \
    contrib/mpdecimal-2.4.0/libmpdec/basearith.h \
    contrib/mpdecimal-2.4.0/libmpdec/bits.h \
    contrib/mpdecimal-2.4.0/libmpdec/constants.h \
    contrib/mpdecimal-2.4.0/libmpdec/convolute.h \
    contrib/mpdecimal-2.4.0/libmpdec/crt.h \
    contrib/mpdecimal-2.4.0/libmpdec/difradix2.h \
    contrib/mpdecimal-2.4.0/libmpdec/fnt.h \
    contrib/mpdecimal-2.4.0/libmpdec/fourstep.h \
    contrib/mpdecimal-2.4.0/libmpdec/io.h \
    contrib/mpdecimal-2.4.0/libmpdec/memory.h \
    contrib/mpdecimal-2.4.0/libmpdec/numbertheory.h \
    contrib/mpdecimal-2.4.0/libmpdec/sixstep.h \
    contrib/mpdecimal-2.4.0/libmpdec/transpose.h \
    contrib/mpdecimal-2.4.0/libmpdec/typearith.h \
    contrib/mpdecimal-2.4.0/libmpdec/umodarith.h \
    contrib/mpdecimal-2.4.0/libmpdec/mpdecimal.h \
    utf8lib.h

INCLUDEPATH += contrib/mpdecimal-2.4.0/libmpdec

OTHER_FILES += \
    LICENSE.txt \
    README.txt
