AC_DEFUN([KSW_IS_PROFILE],
[
  AC_REQUIRE([AC_PROG_CC])

  AC_MSG_CHECKING(for profile)

  AC_ARG_ENABLE([profile],
    [  --enable-profile=yes/no/core       Specify "yes" or "core" to enable profiling.])

  is_profile=no

  PROFILE=
  CORE_PROFILE=

  if test x"$enable_profile" = xyes; then
     PROFILE=-pg
     CORE_PROFILE=-pg
     is_profile=yes
  elif test x"$enable_profile" = xcore; then
     CORE_PROFILE=-pg
     is_profile="yes (core)"
  fi

  AC_DEFINE( [PROFILE], [], [Define to include profiling information] )
  AC_DEFINE( [CORE_PROFILE], [], [Define to include core profiling information] )

  AC_MSG_RESULT($is_profile)
])

AC_DEFUN([KSW_IS_DEBUG],
[
  AC_REQUIRE([AC_PROG_CC])

  AC_MSG_CHECKING(for debug)

  AC_ARG_ENABLE([debug],
    [  --enable-debug=yes/no/cov   Specify "yes" to enable a debug build.])

  is_debug=no

  enable_debug_save_COVFLAGS="${COVFLAGS}"
  enable_debug_save_COVLFLAGS="${COVLFLAGS}"
  enable_debug_save_CFLAGS="${CFLAGS}"
  enable_debug_save_CXXFLAGS="${CXXFLAGS}"
  enable_debug_save_LDFLAGS="${LDFLAGS}"
  if test x"$enable_debug" = xyes; then
    COVFLAGS=""
    COVLFLAGS=""
    CFLAGS="-g -fPIC"
    CXXFLAGS="${CFLAGS}"
    LDFLAGS=""
  elif test x"$enable_debug" = xcov; then
    COVFLAGS="-coverage"
    COVLFLAGS="-lgcov"
    CFLAGS="-g -fPIC"
    CXXFLAGS="${CFLAGS}"
    LDFLAGS=""
    is_debug=coverage
  else
    omit_frame=
    # FIXME?: Using -fomit-frame-pointer causes SEGFAULT at start up on macOS 10.11 using clang++.
    #if test x"${CORE_PROFILE}" = "x"; then
    #   omit_frame="-fomit-frame-pointer"
    #fi
    COVFLAGS=""
    COVLFLAGS=""
    CFLAGS="-O2 ${omit_frame} -fno-math-errno -fPIC"
    CXXFLAGS="${CFLAGS}"
    LDFLAGS="${omit_frame} -fno-math-errno"
  fi

   AC_TRY_LINK([#include <stdio.h>], , [
dnl Yay! 
    if test x"$enable_debug" = xyes; then
      AC_DEFINE( [CODE_DEBUG], [], [Define to include debugging information] )
      is_debug=yes
    fi
   ], [
dnl Boo! 
    if test x"$enable_debug" != xyes; then
      COVFLAGS="${enable_debug_save_COVFLAGS}"
      COVLFLAGS="${enable_debug_save_COVLFLAGS}"
      CFLAGS="${enable_debug_save_CFLAGS}"
      CXXFLAGS="${enable_debug_save_CXXFLAGS}"
      LDFLAGS="${enable_debug_save_LDFLAGS}"
      AC_DEFINE( [CODE_DEBUG], [], [Define to include debugging information] )
      is_debug=yes
    else
      COVFLAGS="${enable_debug_save_COVFLAGS}"
      COVLFLAGS="${enable_debug_save_COVLFLAGS}"
      CFLAGS="${enable_debug_save_CFLAGS}"
      CXXFLAGS="${enable_debug_save_CXXFLAGS}"
      LDFLAGS="${enable_debug_save_LDFLAGS}"
    fi
   ])
  AC_SUBST(COVFLAGS)
  AC_SUBST(COVLFLAGS)

  AC_MSG_RESULT($is_debug)
])

