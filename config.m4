# config.m4

PHP_ARG_ENABLE(oqs, whether to enable oqs support, [ --enable-oqs Enable oqs], no)

if test "$PHP_OQS" != "no"; then
  AC_CHECK_HEADERS([oqs/oqs.h], [], [AC_MSG_ERROR([oqs/oqs.h not found; install liboqs headers])])

  AC_PATH_PROG(PKG_CONFIG, pkg-config, no)
  if test "$PKG_CONFIG" != "no" && $PKG_CONFIG --exists liboqs; then
    OQS_CFLAGS=`$PKG_CONFIG --cflags liboqs`
    OQS_LIBS=`$PKG_CONFIG --libs liboqs`
    PHP_EVAL_INCLINE($OQS_CFLAGS)
    PHP_EVAL_LIBLINE($OQS_LIBS, OQS_SHARED_LIBADD)
  else
    OQS_PREFIX="/opt/homebrew"        dnl adjust if your liboqs is elsewhere
    OQS_LIBDIR="$OQS_PREFIX/lib"
    PHP_ADD_INCLUDE([$OQS_PREFIX/include])
    PHP_ADD_LIBRARY_WITH_PATH([oqs], [$OQS_LIBDIR], [OQS_SHARED_LIBADD])
    LDFLAGS="$LDFLAGS -Wl,-rpath,$OQS_LIBDIR"
  fi

  PHP_SUBST(OQS_SHARED_LIBADD)
  PHP_NEW_EXTENSION(oqs, oqs.c kem.c sig.c, $ext_shared)
fi
