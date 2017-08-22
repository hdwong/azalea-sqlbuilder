dnl $Id$
dnl config.m4 for extension azalea_sqlbuilder

dnl Comments in this file start with the string 'dnl'.
dnl Remove where necessary. This file will not work
dnl without editing.

dnl If your extension references something external, use with:

dnl PHP_ARG_WITH(azalea_sqlbuilder, for azalea_sqlbuilder support,
dnl Make sure that the comment is aligned:
dnl [  --with-azalea_sqlbuilder             Include azalea_sqlbuilder support])

dnl Otherwise use enable:

dnl PHP_ARG_ENABLE(azalea_sqlbuilder, whether to enable azalea_sqlbuilder support,
dnl Make sure that the comment is aligned:
dnl [  --enable-azalea_sqlbuilder           Enable azalea_sqlbuilder support])

if test "$PHP_AZALEA_SQLBUILDER" != "no"; then
  dnl Write more examples of tests here...

  dnl # --with-azalea_sqlbuilder -> check with-path
  dnl SEARCH_PATH="/usr/local /usr"     # you might want to change this
  dnl SEARCH_FOR="/include/azalea_sqlbuilder.h"  # you most likely want to change this
  dnl if test -r $PHP_AZALEA_SQLBUILDER/$SEARCH_FOR; then # path given as parameter
  dnl   AZALEA_SQLBUILDER_DIR=$PHP_AZALEA_SQLBUILDER
  dnl else # search default path list
  dnl   AC_MSG_CHECKING([for azalea_sqlbuilder files in default path])
  dnl   for i in $SEARCH_PATH ; do
  dnl     if test -r $i/$SEARCH_FOR; then
  dnl       AZALEA_SQLBUILDER_DIR=$i
  dnl       AC_MSG_RESULT(found in $i)
  dnl     fi
  dnl   done
  dnl fi
  dnl
  dnl if test -z "$AZALEA_SQLBUILDER_DIR"; then
  dnl   AC_MSG_RESULT([not found])
  dnl   AC_MSG_ERROR([Please reinstall the azalea_sqlbuilder distribution])
  dnl fi

  dnl # --with-azalea_sqlbuilder -> add include path
  dnl PHP_ADD_INCLUDE($AZALEA_SQLBUILDER_DIR/include)

  dnl # --with-azalea_sqlbuilder -> check for lib and symbol presence
  dnl LIBNAME=azalea_sqlbuilder # you may want to change this
  dnl LIBSYMBOL=azalea_sqlbuilder # you most likely want to change this 

  dnl PHP_CHECK_LIBRARY($LIBNAME,$LIBSYMBOL,
  dnl [
  dnl   PHP_ADD_LIBRARY_WITH_PATH($LIBNAME, $AZALEA_SQLBUILDER_DIR/$PHP_LIBDIR, AZALEA_SQLBUILDER_SHARED_LIBADD)
  dnl   AC_DEFINE(HAVE_AZALEA_SQLBUILDERLIB,1,[ ])
  dnl ],[
  dnl   AC_MSG_ERROR([wrong azalea_sqlbuilder lib version or lib not found])
  dnl ],[
  dnl   -L$AZALEA_SQLBUILDER_DIR/$PHP_LIBDIR -lm
  dnl ])
  dnl
  dnl PHP_SUBST(AZALEA_SQLBUILDER_SHARED_LIBADD)

  PHP_NEW_EXTENSION(azalea_sqlbuilder, azalea_sqlbuilder.c, $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
