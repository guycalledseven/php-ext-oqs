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
  dnl An explicit --with-oqs=DIR always wins; pkg-config is only consulted
  dnl when no prefix was given.
  OQS_USE_PKG_CONFIG=no
  if test "x$PHP_OQS" = "x" -o "x$PHP_OQS" = "xyes"; then
    AC_PATH_PROG([PKG_CONFIG], [pkg-config], [no])
    if test "$PKG_CONFIG" != "no" && $PKG_CONFIG --exists liboqs; then
      OQS_USE_PKG_CONFIG=yes
    fi
    OQS_PREFIX=/usr/local
  else
    OQS_PREFIX=$PHP_OQS
  fi

  if test "$OQS_USE_PKG_CONFIG" = "yes"; then
    dnl If you want static, ask for --static (requires .pc with Libs.private)
    OQS_CFLAGS=`$PKG_CONFIG --cflags liboqs`
    OQS_LIBS=`$PKG_CONFIG --libs liboqs`
    PHP_EVAL_INCLINE([$OQS_CFLAGS])
    PHP_EVAL_LIBLINE([$OQS_LIBS], [OQS_SHARED_LIBADD])
  else
    dnl Manual include/lib paths
    OQS_INCDIR="$OQS_PREFIX/include"
    OQS_LIBDIR="$OQS_PREFIX/lib"
    AC_MSG_CHECKING([for oqs/oqs.h in $OQS_INCDIR])
    if test -f "$OQS_INCDIR/oqs/oqs.h"; then
      AC_MSG_RESULT([yes])
    else
      AC_MSG_RESULT([no])
      AC_MSG_ERROR([oqs/oqs.h not found under $OQS_INCDIR])
    fi
    PHP_ADD_INCLUDE([$OQS_INCDIR])

    dnl Prefer static lib if present
    OQS_STATIC="$OQS_LIBDIR/liboqs.a"
    AC_MSG_CHECKING([for liboqs in $OQS_LIBDIR])
    if test -f "$OQS_STATIC"; then
      AC_MSG_RESULT([static (liboqs.a)])
    elif ls "$OQS_LIBDIR"/liboqs.so* "$OQS_LIBDIR"/liboqs*.dylib >/dev/null 2>&1; then
      AC_MSG_RESULT([shared])
    else
      AC_MSG_RESULT([no])
      dnl Without this the linker would silently pick up another liboqs from
      dnl its default search path. Note: "make clean" deletes every *.a below
      dnl the extension directory, so keep the liboqs prefix outside of it.
      AC_MSG_ERROR([no liboqs library found under $OQS_LIBDIR])
    fi
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

  dnl EXTRA_LDFLAGS is already part of the extension link rule
  if test "x$EXTRA_LDFLAGS" != "x"; then
    PHP_ADD_LIBRARY([m], 1, [OQS_SHARED_LIBADD]) dnl harmless; some liboqs builds need it
  fi

  PHP_SUBST([OQS_SHARED_LIBADD])
  PHP_NEW_EXTENSION([oqs], [oqs.c kem.c sig.c], [$ext_shared])
fi
