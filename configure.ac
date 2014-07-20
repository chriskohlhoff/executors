AC_INIT(executors, [0.0.1])
AC_CONFIG_SRCDIR(include/experimental/type_traits)
AM_MAINTAINER_MODE
AM_INIT_AUTOMAKE([tar-ustar])

AC_CANONICAL_HOST
AM_PROG_CC_C_O
AC_PROG_CXX
AC_LANG(C++)
AC_PROG_RANLIB

AC_ARG_WITH(boost,
  AC_HELP_STRING([--with-boost=DIR],[location of boost distribution]),
[
  if test "${withval}" = no; then
    CPPFLAGS="$CPPFLAGS -DEXECUTORS_NO_BOOST=1"
  else
    CPPFLAGS="$CPPFLAGS -I${withval}"
    LIBS="$LIBS -L${withval}/stage/lib -lboost_coroutine -lboost_thread -lboost_context -lboost_system"
  fi
],
[
  CPPFLAGS="$CPPFLAGS -DEXECUTORS_NO_BOOST=1"
])

WINDOWS=no
case $host in
  *-*-linux*)
    CXXFLAGS="$CXXFLAGS -pthread"
    LDFLAGS="$LDFLAGS -pthread"
    ;;
  *-*-solaris*)
    if test "$GXX" = yes; then
      CXXFLAGS="$CXXFLAGS -D_PTHREADS"
    else
      # We'll assume Sun's CC.
      CXXFLAGS="$CXXFLAGS -mt"
    fi
    LIBS="$LIBS -lpthread"
    ;;
  *-*-mingw32*)
    CXXFLAGS="$CXXFLAGS -mthreads"
    LDFLAGS="$LDFLAGS -mthreads"
    ;;
  *-pc-cygwin*)
    ;;
  *-apple-darwin*)
    ;;
  *-*-freebsd*)
    CXXFLAGS="$CXXFLAGS -pthread"
    LDFLAGS="$LDFLAGS -pthread"
    ;;
  *-*-netbsd*)
    CXXFLAGS="$CXXFLAGS -pthread"
    LDFLAGS="$LDFLAGS -pthread"
    ;;
esac

if test "$GXX" = yes; then
  CPPFLAGS="-std=c++1y -Wno-unused-local-typedefs $CPPFLAGS"
fi

AC_OUTPUT([
  Makefile
  include/Makefile
  src/Makefile
  src/examples/Makefile
  src/examples/channel/Makefile
  src/examples/executor/Makefile
  src/examples/timer/Makefile
  src/tests/Makefile
  src/tests/execution_context/Makefile
  src/tests/executor/Makefile
  src/tests/loop_scheduler/Makefile
  src/tests/no_executor/Makefile
  src/tests/performance/Makefile
  src/tests/strand/Makefile
  src/tests/system_executor/Makefile
  src/tests/thread_pool/Makefile
  src/tests/timer/Makefile
  src/tests/traits/Makefile])
