#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_INIT(lcsmgrservice, 1.0)
AM_INIT_AUTOMAKE
AC_CONFIG_HEADERS(config.h)

# Checks for programs.
AC_PROG_CC

# Checks for libraries.
PKG_CHECK_MODULES(GLIB, [glib-2.0 >= 2.31.13])
AC_SUBST(GLIB_CFLAGS)
AC_SUBST(GLIB_LIBS)

PKG_CHECK_MODULES(GIO, [gio-unix-2.0 >= 2.31.13])
AC_SUBST(GIO_CFLAGS)
AC_SUBST(GIO_LIBS)

PKG_CHECK_MODULES(UDISKS2, udisks2, have_udisks2=yes, have_udisks2=no)
if test x$have_udisks2 = xno ; then
    AC_MSG_ERROR([have_udisks2 development libraries not found])
fi

AM_CONDITIONAL(HAVE_UDISKS2, test x$have_udisks2 = xyes)
AC_SUBST(UDISKS2_CFLAGS)
AC_SUBST(UDISKS2_LIBS)

PKG_CHECK_MODULES(CRYPTO, libcrypto, have_crypto=yes, have_crypto=no)
if test x$have_crypto = xno ; then
    AC_MSG_ERROR([have_crypto development libraries not found])
fi

AM_CONDITIONAL(HAVE_CRYPTO, test x$have_crypto = xyes)
AC_SUBST(CRYPTO_CFLAGS)
AC_SUBST(CRYPTO_LIBS)

# systemd
AC_ARG_WITH([systemdsystemunitdir],
            AS_HELP_STRING([--with-systemdsystemunitdir=DIR], [Directory for systemd service files]),
            [],
            [with_systemdsystemunitdir=$($PKG_CONFIG --variable=systemdsystemunitdir systemd)])
if test "x$with_systemdsystemunitdir" != "xno"; then
  AC_SUBST([systemdsystemunitdir], [$with_systemdsystemunitdir])
fi
AM_CONDITIONAL(HAVE_SYSTEMD, [test -n "$systemdsystemunitdir"])

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_OUTPUT([Makefile
src/Makefile
data/Makefile
tools/Makefile])

echo "
               lcsmgrservice $VERSION
             ================

        prefix:                     ${prefix}
        libdir:                     ${libdir}
        libexecdir:                 ${libexecdir}
        bindir:                     ${bindir}
        sbindir:                    ${sbindir}
        datadir:                    ${datadir}
        sysconfdir:                 ${sysconfdir}
        localstatedir:              ${localstatedir}
        docdir:                     ${docdir}
        introspection:              ${found_introspection}

        udevdir:                    ${udevdir}
        systemdsystemunitdir:       ${systemdsystemunitdir}
        using libsystemd-login:     ${have_libsystemd_login}
        using libelogind:           ${have_libelogind}
        use /media for mounting:    ${fhs_media}
        acl support:                ${have_acl}
        libblockdev_part support:   ${have_libblockdev_part}

        compiler:                   ${CC}
        cflags:                     ${CFLAGS}
        cppflags:                   ${CPPFLAGS}

        Maintainer mode:            ${USE_MAINTAINER_MODE}
        Building api docs:          ${enable_gtk_doc}
        Building man pages:         ${enable_man}

        Enable modules:             ${enable_modules_info}

        BTRFS module:               ${have_btrfs}
        iSCSI module:               ${have_iscsi}${have_libiscsi_session_info_msg}
        LVM2  module:               ${have_lvm2}
        LVMCache:                   ${have_lvmcache}
        Zram module:                ${have_zram}
        LibStorageMgmt module:      ${have_lsm}
        Bcache module:              ${have_bcache}
"