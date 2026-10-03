#!/bin/sh
# Build a self-contained oqs.so (liboqs linked statically), run the test suite
# against it and drop the result into dist/.
#
# Works the same on a developer machine, inside the official php Docker images
# (see ci/docker-build.sh) and on CI runners. Everything is built in a scratch
# directory, so the source tree and its own phpize build are left untouched.
#
# Environment:
#   LIBOQS_VERSION  liboqs release tag to build against   (default: 0.16.0)
#   OUT_DIR         where the finished binary is written  (default: <repo>/dist)
#   WORK_DIR        scratch directory                     (default: mktemp)
#   PHP_CONFIG      php-config of the target PHP          (default: php-config)
#   PHPIZE          phpize of the target PHP              (default: phpize)
#   PHP             php CLI of the target PHP             (default: php)
set -eu

LIBOQS_VERSION="${LIBOQS_VERSION:-0.16.0}"
SRC_DIR="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="${OUT_DIR:-$SRC_DIR/dist}"
PHP_CONFIG="${PHP_CONFIG:-php-config}"
PHPIZE="${PHPIZE:-phpize}"
PHP="${PHP:-php}"

if [ -z "${WORK_DIR:-}" ]; then
  WORK_DIR="$(mktemp -d)"
  trap 'rm -rf "$WORK_DIR"' EXIT
fi
mkdir -p "$WORK_DIR" "$OUT_DIR"

JOBS="$(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2)"
OQS_PREFIX="$WORK_DIR/liboqs-prefix"
EXT_DIR="$WORK_DIR/ext"

case "$(uname -s)" in
  Darwin)
    OS=darwin-bsdlibc
    # Binaries must load on older macOS than the build machine.
    export MACOSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET:-13.0}"
    ;;
  Linux)
    if ldd --version 2>&1 | grep -qi musl; then OS=linux-musl; else OS=linux-glibc; fi
    ;;
  *) echo "unsupported OS: $(uname -s)" >&2; exit 1 ;;
esac
case "$(uname -m)" in
  aarch64 | arm64) ARCH=arm64 ;;
  x86_64 | amd64) ARCH=x86_64 ;;
  *) ARCH="$(uname -m)" ;;
esac

echo "==> liboqs $LIBOQS_VERSION"
if [ ! -f "$OQS_PREFIX/lib/liboqs.a" ]; then
  mkdir -p "$WORK_DIR/liboqs-src"
  curl -fsSL "https://github.com/open-quantum-safe/liboqs/archive/refs/tags/$LIBOQS_VERSION.tar.gz" |
    tar -xz -C "$WORK_DIR/liboqs-src" --strip-components=1
  # OQS_DIST_BUILD: no -march=native, CPU features are detected at runtime,
  # so the binary runs on machines other than the one that built it.
  cmake -S "$WORK_DIR/liboqs-src" -B "$WORK_DIR/liboqs-build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=OFF \
    -DOQS_BUILD_ONLY_LIB=ON \
    -DOQS_DIST_BUILD=ON \
    -DOQS_USE_OPENSSL=OFF \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DCMAKE_INSTALL_PREFIX="$OQS_PREFIX" >/dev/null
  cmake --build "$WORK_DIR/liboqs-build" --parallel "$JOBS" >/dev/null
  cmake --install "$WORK_DIR/liboqs-build" >/dev/null
fi

echo "==> extension"
rm -rf "$EXT_DIR"
mkdir -p "$EXT_DIR"
cp "$SRC_DIR"/config.m4 "$SRC_DIR"/*.c "$SRC_DIR"/*.h "$EXT_DIR"/
rm -f "$EXT_DIR"/config.h
mkdir "$EXT_DIR/tests"
cp "$SRC_DIR"/tests/*.phpt "$EXT_DIR/tests/"

cd "$EXT_DIR"
"$PHPIZE" >/dev/null
OQS_COMMIT="$LIBOQS_VERSION" ./configure --with-php-config="$PHP_CONFIG" --with-oqs="$OQS_PREFIX" >/dev/null
make -j"$JOBS" >/dev/null

echo "==> tests"
NO_INTERACTION=1 make test TESTS="-q tests" >"$WORK_DIR/test.log" 2>&1 || true
grep -E '^Tests (failed|passed)' "$WORK_DIR/test.log" || true
if ! grep -Eq '^Tests failed +: +0 ' "$WORK_DIR/test.log" ||
  ! grep -Eq '^Tests passed +: +[1-9]' "$WORK_DIR/test.log"; then
  cat "$WORK_DIR/test.log"
  cat tests/*.diff 2>/dev/null || true
  echo "tests failed" >&2
  exit 1
fi

echo "==> sanity checks"
SO="$EXT_DIR/modules/oqs.so"
# -n: ignore php.ini, so an already installed oqs extension cannot interfere
LINKED="$("$PHP" -n -d "extension=$SO" -r 'echo (new ReflectionExtension("oqs"))->info() ?? "";' 2>/dev/null |
  sed -n 's/^liboqs library version => //p')"
if [ "$LINKED" != "$LIBOQS_VERSION" ]; then
  echo "expected liboqs $LIBOQS_VERSION to be linked in, got '$LINKED'" >&2
  exit 1
fi
case "$OS" in
  darwin-*) DEPS="$(otool -L "$SO")" ;;
  *) DEPS="$(ldd "$SO")" ;;
esac
if echo "$DEPS" | grep -qi liboqs; then
  echo "liboqs is linked dynamically, expected static:" >&2
  echo "$DEPS" >&2
  exit 1
fi
if [ "$OS" = linux-glibc ] && command -v objdump >/dev/null 2>&1; then
  echo "newest glibc symbol required: $(objdump -T "$SO" | grep -o 'GLIBC_[0-9.]*' | sort -uV | tail -1)"
fi

# Package using the asset naming PIE (https://github.com/php/pie) looks for with
# "download-url-method": "pre-packaged-binary", so the same file serves both
# "pie install" and a manual download:
#   php_oqs-<version>_php<X.Y>-<arch>-<os>-<libc>[-zts].zip, containing oqs.so
EXT_VERSION="$(sed -n 's/^#define PHP_OQS_VERSION "\(.*\)"/\1/p' "$SRC_DIR/php_oqs.h")"
PHP_VERSION="$("$PHP_CONFIG" --version | cut -d. -f1,2)"
if "$PHP" -n -r 'exit(PHP_ZTS ? 0 : 1);'; then TS=-zts; else TS=; fi
NAME="php_oqs-${EXT_VERSION}_php$PHP_VERSION-$ARCH-$OS$TS.zip"

case "$OS" in
  darwin-*) strip -x "$SO" ;;
  *) strip --strip-unneeded "$SO" ;;
esac
# the stripped file is what gets shipped, so make sure it still loads
"$PHP" -n -d "extension=$SO" -r '(new Oqs\Kem("ML-KEM-768"))->keypair();'

rm -f "$OUT_DIR/$NAME"
zip -q -j "$OUT_DIR/$NAME" "$SO"
echo "==> $OUT_DIR/$NAME ($(wc -c <"$OUT_DIR/$NAME" | tr -d ' ') bytes)"
