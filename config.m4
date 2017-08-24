dnl $Id$
dnl config.m4 for extension azalea_sqlbuilder

PHP_ARG_ENABLE(azalea_sqlbuilder, whether to enable azalea_sqlbuilder support,
[  --enable-azalea_sqlbuilder           Enable azalea_sqlbuilder support], "yes")

if test "$PHP_AZALEA_SQLBUILDER" != "no"; then
  PHP_NEW_EXTENSION(azalea_sqlbuilder, \
    azalea_sqlbuilder.c \
    azalea/sqlbuilder.c \
  , $ext_shared,, -DZEND_ENABLE_STATIC_TSRMLS_CACHE=1)
fi