dnl for lua (KSW was here)
AC_DEFUN([KSW_HAVE_LUA],
[
  AC_REQUIRE([AC_PROG_CC])

  AC_MSG_CHECKING(for lua)

  AC_ARG_WITH([lua-dir],
    [  --with-lua-dir=DIR       DIR is equal to the install prefix of Lua.
                          Header files are in DIR/include, and Library 
                          files are in DIR/lib])
  AC_ARG_WITH([lua-include-dir],
    [  --with-lua-include-dir=DIR
                          Lua header files are in DIR])
  AC_ARG_WITH([lua-lib-dir],
    [  --with-lua-lib-dir=DIR   Lua libraries are in DIR])
  AC_ARG_WITH([lua-lib],
    [  --with-lua-lib=LIB       Use -lLIB to link with the lua library])
  if test x"$with_lua_dir" = x &&
     test x"$with_lua_include_dir" = x &&
     test x"$with_lua_lib_dir" = x &&
     test x"$with_lua_lib" = x; then
    # user did not request Lua support, disable it
    have_lua="disabled"
  else
    # "yes" is a bogus option
    if test x"$with_lua_dir" = xyes; then
      with_lua_dir=
    fi
    if test x"$with_lua_include_dir" = xyes; then
      with_lua_include_dir=
    fi
    if test x"$with_lua_lib_dir" = xyes; then
      with_lua_lib_dir=
    fi
    if test x"$with_lua_lib" = xyes; then
      with_lua_lib=
    fi
    # No lua unless we discover otherwise
    have_lua=no
    # Check whether we are requested to link with a specific version
    if test x"$with_lua_lib" != x; then
      ksw_lua_lib="$with_lua_lib"
    fi
    # Check whether we were supplied with an answer already
    if test x"$with_lua_dir" != x; then
      have_lua=yes
      ksw_lua_dir="$with_lua_dir"
      ksw_lua_include_dir="$with_lua_dir/include"
      if test -d "$with_lua_dir/lib64" ; then
        ksw_lua_lib_dir="$with_lua_dir/lib64"
      else
        ksw_lua_lib_dir="$with_lua_dir/lib"
      fi
      # Only search for the lib if the user did not define one already
      if test x"$ksw_lua_lib" = x; then
        ksw_lua_lib="`ls $ksw_lua_lib_dir/liblua5* 2> /dev/null | sed -n 1p |
                     sed s@$ksw_lua_lib_dir/lib@@ | [sed s@[.].*@@]`"
      fi
      if test x"$ksw_lua_lib" = x; then
        ksw_lua_lib="`ls $ksw_lua_lib_dir/liblua.* 2> /dev/null | sed -n 1p |
                     sed s@$ksw_lua_lib_dir/lib@@ | [sed s@[.].*@@]`"
      fi
      ksw_lua_LIBS="-L$ksw_lua_lib_dir -l$ksw_lua_lib -lm -ldl"
    else
      # Use cached value or do search, starting with suggestions from
      # the command line
      AC_CACHE_VAL(ksw_cv_have_lua,
      [
        # We are not given a solution and there is no cached value.
        ksw_lua_dir=
        if test x"$ksw_lua_include_dir" = x; then
           ksw_lua_include_dir="`ls -dr /usr/include/lua.h /usr/local/include/lua.h /usr/include/lua*/lua.h /usr/local/include/lua*/lua.h 2> /dev/null | sed -n 1p |
                        sed s@/lua.h@@`"
        fi
        if test x"$ksw_lua_lib" = x; then
           ksw_lua_lib_dir="`ls -dr /usr/lib64/liblua5* /usr/lib64/liblua.* /usr/local/lib64/liblua5* /usr/local/lib64/liblua.* /usr/lib/liblua5* /usr/lib/liblua.* /usr/local/lib/liblua5* /usr/local/lib/liblua.* 2> /dev/null | sed -n 1p`"
           ksw_lua_lib="`echo $ksw_lua_lib_dir | sed 's@/.*/@@' `"
           ksw_lua_lib_dir="`echo $ksw_lua_lib_dir | sed s@/$ksw_lua_lib@@ `"
           ksw_lua_lib="`echo $ksw_lua_lib | [sed s@[.].*@@] | sed s@^lib@@ `"
        fi

        if test x"$ksw_lua_lib" != x; then
          if test x"$ksw_lua_lib_dir" = x; then
             ksw_lua_LIBS="-l$ksw_lua_lib -lm -ldl"
          else
             ksw_lua_LIBS="-L$ksw_lua_lib_dir -l$ksw_lua_lib -lm -ldl"
          fi
          # Record where we found lua for the cache.
          ksw_cv_have_lua="have_lua=yes                  \
                       ksw_lua_dir=$ksw_lua_dir          \
               ksw_lua_include_dir=$ksw_lua_include_dir  \
                      ksw_lua_LIBS=\"$ksw_lua_LIBS\""
        fi
      ])dnl
      eval "$ksw_cv_have_lua"
    fi # all $ksw_lua_* are set
  fi   # $have_lua reflects the system status
  if test x"$have_lua" = xyes; then
    LUA_CCFLAGS="-I$ksw_lua_include_dir"
    LUA_DIR="$ksw_lua_dir"
    LUA_LIBS="$ksw_lua_LIBS"
    # All variables are defined, report the result
    AC_DEFINE( [HAVE_LUA], [], [Define when you have Lua installed] )
    AC_MSG_RESULT([$have_lua:
    LUA_CCFLAGS=$LUA_CCFLAGS
    LUA_DIR=$LUA_DIR
    LUA_LIBS=$LUA_LIBS ])
  else
    # lua was not found
    LUA_CCFLAGS=
    LUA_DIR=
    LUA_LIBS=
    AC_MSG_RESULT($have_lua)
  fi
  AC_SUBST(LUA_CCFLAGS)
  AC_SUBST(LUA_DIR)
  AC_SUBST(LUA_LIBS)

  #### Being paranoid:
  if test x"$have_lua" = xyes; then
    AC_MSG_CHECKING(correct functioning of lua installation)
    AC_CACHE_VAL(ksw_cv_lua_test_result,
    [
      cat > ksw_lua_test.h << EOF
EOF

      cat > ksw_lua_test.c << EOF
#include "ksw_lua_test.h"
#include <lua.h>
#include <lauxlib.h>
//#include <lualib.h>

int main( int argc, char **argv )
{
   lua_State * L = lua_open();
   //luaopen_math( L );
   lua_close( L );
   return( 0 );
}
EOF

      ksw_cv_lua_test_result="failure"
        ksw_try_1="$CC $LUA_CCFLAGS $LUA_LIBS -o ksw_lua_test ksw_lua_test.c >/dev/null 2>ksw_lua_test_1.out"
        AC_TRY_EVAL(ksw_try_1)
        ksw_err_1=`grep -v '^ *+' ksw_lua_test_1.out | grep -v "^ksw_lua_test.{$ac_ext}\$"`
        if test x"$ksw_err_1" != x; then
          echo "$ksw_err_1" >&AC_FD_CC
          echo "configure: could not compile:" >&AC_FD_CC
          cat ksw_lua_test.c >&AC_FD_CC
        else
         ksw_cv_lua_test_result="success"
        fi
      fi
    ])dnl AC_CACHE_VAL ksw_cv_lua_test_result
    AC_MSG_RESULT([$ksw_cv_lua_test_result]);
    if test x"$ksw_cv_lua_test_result" = "xfailure"; then
      AC_MSG_ERROR([Failed to find matching components of a complete
                  lua installation. Try using more options,
                  see ./configure --help.])
    fi

    rm -f ksw_lua_test.h \
          ksw_lua_test.c ksw_lua_test.o ksw_lua_test \
          ksw_lua_test_1.out
])

dnl for lualib (KSW was here)
AC_DEFUN([KSW_HAVE_LUALIB],
[
   if test x"${LUA_LIBS}" != x; then
   
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([KSW_HAVE_LUA])

  AC_MSG_CHECKING(for lualib)

  AC_ARG_WITH([lualib-dir],
    [  --with-lualib-dir=DIR       DIR is equal to the install prefix of Lualib.
                          Header files are in DIR/include, and Library 
                          files are in DIR/lib])
  AC_ARG_WITH([lualib-include-dir],
    [  --with-lualib-include-dir=DIR
                          Lua header files are in DIR])
  AC_ARG_WITH([lualib-lib-dir],
    [  --with-lualib-lib-dir=DIR   Lua libraries are in DIR])
  AC_ARG_WITH([lualib-lib],
    [  --with-lualib-lib=LIB       Use -lLIB to link with the lualib library])
  if test x"$with_lualib_dir" = x"no" ||
     test x"$with_lualib_include_dir" = x"no" ||
     test x"$with_lualib_lib_dir" = x"no" ||
     test x"$with_lualib_lib" = x"no"; then
    # user disabled lualib. Leave cache alone.
    have_lualib="User disabled lualib."
  else
    # "yes" is a bogus option
    if test x"$with_lualib_dir" = xyes; then
      with_lualib_dir=
    fi
    if test x"$with_lualib_include_dir" = xyes; then
      with_lualib_include_dir=
    fi
    if test x"$with_lualib_lib_dir" = xyes; then
      with_lualib_lib_dir=
    fi
    if test x"$with_lualib_lib" = xyes; then
      with_lualib_lib=
    fi
    # No lualib unless we discover otherwise
    have_lualib=no
    # Check whether we are requested to link with a specific version
    if test x"$with_lualib_lib" != x; then
      ksw_lualib_lib="$with_lualib_lib"
    fi
    # Check whether we were supplied with an answer already
    if test x"$with_lualib_dir" != x; then
      have_lualib=yes
      ksw_lualib_dir="$with_lualib_dir"
      ksw_lualib_include_dir="$with_lualib_dir/include"
      if test -d "$with_lualib_dir/lib64" ; then
        ksw_lualib_lib_dir="$with_lualib_dir/lib64"
      else
        ksw_lualib_lib_dir="$with_lualib_dir/lib"
      fi
      # Only search for the lib if the user did not define one already
      if test x"$ksw_lualib_lib" = x; then
        ksw_lualib_lib="`ls $ksw_lualib_lib_dir/liblualib5* 2> /dev/null | sed -n 1p |
                     sed s@$ksw_lualib_lib_dir/lib@@ | [sed s@[.].*@@]`"
      fi
      if test x"$ksw_lualib_lib" = x; then
        ksw_lualib_lib="`ls $ksw_lualib_lib_dir/liblualib.* 2> /dev/null | sed -n 1p |
                     sed s@$ksw_lualib_lib_dir/lib@@ | [sed s@[.].*@@]`"
      fi
      ksw_lualib_LIBS="-L$ksw_lualib_lib_dir -l$ksw_lualib_lib"
    else
      # Use cached value or do search, starting with suggestions from
      # the command line
      AC_CACHE_VAL(ksw_cv_have_lualib,
      [
        # We are not given a solution and there is no cached value.
        ksw_lualib_dir=
        if test x"$ksw_lualib_include_dir" = x; then
           ksw_lualib_include_dir="`ls -dr /usr/include/lualib.h /usr/local/include/lualib.h /usr/include/lua*/lualib.h /usr/local/include/lua*/lualib.h 2> /dev/null | sed -n 1p |
                        sed s@/lualib.h@@`"
        fi
        if test x"$ksw_lualib_lib" = x; then
           ksw_lualib_lib_dir="`ls -dr /usr/lib64/liblualib5* /usr/lib64/liblualib.* /usr/local/lib64/liblualib5* /usr/local/lib64/liblualib.* /usr/lib/liblualib5* /usr/lib/liblualib.* /usr/local/lib/liblualib5* /usr/local/lib/liblualib.* 2> /dev/null | sed -n 1p`"
           ksw_lualib_lib="`echo $ksw_lualib_lib_dir | sed 's@/.*/@@' `"
           ksw_lualib_lib_dir="`echo $ksw_lualib_lib_dir | sed s@/$ksw_lualib_lib@@ `"
           ksw_lualib_lib="`echo $ksw_lualib_lib | [sed s@[.].*@@] | sed s@^lib@@ `"
        fi
        if test x"$ksw_lualib_lib" != x; then
          if test x"$ksw_lualib_lib_dir" = x; then
             ksw_lualib_LIBS="-l$ksw_lualib_lib"
          else
             ksw_lualib_LIBS="-L$ksw_lualib_lib_dir -l$ksw_lualib_lib"
          fi
          # Record where we found lualib for the cache.
          ksw_cv_have_lualib="have_lualib=yes                  \
                       ksw_lualib_dir=$ksw_lualib_dir          \
               ksw_lualib_include_dir=$ksw_lualib_include_dir  \
                      ksw_lualib_LIBS=\"$ksw_lualib_LIBS\""
        fi
      ])dnl
      eval "$ksw_cv_have_lualib"
    fi # all $ksw_lualib_* are set
  fi   # $have_lualib reflects the system status
  if test x"$have_lualib" = xyes; then
    LUALIB_CCFLAGS="-I$ksw_lualib_include_dir"
    LUALIB_DIR="$ksw_lualib_dir"
    LUALIB_LIBS="-lm -ldl $LUA_LIBS $ksw_lualib_LIBS"
    # All variables are defined, report the result
    AC_DEFINE( [HAVE_LUALIB], [], [Define when you have Lua libs installed] )
    AC_MSG_RESULT([$have_lualib:
    LUALIB_CCFLAGS=$LUALIB_CCFLAGS
    LUALIB_DIR=$LUALIB_DIR
    LUALIB_LIBS=$LUALIB_LIBS ])
  else
    # lualib was not found
    LUALIB_CCFLAGS=
    LUALIB_DIR=
    LUALIB_LIBS=
    AC_MSG_RESULT($have_lualib)
  fi
  AC_SUBST(LUALIB_CCFLAGS)
  AC_SUBST(LUALIB_DIR)
  AC_SUBST(LUALIB_LIBS)

  #### Being paranoid:
  if test x"$have_lualib" = xyes; then
    AC_MSG_CHECKING(correct functioning of lualib installation)
    AC_CACHE_VAL(ksw_cv_lualib_test_result,
    [
      cat > ksw_lualib_test.h << EOF
EOF

      cat > ksw_lualib_test.c << EOF
#include "ksw_lualib_test.h"
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

int main( int argc, char **argv )
{
   lua_State * L = lua_open();
   luaopen_math( L );
   lua_close( L );
   return( 0 );
}
EOF

      ksw_cv_lualib_test_result="failure"
        ksw_try_1="$CC $LUALIB_CCFLAGS $LUALIB_LIBS -o ksw_lualib_test ksw_lualib_test.c >/dev/null 2>ksw_lualib_test_1.out"
        AC_TRY_EVAL(ksw_try_1)
        ksw_err_1=`grep -v '^ *+' ksw_lualib_test_1.out | grep -v "^ksw_lualib_test.{$ac_ext}\$"`
        if test x"$ksw_err_1" != x; then
          echo "$ksw_err_1" >&AC_FD_CC
          echo "configure: could not compile:" >&AC_FD_CC
          cat ksw_lualib_test.c >&AC_FD_CC
        else
         ksw_cv_lualib_test_result="success"
        fi
      fi
    ])dnl AC_CACHE_VAL ksw_cv_lualib_test_result
    AC_MSG_RESULT([$ksw_cv_lualib_test_result]);
    if test x"$ksw_cv_lualib_test_result" = "xfailure"; then
      AC_MSG_ERROR([Failed to find matching components of a complete
                  lualib installation. Try using more options,
                  see ./configure --help.])
    fi

    rm -f ksw_lualib_test.h \
          ksw_lualib_test.c ksw_lualib_test.o ksw_lualib_test \
          ksw_lualib_test_1.out
    fi
])

dnl for Qt (KSW was here)
AC_DEFUN([BNV_HAVE_QT],
[
  dnl THANKS! This code includes bug fixes by:
  dnl Tim McClarren.

  AC_REQUIRE([AC_PROG_CXX])
  AC_REQUIRE([AC_PATH_X])
  AC_REQUIRE([AC_PATH_XTRA])

    AC_LANG_SAVE
    AC_LANG_CPLUSPLUS

  AC_MSG_CHECKING(for Qt)


  dnl Recent QT4 doesn't need these anymore. -- garden@acheronte.it
  dnl QT_XLIBS="$X_PRE_LIBS $X_LIBS -lX11 -lXext -lXmu -lXt -lXi $X_EXTRA_LIBS"
  QT_XLIBS=""

  if test x"$no_x" = xyes; then
     QT_XLIBS=""
  fi
  dnl KSW hackish... requires another macro to set is_osx
  if test x"$is_osx" = xyes; then
     QT_XLIBS=""
  fi

  AC_ARG_WITH([Qt-dir],
    [  --with-Qt-dir=DIR       DIR is equal to \$QTDIR if you have followed the
                          installation instructions of Trolltech. Header
                          files are in DIR/include, binary utilities are
                          in DIR/bin and the library is in DIR/lib])
  AC_ARG_WITH([Qt-include-dir],
    [  --with-Qt-include-dir=DIR
                          Qt header files are in DIR])
  AC_ARG_WITH([Qt-bin-dir],
    [  --with-Qt-bin-dir=DIR   Qt utilities such as moc and uic are in DIR])
  AC_ARG_WITH([Qt-lib-dir],
    [  --with-Qt-lib-dir=DIR   The Qt library is in DIR])

  bnv_is_qt5=no
  if test x"$is_osx" = xyes; then
    bnv_qt5_libs="-framework QtCore -framework QtGui -framework QtWidgets -framework QtOpenGL"
  else
    bnv_qt5_libs="-lQt5Core -lQt5Gui -lQt5Widgets -lQt5OpenGL"
  fi
  if test x"$with_Qt_dir" = x"no" ||
     test x"$with_Qt_include_dir" = x"no" ||
     test x"$with_Qt_bin_dir" = x"no" ||
     test x"$with_Qt_lib_dir" = x"no"; then
    # user disabled Qt. Leave cache alone.
    have_qt="User disabled Qt."
  else
    # "yes" is a bogus option
    if test x"$with_Qt_dir" = xyes; then
      with_Qt_dir=
    fi
    if test x"$with_Qt_include_dir" = xyes; then
      with_Qt_include_dir=
    fi
    if test x"$with_Qt_bin_dir" = xyes; then
      with_Qt_bin_dir=
    fi
    if test x"$with_Qt_lib_dir" = xyes; then
      with_Qt_lib_dir=
    fi
    # No Qt unless we discover otherwise
    have_qt=no
    # Check whether we were supplied with an answer already
    if test x"$with_Qt_dir" != x; then
      have_qt=yes
      bnv_qt_dir="$with_Qt_dir"
      bnv_qt_include_dir="$with_Qt_dir/include"
      bnv_qt_bin_dir="$with_Qt_dir/bin"
      if test -d "$with_Qt_dir/lib64" ; then
        bnv_qt_lib_dir="$with_Qt_dir/lib64"
      else
        bnv_qt_lib_dir="$with_Qt_dir/lib"
      fi
      if test x"$is_osx" = xyes; then
        bnv_qt_LIBS="-F$bnv_qt_dir/Frameworks -L$bnv_qt_lib_dir $bnv_qt5_libs $QT_XLIBS "
      else
        bnv_qt_LIBS="-L$bnv_qt_lib_dir $bnv_qt5_libs $QT_XLIBS "
      fi
    else
      # Use cached value or do search, starting with suggestions from
      # the command line
      AC_CACHE_VAL(bnv_cv_have_qt,
      [
        # We are not given a solution and there is no cached value.
        bnv_qt_dir=NO
        bnv_qt_include_dir=NO
        bnv_qt_lib_dir=NO
        BNV_PATH_QT_DIRECT
        if test "$bnv_qt_dir" = NO ||
           test "$bnv_qt_include_dir" = NO ||
           test "$bnv_qt_lib_dir" = NO; then
          # Problem with finding complete Qt.  Cache the known absence of Qt.
          bnv_cv_have_qt="have_qt=no"
        else
          # Record where we found Qt for the cache.
          bnv_cv_have_qt="have_qt=yes                  \
                       bnv_qt_dir=$bnv_qt_dir          \
               bnv_qt_include_dir=$bnv_qt_include_dir  \
                   bnv_qt_bin_dir=$bnv_qt_bin_dir      \
                   bnv_is_qt5=$bnv_is_qt5              \
                      bnv_qt_LIBS=\"$bnv_qt_LIBS\""
        fi
      ])dnl
      eval "$bnv_cv_have_qt"
    fi # all $bnv_qt_* are set
  fi   # $have_qt reflects the system status
  if test x"$have_qt" = xyes; then
    QT_CXXFLAGS="-I$bnv_qt_include_dir"
    QT_DIR="$bnv_qt_dir"
    QT_LIBS="$bnv_qt_LIBS"
    if test x"$bnv_qt_bin_dir" != x; then
      # We were told where to look for the utilities?
      # UIC detection
      if test -x "$bnv_qt_bin_dir/uic"; then
        QT_UIC="$bnv_qt_bin_dir/uic"
      fi
      # MOC detection
      if test -x "$bnv_qt_bin_dir/moc"; then
        QT_MOC="$bnv_qt_bin_dir/moc"
      fi
      # RCC detection
      if test -x "$bnv_qt_bin_dir/rcc"; then
        QT_RCC="$bnv_qt_bin_dir/rcc"
      fi
      # LRELEASE detection
      if test -x "$bnv_qt_bin_dir/lrelease"; then
        QT_LRELEASE="$bnv_qt_bin_dir/lrelease"
      fi
      # MACDEPLOYQT detection
      if test -x "$bnv_qt_bin_dir/macdeployqt"; then
        QT_MACDEPLOYQT="$bnv_qt_bin_dir/macdeployqt"
      fi
    elif test x"$bnv_qt_dir" != x; then
      # If bnv_qt_dir is defined, utilities are expected to be in the
      # bin subdirectory
      # UIC detection
      if test -x "$bnv_qt_dir/bin/uic"; then
        QT_UIC="$bnv_qt_dir/bin/uic"
      fi
      # MOC detection
      if test -x "$bnv_qt_dir/bin/moc"; then
        QT_MOC="$bnv_qt_dir/bin/moc"
      fi
      # RCC detection
      if test -x "$bnv_qt_dir/bin/rcc"; then
        QT_RCC="$bnv_qt_dir/bin/rcc"
      fi
      # LRELEASE detection
      if test -x "$bnv_qt_dir/bin/lrelease"; then
        QT_LRELEASE="$bnv_qt_dir/bin/lrelease"
      fi
      # MACDEPLOYQT detection
      if test -x "$bnv_qt_dir/bin/macdeployqt"; then
        QT_MACDEPLOYQT="$bnv_qt_dir/bin/macdeployqt"
      fi
    fi

    # If binaries are still not set, try /usr/lib/x86_64-linux-gnu/qt5/bin/
    if test x"$host_alias" != x; then
      # set by configure --host
      bnv_qt_lib_host=$host_alias
    else
      bnv_qt_lib_host=`$SHELL "$srcdir/config.guess" | cut -d'-' -f 1,3-4`
    fi
    if test x"$QT_UIC" = x; then
      # UIC detection
      if test -x "/usr/lib/$bnv_qt_lib_host/qt5/bin/uic"; then
        QT_UIC="/usr/lib/$bnv_qt_lib_host/qt5/bin/uic"
      fi
    fi
    if test x"$QT_MOC" = x; then
      # MOC detection
      if test -x "/usr/lib/$bnv_qt_lib_host/qt5/bin/moc"; then
        QT_MOC="/usr/lib/$bnv_qt_lib_host/qt5/bin/moc"
      fi
    fi
    if test x"$QT_RCC" = x; then
      # RCC detection
      if test -x "/usr/lib/$bnv_qt_lib_host/qt5/bin/rcc"; then
        QT_RCC="/usr/lib/$bnv_qt_lib_host/qt5/bin/rcc"
      fi
    fi
    if test x"$QT_LRELEASE" = x; then
      # LRELEASE detection
      if test -x "/usr/lib/$bnv_qt_lib_host/qt5/bin/lrelease"; then
        QT_LRELEASE="/usr/lib/$bnv_qt_lib_host/qt5/bin/lrelease"
      fi
    fi
    if test x"$QT_MACDEPLOYQT" = x; then
      # MACDEPLOYQT detection
      if test -x "/usr/lib/$bnv_qt_lib_host/qt5/bin/macdeployqt"; then
        QT_MACDEPLOYQT="/usr/lib/$bnv_qt_lib_host/qt5/bin/macdeployqt"
      fi
    fi

    # If binaries are still not set, try qtchooser
    if test x"$QT_UIC" = x; then
      # UIC detection
      if test `which qtchooser 2> /dev/null`; then
        QT_UIC="qtchooser -qt=5 -run-tool=uic"
      fi
    fi
    if test x"$QT_MOC" = x; then
      # MOC detection
      if test `which qtchooser 2> /dev/null`; then
        QT_MOC="qtchooser -qt=5 -run-tool=moc"
      fi
    fi
    if test x"$QT_RCC" = x; then
      # RCC detection
      if test `which qtchooser 2> /dev/null`; then
        QT_RCC="qtchooser -qt=5 -run-tool=rcc"
      fi
    fi
    if test x"$QT_LRELEASE" = x; then
      # LRELEASE detection
      if test `which qtchooser 2> /dev/null`; then
        QT_LRELEASE="qtchooser -qt=5 -run-tool=lrelease"
      fi
    fi
    if test x"$QT_MACDEPLOYQT" = x; then
      # MACDEPLOYQT detection
      if test `which qtchooser 2> /dev/null`; then
        QT_MACDEPLOYQT="qtchooser -qt=5 -run-tool=macdeployqt"
      fi
    fi

    # If binaries are still not set, try $PATH
    if test x"$QT_UIC" = x; then
      # UIC detection
      if test `which uic 2> /dev/null`; then
        QT_UIC=`which uic`
      fi
    fi
    if test x"$QT_MOC" = x; then
      # MOC detection
      if test `which moc 2> /dev/null`; then
        QT_MOC=`which moc`
      fi
    fi
    if test x"$QT_RCC" = x; then
      # RCC detection
      if test `which rcc 2> /dev/null`; then
        QT_RCC=`which rcc`
      fi
    fi
    if test x"$QT_LRELEASE" = x; then
      # LRELEASE detection
      if test `which lrelease 2> /dev/null`; then
        QT_LRELEASE=`which lrelease`
      fi
    fi
    if test x"$QT_MACDEPLOYQT" = x; then
      # MACDEPLOYQT detection
      if test `which macdeployqt 2> /dev/null`; then
        QT_MACDEPLOYQT=`which macdeployqt`
      fi
    fi
    # All variables are defined, report the result
    AC_MSG_RESULT([$have_qt:
    QT_CXXFLAGS=$QT_CXXFLAGS
    QT_DIR=$QT_DIR
    QT_LIBS=$QT_LIBS
    QT_UIC=$QT_UIC
    QT_MOC=$QT_MOC
    QT_RCC=$QT_RCC
    QT_LRELEASE=$QT_LRELEASE
    QT_MACDEPLOYQT=$QT_MACDEPLOYQT])
  else
    # Qt was not found
    QT_CXXFLAGS=
    QT_DIR=
    QT_LIBS=
    QT_UIC=
    QT_MOC=
    QT_RCC=
    QT_LRELEASE=
    QT_MACDEPLOYQT=
    AC_MSG_RESULT($have_qt)
  fi
  if test x"$bnv_is_qt5" = xyes; then
    HAVE_QT5=1
    AC_SUBST(HAVE_QT5)
    AC_DEFINE( [HAVE_QT5], [], [Define when you have QT5 installed] )
  fi
  AC_SUBST(QT_CXXFLAGS)
  AC_SUBST(QT_DIR)
  AC_SUBST(QT_LIBS)
  AC_SUBST(QT_UIC)
  AC_SUBST(QT_MOC)
  AC_SUBST(QT_RCC)
  AC_SUBST(QT_LRELEASE)
  AC_SUBST(QT_MACDEPLOYQT)


  #### Being paranoid:
  if test x"$have_qt" = xyes; then
    AC_MSG_CHECKING(correct functioning of Qt installation)
    AC_CACHE_VAL(bnv_cv_qt_test_result,
    [
      cat > bnv_qt_test.h << EOF
#include <QtCore/QObject>
class Test : public QObject
{
Q_OBJECT
public:
  Test() {}
  ~Test() {}
public slots:
  void receive() {}
signals:
  void send();
};
EOF

      cat > bnv_qt_main.$ac_ext << EOF
#include "bnv_qt_test.h"
#include <QtWidgets/QApplication>
int main( int argc, char **argv )
{
  QApplication app( argc, argv );
  Test t;
  QObject::connect( &t, SIGNAL(send()), &t, SLOT(receive()) );
}
EOF

      bnv_cv_qt_test_result="failure"
      bnv_try_1="$QT_MOC bnv_qt_test.h -o moc_bnv_qt_test.$ac_ext >/dev/null 2>bnv_qt_test_1.out"
      AC_TRY_EVAL(bnv_try_1)
      bnv_err_1=`grep -v '^ *+' bnv_qt_test_1.out | grep -v "^bnv_qt_test.h\$"`
      if test x"$bnv_err_1" != x; then
        echo "$bnv_err_1" >&AC_FD_CC
        echo "configure: could not run $QT_MOC on:" >&AC_FD_CC
        cat bnv_qt_test.h >&AC_FD_CC
      else
        bnv_try_2="$CXX $QT_CXXFLAGS -c $CXXFLAGS -Wno-non-virtual-dtor -o moc_bnv_qt_test.o moc_bnv_qt_test.$ac_ext >/dev/null 2>bnv_qt_test_2.out"
        AC_TRY_EVAL(bnv_try_2)
        bnv_err_2=`grep -v '^ *+' bnv_qt_test_2.out | grep -v "^bnv_qt_test.{$ac_ext}\$"`
        if test x"$bnv_err_2" != x; then
          echo "$bnv_err_2" >&AC_FD_CC
          echo "configure: could not compile:" >&AC_FD_CC
          cat bnv_qt_test.$ac_ext >&AC_FD_CC
        else
          bnv_try_3="$CXX $QT_CXXFLAGS -c $CXXFLAGS -o bnv_qt_main.o bnv_qt_main.$ac_ext >/dev/null 2>bnv_qt_test_3.out"
          AC_TRY_EVAL(bnv_try_3)
          bnv_err_3=`grep -v '^ *+' bnv_qt_test_3.out | grep -v "^bnv_qt_main.{$ac_ext}\$"`
          if test x"$bnv_err_3" != x; then
            echo "$bnv_err_3" >&AC_FD_CC
            echo "configure: could not compile:" >&AC_FD_CC
            cat bnv_qt_main.$ac_ext >&AC_FD_CC
          else
            bnv_try_4="$CXX $LDFLAGS -o bnv_qt_main bnv_qt_main.o moc_bnv_qt_test.o $QT_LIBS $LIBS >/dev/null 2>bnv_qt_test_4.out"
            AC_TRY_EVAL(bnv_try_4)
            bnv_err_4=`grep -v '^ *+' bnv_qt_test_4.out`
            if test x"$bnv_err_4" != x; then
              echo "$bnv_err_4" >&AC_FD_CC
            else
              bnv_cv_qt_test_result="success"
            fi
          fi
        fi
      fi
    ])dnl AC_CACHE_VAL bnv_cv_qt_test_result
    AC_MSG_RESULT([$bnv_cv_qt_test_result]);
    if test x"$bnv_cv_qt_test_result" = "xfailure"; then
      AC_MSG_ERROR([Failed to find matching components of a complete
                  Qt installation. Try using more options,
                  see ./configure --help.])
    fi

    rm -f bnv_qt_test.h moc_bnv_qt_test.$ac_ext moc_bnv_qt_test.o \
          bnv_qt_main.$ac_ext bnv_qt_main.o bnv_qt_main \
          bnv_qt_test_1.out bnv_qt_test_2.out bnv_qt_test_3.out bnv_qt_test_4.out
  fi

    AC_LANG_RESTORE
])

dnl Internal subroutine of BNV_HAVE_QT
dnl Set bnv_qt_dir bnv_qt_include_dir bnv_qt_bin_dir bnv_qt_lib_dir
dnl Copyright 2001 Bastiaan N. Veelo <Bastiaan.N.Veelo@immtek.ntnu.no>
dnl Modified in 2018 by Zack Middleton <zturtleman>
AC_DEFUN([BNV_PATH_QT_DIRECT],
[
  if test x"$host_alias" != x; then
    # set by configure --host
    bnv_qt_host=$host_alias
  else
    bnv_qt_host=`$SHELL "$srcdir/config.guess" | cut -d'-' -f 1,3-4`
  fi

  ## Binary utilities ##
  if test x"$with_Qt_bin_dir" != x; then
    bnv_qt_bin_dir=$with_Qt_bin_dir
  fi
  ## Look for header files ##
  if test x"$with_Qt_include_dir" != x; then
    bnv_qt_include_dir="$with_Qt_include_dir"
  else
    # The following header file is expected to define QT_VERSION.
    # Look for the header file in a standard set of common directories.
    bnv_include_path_list="
      /usr/include/$bnv_qt_host/qt5
      /usr/include/qt5
      /usr/qt5/include
      /usr/local/include/$bnv_qt_host/qt5
      /usr/local/include/qt5
      /usr/local/qt5/include
      `ls -dr /usr/local/Cellar/qt/5*/include 2>/dev/null`
    "
    for bnv_dir in $bnv_include_path_list; do
      if test -r "$bnv_dir/QtCore/qconfig.h"; then
        bnv_dirs="$bnv_dirs $bnv_dir"
      fi
    done
    # Now look for the newest in this list
    bnv_prev_ver=0x04FFFF
    for bnv_dir in $bnv_dirs; do
      # Qt 5.7.0 and later use separate defines in qconfig.h
      if test -r "$bnv_dir/QtCore/qconfig.h"; then
        ztm_qt_ver_major=`egrep -w '#define QT_VERSION_MAJOR' $bnv_dir/QtCore/qconfig.h | sed s/'#define QT_VERSION_MAJOR'//`
        if test x"$ztm_qt_ver_major" != x; then
          ztm_qt_ver_minor=`egrep -w '#define QT_VERSION_MINOR' $bnv_dir/QtCore/qconfig.h | sed s/'#define QT_VERSION_MINOR'//`
          ztm_qt_ver_patch=`egrep -w '#define QT_VERSION_PATCH' $bnv_dir/QtCore/qconfig.h | sed s/'#define QT_VERSION_PATCH'//`
          bnv_this_ver=`printf "0x%02X%02X%02X" $ztm_qt_ver_major $ztm_qt_ver_minor $ztm_qt_ver_patch`
          if expr $bnv_this_ver '>' $bnv_prev_ver > /dev/null; then
            bnv_is_qt5=yes
            bnv_qt_include_dir=$bnv_dir
            bnv_prev_ver=$bnv_this_ver
          fi

          # don't check qglobal.h
          continue
        fi
      fi

      # Before Qt 5.7.0 the version was a constant (i.e., 0x050600 for 5.6.0) in qglobal.h
      if test -r "$bnv_dir/QtCore/qglobal.h"; then
         bnv_this_ver=`egrep -w '#define QT_VERSION' $bnv_dir/QtCore/qglobal.h | sed s/'#define QT_VERSION'//`
         if expr $bnv_this_ver '>' $bnv_prev_ver > /dev/null; then
           bnv_is_qt5=yes
           bnv_qt_include_dir=$bnv_dir
           bnv_prev_ver=$bnv_this_ver
         fi
      fi
    done
  fi dnl Found header files.

  # Are these headers located in a traditional Trolltech installation?
  # That would be $bnv_qt_include_dir stripped from its last element:
  bnv_found_traditional=no
  bnv_possible_qt_dir=`dirname $bnv_qt_include_dir`
  if test -x $bnv_possible_qt_dir/bin/moc; then
    if ls $bnv_possible_qt_dir/lib/libQt5Core.* 1> /dev/null 2> /dev/null; then
      bnv_found_traditional=yes
    elif test -r $bnv_possible_qt_dir/Frameworks/QtCore.framework; then
      bnv_found_traditional=yes
    fi
  fi
  if test x"$bnv_found_traditional" = xyes; then
    # Then the rest is a piece of cake
    bnv_qt_dir=$bnv_possible_qt_dir
    bnv_qt_bin_dir="$bnv_qt_dir/bin"
    if test -d "$bnv_qt_dir/lib64" ; then
      bnv_qt_lib_dir="$bnv_qt_dir/lib64"
    else
      bnv_qt_lib_dir="$bnv_qt_dir/lib"
    fi
    if test x"$is_osx" = xyes; then
      bnv_qt_LIBS="-F$bnv_qt_dir/Frameworks -L$bnv_qt_lib_dir $bnv_qt5_libs $QT_XLIBS"
    else
      bnv_qt_LIBS="-L$bnv_qt_lib_dir $bnv_qt5_libs $QT_XLIBS"
    fi
  fi
  if test $bnv_found_traditional = no; then
    # There is no valid definition for $QTDIR as Trolltech likes to see it
    bnv_qt_dir=
    ## Look for Qt library ##
    if test x"$with_Qt_lib_dir" != x; then
      bnv_qt_lib_dir="$with_Qt_lib_dir"
      bnv_qt_LIBS="-L$bnv_qt_lib_dir $bnv_qt5_libs $QT_XLIBS"
    else
      # Normally, when there is no traditional Trolltech installation,
      # the library is installed in a place where the linker finds it
      # automatically.
      qt_direct_test_header=QtWidgets/QApplication
      qt_direct_test_main="
        int argc;
        char ** argv;
        QApplication app(argc,argv);
      "
      # See if we find the library without any special options.
      # Don't add top $LIBS permanently yet
      bnv_save_LIBS="$LIBS"
      bnv_save_CXXFLAGS="$CXXFLAGS"
      CXXFLAGS="-I$bnv_qt_include_dir -fPIC"
      LIBS="$bnv_qt5_libs  $QT_XLIBS"
      bnv_qt_LIBS="$LIBS"
      AC_TRY_LINK([#include <$qt_direct_test_header>],
        $qt_direct_test_main,
      [
        # Succes.
        # We can link with no special library directory.
        bnv_qt_lib_dir=
      ], [
        # That did not work. Maybe a library version I don't know about?
        # Look for some Qt lib in a standard set of common directories.
        bnv_dir_list="
          `echo $bnv_qt_include_dir | sed "s|/include|/lib|"`
          /usr/lib/$bnv_qt_host
          /usr/lib/qt5
          /usr/qt5/lib
          /usr/lib64
          /usr/lib
          /usr/local/lib/$bnv_qt_host
          /usr/local/lib/qt5
          /usr/local/qt5/lib
          /usr/local/lib64
          /usr/local/lib
          /lib64
          /lib
          /opt/lib64
          /opt/lib
        "
        for bnv_dir in $bnv_dir_list; do
          if ls $bnv_dir/libQt5Core* > /dev/null 2> /dev/null ; then
            bnv_qt_lib_dir="$bnv_dir"
            break
          fi
        done
        # Try with that one
        LIBS="$bnv_qt5_libs  $QT_XLIBS"
      ])
      if test x"$bnv_qt_lib_dir" != x; then
        bnv_qt_LIBS="-L$bnv_qt_lib_dir $LIBS"
      else
        bnv_qt_LIBS="$LIBS"
      fi
      LIBS="$bnv_save_LIBS"
      CXXFLAGS="$bnv_save_CXXFLAGS"

    fi dnl $with_Qt_lib_dir was not given

  fi dnl Done setting up for non-traditional Trolltech installation

])

dnl for -lqglviewer (KSW was here)
AC_DEFUN([KSW_HAVE_QGL],
[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([BNV_HAVE_QT])

  AC_LANG_SAVE
  AC_LANG_CPLUSPLUS

  AC_MSG_CHECKING(Qt OpenGL)

    AC_CACHE_VAL(ksw_cv_qgl_test_result,
    [
      cat > ksw_qgl_test.${ac_ext} << EOF
#include <QtWidgets/QApplication>
#include <QtWidgets/QWidget>
#include <QtOpenGL/QGLWidget>

int main( int argc, char ** argv )
{
   QApplication app( argc, argv );
   QGLWidget w;
   w.show();
   return app.exec();
}
EOF

      ksw_cv_qgl_test_result="failure"
        ksw_try_1="$CXX $QT_CXXFLAGS $CXXFLAGS $GL_CFLAGS -Wno-uninitialized -o ksw_qgl_test ksw_qgl_test.${ac_ext} $QT_LIBS $GL_LIBS >/dev/null 2>ksw_qgl_test_1.out"
        AC_TRY_EVAL(ksw_try_1)
        ksw_err_1=`grep -v '^ *+' ksw_qgl_test_1.out | grep -v "^ksw_qgl_test.{$ac_ext}\$"`
        if test x"$ksw_err_1" != x; then
           ksw_try_2="$CXX $QT_CXXFLAGS $CXXFLAGS $GL_CFLAGS -Wno-uninitialized -o ksw_qgl_test ksw_qgl_test.${ac_ext} $QT_LIBS -lqglviewer $GL_LIBS  >/dev/null 2>ksw_qgl_test_2.out"
           AC_TRY_EVAL(ksw_try_2)
           ksw_err_2=`grep -v '^ *+' ksw_qgl_test_2.out | grep -v "^ksw_qgl_test.{$ac_ext}\$"`
           if test x"$ksw_err_2" != x; then
             echo "$ksw_err_2" >&AC_FD_CC
             echo "configure: could not compile:" >&AC_FD_CC
             cat ksw_qgl_test.${ac_ext} >&AC_FD_CC
           else
             ksw_cv_qgl_test_result="success"
             QGL_LIBS=-lqglviewer
           fi
        else
         ksw_cv_qgl_test_result="success"
         QGL_LIBS=
        fi
    ])dnl AC_CACHE_VAL ksw_cv_qgl_test_result
    AC_MSG_RESULT([$ksw_cv_qgl_test_result]);
    if test x"$ksw_cv_qgl_test_result" = "xfailure"; then
      AC_MSG_ERROR([Failed to link Qt with OpenGL support.])
    fi

    rm -f ksw_qgl_test.h \
          ksw_qgl_test.${ac_ext} ksw_qgl_test.o ksw_qgl_test \
          ksw_qgl_test_1.out ksw_qgl_test_2.out

    AC_SUBST(QGL_LIBS)

    AC_LANG_RESTORE
])

AC_DEFUN([MDL_HAVE_OPENGL],
[
  AC_REQUIRE([AC_PROG_CC])
  AC_REQUIRE([AC_PATH_X])
  AC_REQUIRE([AC_PATH_XTRA])

  AC_CACHE_CHECK([for OpenGL], mdl_cv_have_OpenGL,
  [
dnl Check for Mesa first, unless we were asked not to.
    AC_ARG_WITH([--with-Mesa],
                   [Prefer the Mesa library over a vendors native OpenGL library (default=yes)],
                   with_Mesa_help_string)
    AC_ARG_ENABLE(Mesa, $with_Mesa_help_string, use_Mesa=$enableval, use_Mesa=yes)

    if test x"$use_Mesa" = xyes; then
       GL_search_list="MesaGL   GL"
      GLU_search_list="MesaGLU GLU"
    else
       GL_search_list="GL  MesaGL"
      GLU_search_list="GLU MesaGLU"
    fi

    AC_LANG_SAVE
    AC_LANG_C

if test x"$is_osx" = xyes; then
   GL_CFLAGS="-framework OpenGL -framework AGL"
   GL_LIBS="-framework OpenGL -framework AGL"
else
dnl If we are running under X11 then add in the appropriate libraries.
   if test x"$no_x" != xyes; then
dnl Add everything we need to compile and link X programs to GL_X_CFLAGS
dnl and GL_X_LIBS.
      GL_CFLAGS="$X_CFLAGS"
      dnl Recent QT4 doesn't need these anymore. -- garden@acheronte.it
      dnl GL_X_LIBS="$X_PRE_LIBS $X_LIBS -lX11 -lXext -lXmu -lXt -lXi $X_EXTRA_LIBS"
      GL_X_LIBS=""
   fi
fi
    GL_save_CPPFLAGS="$CPPFLAGS"
    CPPFLAGS="$GL_CFLAGS"

    GL_save_LIBS="$LIBS"
    LIBS="$GL_X_LIBS"


    # Save the "AC_MSG_RESULT file descriptor" to FD 8.
    exec 8>&AC_FD_MSG

    # Temporarily turn off AC_MSG_RESULT so that the user gets pretty
    # messages.
    exec AC_FD_MSG>/dev/null

    AC_SEARCH_LIBS(glAccum,          $GL_search_list, have_GL=yes,   have_GL=no)
    AC_SEARCH_LIBS(gluBeginCurve,   $GLU_search_list, have_GLU=yes,  have_GLU=no)
    AC_SEARCH_LIBS(glutInit,        glut,             have_glut=yes, have_glut=no)



    # Restore pretty messages.
    exec AC_FD_MSG>&8

    if test -n "$LIBS"; then
      mdl_cv_have_OpenGL=yes
      GL_LIBS="$LIBS"
      AC_SUBST(GL_CFLAGS)
      AC_SUBST(GL_LIBS)
    else
      mdl_cv_have_OpenGL=no
      GL_CFLAGS=
    fi

dnl Reset GL_X_LIBS regardless, since it was just a temporary variable
dnl and we don't want to be global namespace polluters.
    GL_X_LIBS=

    LIBS="$GL_save_LIBS"
    CPPFLAGS="$GL_save_CPPFLAGS"

    AC_LANG_RESTORE
    
dnl bugfix: dont forget to cache this variables, too
    mdl_cv_GL_CFLAGS="$GL_CFLAGS"
    mdl_cv_GL_LIBS="$GL_LIBS"
    mdl_cv_have_GL="$have_GL"
    mdl_cv_have_GLU="$have_GLU"
    mdl_cv_have_glut="$have_glut"
  ])
  GL_CFLAGS="$mdl_cv_GL_CFLAGS"
  GL_LIBS="$mdl_cv_GL_LIBS"
  have_GL="$mdl_cv_have_GL"
  have_GLU="$mdl_cv_have_GLU"
  have_glut="$mdl_cv_have_glut"
])
dnl endof bugfix -ainan

AC_DEFUN([KSW_HAVE_DLOPEN],
[
  AC_REQUIRE([AC_PROG_CC])
  
  AC_CACHE_CHECK([for dlopen], ksw_cv_have_dlopen,
  [
    DLOPEN_save_LIBS="$LIBS"

    DLOPEN_LIBS="-ldl -rdynamic"
    LIBS="$DLOPEN_save_LIBS $DLOPEN_LIBS"
    cat > ksw_dlopen_test.c << EOF
#include <stdio.h>
#include <dlfcn.h>
int main( int argc, char ** argv )
{
   dlopen( "filename", 0 );
   return 0;
}
EOF
    if ${CC} -o ksw_dlopen_test ksw_dlopen_test.c $LIBS 2> /dev/null > /dev/null; then
       have_dlopen=yes
    else
        DLOPEN_LIBS="-ldl -Wl,-export-dynamic"
        LIBS="$DLOPEN_save_LIBS $DLOPEN_LIBS"
        if ${CC} -o ksw_dlopen_test ksw_dlopen_test.c $LIBS 2> /dev/null > /dev/null; then
           have_dlopen=yes
        else
           DLOPEN_LIBS="-ldl"
           LIBS="$DLOPEN_save_LIBS $DLOPEN_LIBS"
           if ${CC} -o ksw_dlopen_test ksw_dlopen_test.c $LIBS 2> /dev/null > /dev/null; then
              have_dlopen=yes
           else
              have_dlopen=no
              DLOPEN_LIBS=
           fi
        fi
    fi

    rm -f ksw_dlopen_test ksw_dlopen_test.c

    LIBS="$DLOPEN_save_LIBS"

    ksw_cv_DLOPEN_LIBS="$DLOPEN_LIBS"
    ksw_cv_have_dlopen="$have_dlopen"
  ])
  DLOPEN_LIBS="$ksw_cv_DLOPEN_LIBS"
  have_dlopen="$ksw_cv_have_dlopen"
  AC_SUBST(DLOPEN_LIBS)
  if test x"$have_dlopen" = "xyes"; then
    AC_DEFINE( [HAVE_DLOPEN], [], [Define when you have dlopen function] )
  fi
])


AC_DEFUN([VL_PROG_CC_WARNINGS], [
  ansi=$1
  if test -z "$ansi"; then
    msg="for C compiler warning flags"
  else
    msg="for C compiler warning and ANSI conformance flags"
  fi
  AC_CACHE_CHECK($msg, vl_cv_prog_cc_warnings, [
    if test -n "$CC"; then
      cat > conftest.c <<EOF
int main(int argc, char **argv) { return 0; }
EOF

      dnl GCC
      if test "$GCC" = "yes"; then
        if test -z "$ansi"; then
          vl_cv_prog_cc_warnings="-Wall"
        else
          vl_cv_prog_cc_warnings="-Wall -ansi -pedantic"
        fi

      dnl Most compilers print some kind of a version string with some command
      dnl line options (often "-V").  The version string should be checked
      dnl before doing a test compilation run with compiler-specific flags.
      dnl This is because some compilers (like the Cray compiler) only
      dnl produce a warning message for unknown flags instead of returning
      dnl an error, resulting in a false positive.  Also, compilers may do
      dnl erratic things when invoked with flags meant for a different
      dnl compiler.

      dnl Solaris C compiler
      elif $CC -V 2>&1 | grep -i "WorkShop" > /dev/null 2>&1 &&
           $CC -c -v -Xc conftest.c > /dev/null 2>&1 &&
           test -f conftest.o; then
        if test -z "$ansi"; then
          vl_cv_prog_cc_warnings="-v"
        else
          vl_cv_prog_cc_warnings="-v -Xc"
        fi

      dnl Digital Unix C compiler
      elif $CC -V 2>&1 | grep -i "Digital UNIX Compiler" > /dev/null 2>&1 &&
           $CC -c -verbose -w0 -warnprotos -std1 conftest.c > /dev/null 2>&1 &&
           test -f conftest.o; then
        if test -z "$ansi"; then
          vl_cv_prog_cc_warnings="-verbose -w0 -warnprotos"
        else
          vl_cv_prog_cc_warnings="-verbose -w0 -warnprotos -std1"
        fi

      dnl C for AIX Compiler
      elif $CC 2>&1 | grep -i "C for AIX Compiler" > /dev/null 2>&1 &&
           $CC -c -qlanglvl=ansi -qinfo=all conftest.c > /dev/null 2>&1 &&
           test -f conftest.o; then
        if test -z "$ansi"; then
          vl_cv_prog_cc_warnings="-qsrcmsg -qinfo=all:noppt:noppc:noobs:nocnd"
        else
          vl_cv_prog_cc_warnings="-qsrcmsg -qinfo=all:noppt:noppc:noobs:nocnd -qlanglvl=ansi"
        fi

      dnl IRIX C compiler
      elif $CC -version 2>&1 | grep -i "MIPSpro Compilers" > /dev/null 2>&1 &&
           $CC -c -fullwarn -ansi -ansiE conftest.c > /dev/null 2>&1 &&
           test -f conftest.o; then
        if test -z "$ansi"; then
          vl_cv_prog_cc_warnings="-fullwarn"
        else
          vl_cv_prog_cc_warnings="-fullwarn -ansi -ansiE"
        fi

      dnl HP-UX C compiler
      elif what $CC 2>&1 | grep -i "HP C Compiler" > /dev/null 2>&1 &&
           $CC -c -Aa +w1 conftest.c > /dev/null 2>&1 &&
           test -f conftest.o; then
        if test -z "$ansi"; then
          vl_cv_prog_cc_warnings="+w1"
        else
          vl_cv_prog_cc_warnings="+w1 -Aa"
        fi

      dnl The NEC SX-5 (Super-UX 10) C compiler
      elif $CC -V 2>&1 | grep "/SX" > /dev/null 2>&1 &&
           $CC -c -pvctl[,]fullmsg -Xc conftest.c > /dev/null 2>&1 &&
           test -f conftest.o; then
        if test -z "$ansi"; then
          vl_cv_prog_cc_warnings="-pvctl[,]fullmsg"
        else
          vl_cv_prog_cc_warnings="-pvctl[,]fullmsg -Xc"
        fi

      dnl The Cray C compiler (Unicos)
      elif $CC -V 2>&1 | grep -i "Cray" > /dev/null 2>&1 &&
           $CC -c -h msglevel 2 conftest.c > /dev/null 2>&1 &&
           test -f conftest.o; then
        if test -z "$ansi"; then
          vl_cv_prog_cc_warnings="-h msglevel 2"
        else
          vl_cv_prog_cc_warnings="-h msglevel 2 -h conform"
        fi

      fi
      rm -f conftest.*
    fi
    if test -n "$vl_cv_prog_cc_warnings"; then
      CFLAGS="$CFLAGS $vl_cv_prog_cc_warnings"
      CXXFLAGS="$CXXFLAGS $vl_cv_prog_cc_warnings"
    else
      vl_cv_prog_cc_warnings="unknown"
    fi
  ])
])dnl

AC_DEFUN([AC_C_BIGENDIAN_CROSS],
[AC_CACHE_CHECK(whether byte ordering is bigendian, ac_cv_c_bigendian,
[ac_cv_c_bigendian=unknown
# See if sys/param.h defines the BYTE_ORDER macro.
AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/param.h>], [
#if !BYTE_ORDER || !BIG_ENDIAN || !LITTLE_ENDIAN
 bogus endian macros
#endif], [# It does; now see whether it defined to BIG_ENDIAN or not.
AC_TRY_COMPILE([#include <sys/types.h>
#include <sys/param.h>], [
#if BYTE_ORDER != BIG_ENDIAN
 not big endian
#endif], ac_cv_c_bigendian=yes, ac_cv_c_bigendian=no)])
if test $ac_cv_c_bigendian = unknown; then
AC_TRY_RUN([main () {
  /* Are we little or big endian?  From Harbison&Steele.  */
  union
  {
    long l;
    char c[sizeof (long)];
  } u;
  u.l = 1;
  exit (u.c[sizeof (long) - 1] == 1);
}], ac_cv_c_bigendian=no, ac_cv_c_bigendian=yes,
[ echo $ac_n "cross-compiling... " 2>&AC_FD_MSG ])
fi])
if test $ac_cv_c_bigendian = unknown; then
AC_MSG_CHECKING(to probe for byte ordering)
[
cat >conftest.c <<EOF
short ascii_mm[] = { 0x4249, 0x4765, 0x6E44, 0x6961, 0x6E53, 0x7953, 0 };
short ascii_ii[] = { 0x694C, 0x5454, 0x656C, 0x6E45, 0x6944, 0x6E61, 0 };
void _ascii() { char* s = (char*) ascii_mm; s = (char*) ascii_ii; }
short ebcdic_ii[] = { 0x89D3, 0xE3E3, 0x8593, 0x95C5, 0x89C4, 0x9581, 0 };
short ebcdic_mm[] = { 0xC2C9, 0xC785, 0x95C4, 0x8981, 0x95E2, 0xA8E2, 0 };
void _ebcdic() { char* s = (char*) ebcdic_mm; s = (char*) ebcdic_ii; }
int main() { _ascii (); _ebcdic (); return 0; }
EOF
] if test -f conftest.c ; then
     if ${CC-cc} -c conftest.c -o conftest.o && test -f conftest.o ; then
        if test `grep -l BIGenDianSyS conftest.o` ; then
           echo $ac_n ' big endian probe OK, ' 1>&AC_FD_MSG
           ac_cv_c_bigendian=yes
        fi
        if test `grep -l LiTTleEnDian conftest.o` ; then
           echo $ac_n ' little endian probe OK, ' 1>&AC_FD_MSG
           if test $ac_cv_c_bigendian = yes ; then
            ac_cv_c_bigendian=unknown;
           else
            ac_cv_c_bigendian=no
           fi
        fi
        echo $ac_n 'guessing bigendian ...  ' >&AC_FD_MSG
     fi
  fi
AC_MSG_RESULT($ac_cv_c_bigendian)
fi
if test $ac_cv_c_bigendian = yes; then
  AC_DEFINE(WORDS_BIGENDIAN, 1, [whether byteorder is bigendian])
  BYTEORDER=4321
else
  BYTEORDER=1234
fi
AC_DEFINE_UNQUOTED(BYTEORDER, $BYTEORDER, [1234 = LIL_ENDIAN, 4321 = BIGENDIAN])
if test $ac_cv_c_bigendian = unknown; then
  AC_MSG_ERROR(unknown endianess - sorry, please pre-set ac_cv_c_bigendian)
fi
])


AC_DEFUN([KSW_HAVE_GETTIMEOFDAY],
[
  AC_REQUIRE([AC_PROG_CC])

  AC_CACHE_CHECK([for gettimeofday], ksw_cv_have_gettimeofday,
  [
    cat > ksw_have_gettimeofday_test.c << EOF
#include <stdio.h>
#include <sys/time.h>

int main( int argc, char * argv[] )
{
   struct timeval tv;
   gettimeofday( &tv, NULL );
   return 0;
}
EOF
    if ${CC} -c ksw_have_gettimeofday_test.c 2> /dev/null > /dev/null; then
       have_gettimeofday=yes
    else
       have_gettimeofday=no
    fi

   rm -f ksw_have_gettimeofday*

    ksw_cv_have_gettimeofday="$have_gettimeofday"
  ])
  have_gettimeofday="$ksw_cv_have_gettimeofday"
  if test x"$have_gettimeofday" = "xyes"; then
    AC_DEFINE( [HAVE_GETTIMEOFDAY], [], [Define when you have gettimeofday function] )
  fi
])


AC_DEFUN([KSW_IS_OSX],
[
  AC_REQUIRE([AC_PROG_CC])

  AC_CACHE_CHECK([for OS X], ksw_cv_is_osx,
  [
    if test -e /System/Library/Frameworks/Carbon.framework; then
       is_osx=yes
    else
       is_osx=no
    fi

   rm -f ksw_is_osx*

    ksw_cv_is_osx="$is_osx"
  ])
  is_osx="$ksw_cv_is_osx"
  if test x"$is_osx" = "xyes"; then
    AC_DEFINE( [IS_OSX], [], [Define when you run on OSX] )
  fi
])

AC_DEFUN([ZTM_WITH_MACOSX_VERSION_MIN],
[
  AC_REQUIRE([AC_PROG_CC])

  AC_ARG_WITH([macosx-version-min],
    [  --with-macosx-version-min=VERSION
                          Minimum Mac OS X deployment VERISON (i.e., 10.10),
                          It should not be lower than Qt's macosx-version-min.])

  if test x"$with_macosx_version_min" != x; then
    ztm_macosx_major=`echo $with_macosx_version_min | cut -d. -f1`
    ztm_macosx_minor=`echo $with_macosx_version_min | cut -d. -f2`
    if test $ztm_macosx_minor -gt 9; then
      # Do math and then remove decimal. 10.10 -> 101000.0 -> 101000
      MAC_OS_X_VERSION_MIN_REQUIRED=`echo "$ztm_macosx_major * 10000 + $ztm_macosx_minor * 100" | bc` # | cut -d. -f1
    else
      # Multiply by 100 and then remove decimal. 10.7 -> 1070.0 -> 1070
      MAC_OS_X_VERSION_MIN_REQUIRED=`echo "$with_macosx_version_min * 100" | bc` # | cut -d. -f1
    fi

    LDFLAGS="$LDFLAGS -mmacosx-version-min=$with_macosx_version_min"
    CFLAGS="$CFLAGS -mmacosx-version-min=$with_macosx_version_min -DMAC_OS_X_VERSION_MIN_REQUIRED=$MAC_OS_X_VERSION_MIN_REQUIRED"
    CXXFLAGS="$CXXFLAGS -mmacosx-version-min=$with_macosx_version_min -DMAC_OS_X_VERSION_MIN_REQUIRED=$MAC_OS_X_VERSION_MIN_REQUIRED"

    MACOSX_DEPLOYMENT_TARGET=$with_macosx_version_min
    AC_SUBST(MACOSX_DEPLOYMENT_TARGET)
  fi
])


