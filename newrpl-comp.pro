#-------------------------------------------------
#
# Project created by QtCreator 2014-11-29T15:53:29
#
#-------------------------------------------------

TARGET = newrpl-comp
TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

DEFINES += TARGET_PC NO_RPL_OBJECTS

SOURCES += firmware/ggl/ggl/ggl_bitblt.c \
    firmware/ggl/ggl/ggl_bitbltoper.c \
    firmware/ggl/ggl/ggl_filter.c \
    firmware/ggl/ggl/ggl_fltdarken.c \
    firmware/ggl/ggl/ggl_fltlighten.c \
    firmware/ggl/ggl/ggl_getnib.c \
    firmware/ggl/ggl/ggl_hblt.c \
    firmware/ggl/ggl/ggl_hbltfilter.c \
    firmware/ggl/ggl/ggl_hbltoper.c \
    firmware/ggl/ggl/ggl_hline.c \
    firmware/ggl/ggl/ggl_initscr.c \
    firmware/ggl/ggl/ggl_mkcolor.c \
    firmware/ggl/ggl/ggl_mkcolor32.c \
    firmware/ggl/ggl/ggl_opmask.c \
    firmware/ggl/ggl/ggl_optransp.c \
    firmware/ggl/ggl/ggl_ovlblt.c \
    firmware/ggl/ggl/ggl_pltnib.c \
    firmware/ggl/ggl/ggl_rect.c \
    firmware/ggl/ggl/ggl_rectp.c \
    firmware/ggl/ggl/ggl_revblt.c \
    firmware/ggl/ggl/ggl_scrolldn.c \
    firmware/ggl/ggl/ggl_scrolllf.c \
    firmware/ggl/ggl/ggl_scrollrt.c \
    firmware/ggl/ggl/ggl_scrollup.c \
    firmware/ggl/ggl/ggl_vline.c \
    firmware/hal_battery.c \
    firmware/hal_keyboard.c \
    firmware/hal_screen.c \
    firmware/sys/graphics.c \
    firmware/sys/icons.c \
    firmware/sys/target_pc/non-gui-stubs.c \
    firmware/sys/target_pc/battery.c \
    firmware/sys/target_pc/cpu.c \
    firmware/sys/target_pc/exception.c \
    firmware/sys/target_pc/irq.c \
    firmware/sys/target_pc/keyboard.c \
    firmware/sys/target_pc/lcd.c \
    firmware/sys/target_pc/stdlib.c \
    firmware/sys/target_pc/timer.c \
    firmware/hal_globals.c \
    newrpl/compiler.c \
    newrpl/datastack.c \
    newrpl/directory.c \
    newrpl/errors.c \
    newrpl/gc.c \
    newrpl/lam.c \
    newrpl/lib-24-string.c \
    newrpl/lib-28-dirs.c \
    newrpl/lib-30-complex.c \
    newrpl/lib-64-arithmetic.c \
    newrpl/lib-66-transcendentals.c \
    newrpl/lib-68-flags.c \
    newrpl/lib-70-binary.c \
    newrpl/lib-72-stack.c \
    newrpl/lib-4080-localenv.c \
    newrpl/lib-4090-overloaded.c \
    newrpl/lib-common.c \
    newrpl/lib-eight-docol.c \
    newrpl/lib-nine-docol2.c \
    newrpl/lib-ten-reals.c \
    newrpl/lib-twelve-bint.c \
    newrpl/lib-two-ident.c \
    newrpl/lists.c \
    newrpl/returnstack.c \
    newrpl/romlibs.c \
    newrpl/runstream.c \
    newrpl/symbolic.c \
    newrpl/sysvars.c \
    newrpl/tempob.c \
    firmware/sys/target_pc/mem.c \
    firmware/sys/target_pc/boot.c \
    firmware/ui_cmdline.c \
    newrpl/utf8lib.c \
    newrpl/utf8data.c \
    firmware/sys/Font5C.c \
    firmware/sys/Font6A.c \
    firmware/sys/keybcommon.c \
    firmware/sys/Font7A.c \
    newrpl/matrix.c \
    firmware/sys/Font8C.c \
    firmware/sys/Font8D.c \
    newrpl/atan_1_8_comp.c \
    newrpl/atan_2_8_comp.c \
    newrpl/atan_5_8_comp.c \
    newrpl/atanh_1_8_comp.c \
    newrpl/atanh_2_8_comp.c \
    newrpl/atanh_5_8_comp.c \
    newrpl/cordic_K_8_comp.c \
    newrpl/cordic_Kh_8_comp.c \
    newrpl/decimal.c \
    newrpl/dectranscen.c \
    newrpl/backup.c \
    newrpl/sanity.c \
    newrpl/lib-32-lam.c \
    newrpl/lib-65-system.c \
    firmware/hal_msgenglish.c \
    newrpl/units.c \
    newrpl/lib-62-lists.c \
    newrpl/lib-56-symbolic.c \
    newrpl/lib-52-matrix.c \
    newrpl/lib-54-units.c \
    newrpl/autocomplete.c \
    newrpl/arithmetic.c \
    newrpl/lib-20-comments.c \
    firmware/sys/target_pc/flash.c \
    firmware/ui_softmenu.c \
    firmware/ggl/ggl/ggl_fltinvert.c \
    newrpl-comp.c \
    newrpl/lib-4079-rpl2c.c \
    newrpl/lib-48-angles.c \
    newrpl/lib-80-fonts.c \
    firmware/sys/Font8B.c \
    firmware/sys/Font8A.c \
    firmware/sys/Font6m.c \
    firmware/sys/Font5B.c \
    firmware/sys/Font5A.c \
    firmware/sys/target_pc/rtc.c


HEADERS  += firmware/include/ggl.h \
    target_pc.h \
    firmware/include/ui.h \
    firmware/include/hal_api.h \
    newrpl/hal.h \
    newrpl/libraries.h \
    newrpl/newrpl.h \
    newrpl/sysvars.h \
    newrpl/utf8lib.h \
    newrpl/decimal.h \
    newrpl/errorcodes.h \
    newrpl/arithmetic.h \
    newrpl/cmdcodes.h \
    newrpl/common-macros.h \
    newrpl/lib-header.h \
    newrpl/include-all.h \
    newrpl/romlibs.h \
    firmware/include/fsystem.h \
    firmware/sys/fsystem/fsyspriv.h

INCLUDEPATH += firmware/include newrpl /usr/local/include /usr/include

LIBS += -L/usr/local/lib

DISTFILES +=


QMAKE_CFLAGS += -Wno-duplicate-decl-specifier



install_bin.path = $$PWD/tools-bin
install_bin.files = $$OUT_PWD/newrpl-comp
INSTALLS += install_bin
