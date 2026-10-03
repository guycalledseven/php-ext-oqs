#!/bin/sh
# Build the Linux binary for one PHP version inside the official php image.
#
#   ci/docker-build.sh 8.4            # host architecture
#   PLATFORM=linux/amd64 ci/docker-build.sh 8.4
#
# The image is pinned to Debian bookworm so the binary only needs an old glibc.
set -eu

PHP_VERSION="${1:?usage: ci/docker-build.sh <php version, e.g. 8.4>}"
SRC_DIR="$(cd "$(dirname "$0")/.." && pwd)"

exec docker run --rm \
  ${PLATFORM:+--platform "$PLATFORM"} \
  -e LIBOQS_VERSION \
  -v "$SRC_DIR":/src \
  "php:$PHP_VERSION-cli-bookworm" \
  sh -c 'apt-get update -qq >/dev/null && apt-get install -y -qq --no-install-recommends cmake binutils zip >/dev/null && /src/ci/build.sh'
