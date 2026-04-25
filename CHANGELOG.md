# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [1.2.1] - 2026-04-25

### Changed

- Upgraded **gnutls** from `3.8.9` to `3.8.12`
- Upgraded **SDL** from `2.0.8` to `2.32.10`
- Upgrade Flutter package version to `7.2.1`

### Fixed

- Added `APP_ALLOW_MISSING_DEPS=true` in `android.sh` to prevent build failures when dependencies
  are missing (fix for `c++_shared`)
- Added **16KB page size flag** in `function-android.sh`
- Fixed loading `flock` from `sys/file.h` in `libuuid.sh`
- Added `CMAKE_POLICY_VERSION_MINIMUM=3.5` before running CMake to suppress deprecation warnings
  from NDK 27 toolchain files in `cpu-features.sh`
- Added `-Wno-single-bit-bitfield-constant-conversion` to `MY_CFLAGS` in `Android.mk` for **NDK 27 /
  Clang 18** compatibility

- Prepared **gnutls.sh** source for NDK 27 build:
    - Fixed duplicate autoconf macro in `configure.ac`
    - Added missing auxiliary files required by automake
    - Explicitly defined `CRAU_MAYBE_UNUSED` (NDK 27 no longer accepts implicit definition)