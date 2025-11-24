# config.m4 - static linking

dnl config.m4 for oqs

PHP_ARG_WITH([oqs],
  [for liboqs install prefix],
  [AS_HELP_STRING([--with-oqs[=DIR]], [liboqs install prefix (for headers/libs)])],
  [no])

dnl Optional: pass OQS_COMMIT=abcd during configure
AC_ARG_VAR([OQS_COMMIT], [liboqs short commit for phpinfo])
if test "x$OQS_COMMIT" = "x"; then
  OQS_COMMIT=unknown
fi
AC_DEFINE_UNQUOTED([PHP_OQS_LIB_COMMIT], ["$OQS_COMMIT"], [liboqs git commit])

if test "$PHP_OQS" != "no"; then
  dnl Prefer pkg-config if available
  AC_PATH_PROG([PKG_CONFIG], [pkg-config], [no])

  if test "$PKG_CONFIG" != "no" && $PKG_CONFIG --exists liboqs; then
    dnl If you want static, ask for --static (requires .pc with Libs.private)
    OQS_CFLAGS=`$PKG_CONFIG --cflags liboqs`
    OQS_LIBS=`$PKG_CONFIG --libs liboqs`
    PHP_EVAL_INCLINE([$OQS_CFLAGS])
    PHP_EVAL_LIBLINE([$OQS_LIBS], [OQS_SHARED_LIBADD])
  else
    dnl Manual include/lib paths
    if test "x$PHP_OQS" = "x" -o "x$PHP_OQS" = "xyes"; then
      OQS_PREFIX=/usr/local
    else
      OQS_PREFIX=$PHP_OQS
    fi
    OQS_INCDIR="$OQS_PREFIX/include"
    OQS_LIBDIR="$OQS_PREFIX/lib"
    AC_CHECK_HEADERS([oqs/oqs.h], [], [AC_MSG_ERROR([oqs/oqs.h not found under $OQS_INCDIR])])
    PHP_ADD_INCLUDE([$OQS_INCDIR])

    dnl Prefer static lib if present
    OQS_STATIC="$OQS_LIBDIR/liboqs.a"
    case "$host_os" in
      darwin*)
        if test -f "$OQS_STATIC"; then
          dnl macOS: use -force_load to pull all objects from static archive
          EXTRA_LDFLAGS="$EXTRA_LDFLAGS -Wl,-force_load,$OQS_STATIC -Wl,-dead_strip -fvisibility=hidden"
        else
          PHP_ADD_LIBRARY_WITH_PATH([oqs], [$OQS_LIBDIR], [OQS_SHARED_LIBADD])
        fi
        ;;
      *)
        if test -f "$OQS_STATIC"; then
          dnl Linux/BSD: whole-archive for static pull-in and hide symbols
          EXTRA_LDFLAGS="$EXTRA_LDFLAGS -Wl,--whole-archive,$OQS_STATIC,--no-whole-archive -Wl,--exclude-libs,ALL -fvisibility=hidden"
        else
          PHP_ADD_LIBRARY_WITH_PATH([oqs], [$OQS_LIBDIR], [OQS_SHARED_LIBADD])
        fi
        ;;
    esac
  fi

  dnl Wire LDFLAGS if we built a static choice above
  if test "x$EXTRA_LDFLAGS" != "x"; then
    PHP_ADD_LIBRARY([m], 1, [OQS_SHARED_LIBADD]) dnl harmless; some liboqs builds need it
    LDFLAGS="$LDFLAGS $EXTRA_LDFLAGS"
  fi

  PHP_SUBST([OQS_SHARED_LIBADD])
  PHP_NEW_EXTENSION([oqs], [oqs.c kem.c sig.c], [$ext_shared])
fi
