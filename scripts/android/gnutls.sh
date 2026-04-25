#!/bin/bash

# INIT SUBMODULES
${SED_INLINE} 's|openssl/openssl|arthenica/openssl|g' "${BASEDIR}"/src/"${LIB_NAME}"/.gitmodules || return 1
${SED_INLINE} 's|tomato42|arthenica|g' "${BASEDIR}"/src/"${LIB_NAME}"/.gitmodules || return 1
${SED_INLINE} 's|warner|arthenica|g' "${BASEDIR}"/src/"${LIB_NAME}"/.gitmodules || return 1
${SED_INLINE} 's|gitlab.com/libidn/gnulib-mirror|github.com/arthenica/gnulib|g' "${BASEDIR}"/src/"${LIB_NAME}"/.gitmodules || return 1
${SED_INLINE} 's|gitlab.com/gnutls/libtasn1|github.com/arthenica/libtasn1|g' "${BASEDIR}"/src/"${LIB_NAME}"/.gitmodules || return 1
${SED_INLINE} 's|gitlab.com/gnutls/nettle|github.com/arthenica/nettle|g' "${BASEDIR}"/src/"${LIB_NAME}"/.gitmodules || return 1
${SED_INLINE} 's|gitlab.com/gnutls/abi-dump|github.com/arthenica/abi-dump|g' "${BASEDIR}"/src/"${LIB_NAME}"/.gitmodules || return 1
${SED_INLINE} 's|gitlab.com/gnutls/cligen|github.com/arthenica/cligen|g' "${BASEDIR}"/src/"${LIB_NAME}"/.gitmodules || return 1
${SED_INLINE} 's|gitlab.com/redhat-crypto/tests/interop|github.com/arthenica/redhat-crypto-tests-interop|g' "${BASEDIR}"/src/"${LIB_NAME}"/.gitmodules || return 1

# UPDATE BUILD FLAGS
export CFLAGS="$(get_cflags ${LIB_NAME}) -I${LIB_INSTALL_BASE}/libiconv/include"
export CXXFLAGS=$(get_cxxflags "${LIB_NAME}")
export LDFLAGS="$(get_ldflags ${LIB_NAME}) -L${LIB_INSTALL_BASE}/libiconv/lib"

export NETTLE_CFLAGS="-I${LIB_INSTALL_BASE}/nettle/include"
export NETTLE_LIBS="-L${LIB_INSTALL_BASE}/nettle/lib -lnettle -L${LIB_INSTALL_BASE}/gmp/lib -lgmp"
export HOGWEED_CFLAGS="-I${LIB_INSTALL_BASE}/nettle/include"
export HOGWEED_LIBS="-L${LIB_INSTALL_BASE}/nettle/lib -lhogweed -L${LIB_INSTALL_BASE}/gmp/lib -lgmp"
export GMP_CFLAGS="-I${LIB_INSTALL_BASE}/gmp/include"
export GMP_LIBS="-L${LIB_INSTALL_BASE}/gmp/lib -lgmp"

# SET BUILD OPTIONS
ASM_OPTIONS=""
case ${ARCH} in
x86)
  ASM_OPTIONS="--disable-hardware-acceleration"
  ;;
*)
  ASM_OPTIONS="--enable-hardware-acceleration"
  ;;
esac

# ALWAYS CLEAN THE PREVIOUS BUILD
make distclean 2>/dev/null 1>/dev/null

# REGENERATE BUILD FILES IF NECESSARY OR REQUESTED
if [[ ! -f "${BASEDIR}"/src/"${LIB_NAME}"/configure ]] || [[ ${RECONF_gnutls} -eq 1 ]]; then

  # autoconf fails when AM_GNU_GETTEXT_REQUIRE_VERSION appears more than once (NDK 27)
  awk '/AM_GNU_GETTEXT_REQUIRE_VERSION/{if(seen++)next} 1' \
    "${BASEDIR}/src/${LIB_NAME}/configure.ac" \
    > "${BASEDIR}/src/${LIB_NAME}/configure.ac.tmp" \
    && mv "${BASEDIR}/src/${LIB_NAME}/configure.ac.tmp" \
          "${BASEDIR}/src/${LIB_NAME}/configure.ac"

  ./bootstrap --skip-po || return 1
  git submodule update --remote gnulib || return 1
  overwrite_file ./gnulib/lib/fpending.c ./src/gl/fpending.c || return 1

   # automake --add-missing copies required auxiliary files (install-sh, missing, depcomp)
    # that may not exist in the source tree when building with newer toolchains (NDK 27)
  automake --add-missing --copy 2>/dev/null || true
fi

# CRAU_MAYBE_UNUSED is used in source but not defined when building with NDK 27
# prepend the define to crau.h using cat to avoid macOS sed -i incompatibility
# ffmpeg-kit-patch marker prevents double-patching on repeated builds
CRAU_H="${BASEDIR}/src/${LIB_NAME}/lib/crau/crau.h"
if [[ -f "${CRAU_H}" ]]; then
  if ! grep -q "ffmpeg-kit-patch" "${CRAU_H}"; then
    printf '/* ffmpeg-kit-patch */\n#define CRAU_MAYBE_UNUSED __attribute__((unused))\n' \
      | cat - "${CRAU_H}" > "${CRAU_H}.tmp" \
      && mv "${CRAU_H}.tmp" "${CRAU_H}"
    echo "INFO: patched crau.h with CRAU_MAYBE_UNUSED"
  fi
else
  echo "ERROR: crau.h not found at ${CRAU_H}"
  find "${BASEDIR}/src/${LIB_NAME}" -name "crau.h" 2>/dev/null
  return 1
fi

./configure \
  --prefix="${LIB_INSTALL_PREFIX}" \
  --with-pic \
  --with-sysroot="${ANDROID_SYSROOT}" \
  --with-included-libtasn1 \
  --with-included-unistring \
  --without-idn \
  --without-p11-kit \
  ${ASM_OPTIONS} \
  --enable-static \
  --disable-openssl-compatibility \
  --disable-shared \
  --disable-fast-install \
  --disable-code-coverage \
  --disable-doc \
  --disable-manpages \
  --disable-guile \
  --disable-tests \
  --disable-tools \
  --disable-maintainer-mode \
  --disable-full-test-suite \
  --host="${HOST}" || return 1

make -j$(get_cpu_count) || return 1

make install || return 1

# CREATE PACKAGE CONFIG MANUALLY
create_gnutls_package_config "3.7.9" || return 1