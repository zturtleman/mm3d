AC_PREREQ(2.61)

dnl If you change the version, also change the following files
dnl    src/mm3dcore/version.h (for x.x.x)
dnl    src/win_resource.rc (for x.x.x)
dnl    mm3d-win32-installer.nsi (for x.x.x)
dnl    Makefile.am (for Contents/PlugIns/mm3d/x.x)
dnl    Makefile.generic (for plugins/x.x)
dnl    plugins/Makefile.am (for plugins/x.x)

AC_INIT([mm3d], [1.3.12])
AM_INIT_AUTOMAKE
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_HEADERS([config.h])

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_RANLIB
KSW_IS_PROFILE
KSW_IS_DEBUG
ZTM_WITH_MACOSX_VERSION_MIN

# Checks for libraries.

# Checks for header files.
AC_HEADER_DIRENT
AC_HEADER_STDC
AC_HEADER_SYS_WAIT
AC_CHECK_HEADERS([limits.h netinet/in.h stddef.h stdint.h stdlib.h string.h sys/time.h sys/timeb.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STAT
AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE
AC_TYPE_INT16_T
AC_TYPE_INT32_T
AC_TYPE_INT8_T
AC_TYPE_MODE_T
AC_TYPE_SIZE_T
AC_HEADER_TIME
AC_STRUCT_TM
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# Checks for library functions.
AC_FUNC_CLOSEDIR_VOID
AC_FUNC_ERROR_AT_LINE
AC_FUNC_FORK
AC_FUNC_LSTAT
AC_FUNC_LSTAT_FOLLOWS_SLASHED_SYMLINK
AC_FUNC_MALLOC
AC_FUNC_MEMCMP
AC_FUNC_REALLOC
AC_TYPE_SIGNAL
AC_FUNC_STAT
AC_FUNC_VPRINTF
AC_CHECK_FUNCS([ftime getcwd gettimeofday localtime_r memmove memset mkdir pow realpath sqrt strcasecmp strchr strcspn strdup strerror strncasecmp strrchr strspn strstr])

KSW_HAVE_LUA
KSW_HAVE_LUALIB
KSW_IS_OSX
KSW_HAVE_DLOPEN
MDL_HAVE_OPENGL
BNV_HAVE_QT
KSW_HAVE_GETTIMEOFDAY
dnl AC_C_BIGENDIAN_CROSS( AC_DEFINE( MM3D_BIGENDIAN ), AC_DEFINE( MM3D_LITTLEENDIAN ), AC_DEFINE(MM3D_UNKNOWNENDIAN) )

if test x"$prefix" != xNONE; then
AC_DEFINE_UNQUOTED( [PREFIX], ["$prefix"], [Define installation prefix] )
else
AC_DEFINE_UNQUOTED( [PREFIX], ["$ac_default_prefix"], [Define installation prefix] )
fi

AC_SUBST(QT_LIBS)
AC_SUBST(QT_CXXFLAGS)
AC_SUBST(QT_CXXFLAGS)
AC_SUBST(DLOPEN_LIBS)
AC_SUBST(HAVE_QT5)
AC_SUBST(BYTEORDER)
AC_SUBST(PROFILE)
AC_SUBST(CORE_PROFILE)

if test x"${QT_LIBS}" = x; then
   echo ""
   AC_MSG_ERROR([Qt is required.  If you have Qt installed see --help for more options])
   echo ""
fi

if test x"$have_GLU" != xyes; then
   echo ""
   AC_MSG_ERROR([OpenGL (with GLU) is required.])
   echo ""
fi

if test x"${ksw_cv_qgl_test_result}" = xfailure; then
   echo ""
   AC_MSG_ERROR([Qt with OpenGL is required.])
   echo ""
fi

AC_CONFIG_FILES([ Makefile 
          src/Makefile
          src/libmm3d/Makefile
          src/mm3dcore/Makefile
          src/depui/Makefile
          src/qtui/Makefile
          src/qtui/images/Makefile
          src/implui/Makefile
          src/tools/Makefile
          src/commands/Makefile
          src/pixmap/Makefile
          plugins/Makefile
          man/Makefile
          i18n/Makefile
          desktop/Makefile
          doc/Makefile
          doc/html/Makefile
          doc/html/olh_images/Makefile
          doc/html/olh_images/screencaps/Makefile
          doc/html/olh_images/tools/Makefile
          ])
AC_OUTPUT

if test x"${have_dlopen}" != xyes; then
   echo ""
   echo "Warning: dlopen was not detected.  You can contiue with the"
   echo "install, but plugins will not work."
   echo ""
fi

