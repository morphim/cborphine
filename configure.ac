#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([libcborphine], [0.1], [https://github.com/morphim/cborphine/issues])
AC_CONFIG_SRCDIR([include/pstdint.h])
#AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([foreign subdir-objects])
LT_INIT

# Checks for programs.
AC_PROG_CC

# Checks for libraries.

# Checks for header files.
AC_CHECK_HEADERS([stdlib.h string.h stddef.h limits.h signal.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT64_T
AC_TYPE_INT8_T
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T
AC_TYPE_UINT8_T
AC_CHECK_TYPES([ptrdiff_t])

# Checks for library functions.


AC_C_BIGENDIAN(
    AC_DEFINE(CBOR_BIGENDIAN_PLATFORM, 1, [machine is bigendian]),
    AC_DEFINE(CBOR_BIGENDIAN_PLATFORM, 0, [machine is littleendian]),
    AC_MSG_ERROR(unknown endianess),
    AC_MSG_ERROR(universial endianess not supported)
)

AC_CHECK_SIZEOF([size_t])
AS_IF([test "$ac_cv_sizeof_size_t" -eq 8],
    [AC_DEFINE([CBOR_INT64_SUPPORT], 1, [Target platform is 64-bit.])],
    [AC_DEFINE([CBOR_INT64_SUPPORT], 0, [Target platform is 32-bit.])]
)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
