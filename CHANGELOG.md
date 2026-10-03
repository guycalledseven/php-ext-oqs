# Changelog

All notable changes to this project are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.2.0] - 2026-10-03

### Security
- `Kem::encap()`, `Kem::decap()`, `Sig::sign()` and `Sig::verify()` now validate
  the length of keys and ciphertexts before passing them to liboqs. Previously a
  too-short string caused an out-of-bounds read. A wrong length throws
  `Oqs\Exception` (`Invalid <what> length: expected N bytes, got M`).
- `Sig::verify()` returns `false` for a signature longer than the algorithm's
  maximum instead of handing it to liboqs.
- Secret key and signature buffers are wiped on error paths in `sig.c`, and the
  secret key buffer on `Kem::keypair()` failure (previously only `Kem::encap()`
  and `Kem::decap()` did this).

### Removed
- **BC break:** `Oqs\randombytes_switch_algorithm()` and the constants
  `Oqs\RAND_ALG_SYSTEM` / `Oqs\RAND_ALG_OPENSSL`. The switch is process-global in
  liboqs, so it leaked between requests in the same PHP-FPM worker and was not
  safe under ZTS. The system RNG is always used.

### Changed
- Bundled liboqs: **0.16.0** (previously tested against a 0.14.1-dev commit). Algorithm
  constants follow the linked liboqs, so with 0.16.0:
  - added KEMs: `HQC-1`, `HQC-3`, `HQC-5`, `eFrodoKEM-{640,976,1344}-{AES,SHAKE}`
  - added signatures: `mqom2_*` (12 variants)
  - removed signatures: `SPHINCS+-*` (12 variants; use the `SLH_DSA_*` names)
- `config.m4`: an explicit `--with-oqs=DIR` now takes precedence over pkg-config
  (it used to be ignored whenever pkg-config knew a liboqs), and the header is
  looked up in `DIR/include` instead of the compiler's default path.
- `config.m4`: configure fails if `DIR/lib` contains no liboqs, instead of
  silently linking whichever liboqs the linker finds elsewhere.
- `OQS_LIB_VERSION` is derived from the liboqs headers (was hard-coded to
  `liboqs 0.11.0`).
- phpinfo shows both the liboqs headers version and the runtime library version.

### Fixed
- `OQS_LIB_COMMIT` / phpinfo "liboqs commit" now reflect the `OQS_COMMIT`
  configure variable (it was always `unknown` unless passed via `CPPFLAGS`).
- Static macOS builds no longer pass `-force_load` twice.
- README: test command (`make test`, not `run_tests.php`).

### Added
- Prebuilt binaries (liboqs linked statically) attached to every GitHub release:
  PHP 8.1 - 8.5 NTS for Linux glibc x86_64/arm64 and macOS arm64.
- `ci/build.sh` / `ci/docker-build.sh`: reproducible build + test + packaging,
  used both locally and by the `.github/workflows/build.yml` workflow.
- `composer.json` for [PIE](https://github.com/php/pie)
  (`pie install guycalledseven/php-ext-oqs`), using the prebuilt binaries when available.
- `tests/008-input-lengths.phpt`.
- `CHANGELOG.md`.

## [0.1.0]

Initial version: `Oqs\Kem`, `Oqs\Sig`, `Oqs\Exception`, `ALG_*` class constants,
`sizes()`, `details()`, `algorithms()`, liboqs version constants.
