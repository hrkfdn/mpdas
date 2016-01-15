AC_INIT([mpdas], [0.4.1], [henrik@affekt.org],
             [mpdas], [http://www.50hz.ws])
AC_PREREQ([2.53])
AC_CONFIG_SRCDIR([mpdas.h])

AM_INIT_AUTOMAKE([1.10 no-define subdir-objects foreign])

AC_PROG_CXX

# ! Not sure that this is needed for upmpdcli and not only libupnpp
# libupnp is configured with large file support, and we need to do the same,
# else a difference in off_t size impacts struct File_Info and prevents the
# vdir to work. This does make a difference, for exemple, for Raspbian
# on the Raspberry PI. Use the same directives as libupnp's configure.ac
AC_TYPE_SIZE_T
AC_TYPE_OFF_T
AC_DEFINE([_LARGE_FILE_SOURCE], [], [Large files support])
AC_DEFINE([_FILE_OFFSET_BITS], [64], [File Offset size])

#### Libraries

# Support monitoring an UPnP renderer
AC_ARG_ENABLE(upnp,
    AC_HELP_STRING([--enable-upnp],
   [Enable upnp monitoring (needs libupnpp).]),
        upnpEnabled=$enableval, upnpEnabled=no)
AM_CONDITIONAL(DOUPNP, [test X$upnpEnabled = Xyes])
if test X$upnpEnabled = Xyes ; then
   AC_CHECK_LIB([upnpp], [getsyshwaddr], [], [AC_MSG_ERROR([libupnpp])])
   AC_DEFINE(WITH_UPNP, 1, [Support UPnP monitoring])
fi

AC_CHECK_LIB([curl], [curl_easy_init], [],AC_MSG_ERROR([libcurl not found]))
AC_CHECK_LIB([mpdclient], [mpd_connection_new], [],
                          AC_MSG_ERROR([libmpdclient not found]))


MPDAS_LIBS="$LIBS"
echo "MPDAS_LIBS=$LIBS"

LIBS=""

AC_SUBST(MPDAS_LIBS)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
