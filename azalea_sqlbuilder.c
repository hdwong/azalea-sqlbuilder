/*
  +----------------------------------------------------------------------+
  | PHP Version 7                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2016 The PHP Group                                |
  +----------------------------------------------------------------------+
  | This source file is subject to version 3.01 of the PHP license,      |
  | that is bundled with this package in the file LICENSE, and is        |
  | available through the world-wide-web at the following url:           |
  | http://www.php.net/license/3_01.txt                                  |
  | If you did not receive a copy of the PHP license and are unable to   |
  | obtain it through the world-wide-web, please send a note to          |
  | license@php.net so we can mail you a copy immediately.               |
  +----------------------------------------------------------------------+
  | Author:                                                              |
  +----------------------------------------------------------------------+
*/

/* $Id$ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_azalea_sqlbuilder.h"

#include "azalea/sqlbuilder.h"

/* True global resources - no need for thread safety here */
static int le_azalea_sqlbuilder;

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(azalea_sqlbuilder)
{
	ZEND_MODULE_STARTUP_N(main)(INIT_FUNC_ARGS_PASSTHRU);

	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(azalea_sqlbuilder)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(azalea_sqlbuilder)
{
#if defined(COMPILE_DL_AZALEA_SQLBUILDER) && defined(ZTS)
	ZEND_TSRMLS_CACHE_UPDATE();
#endif
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(azalea_sqlbuilder)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(azalea_sqlbuilder)
{
	php_info_print_table_start();
	php_info_print_table_row(2, "Version", PHP_AZALEA_SQLBUILDER_VERSION);
	php_info_print_table_end();
}
/* }}} */

/* {{{ azalea_sqlbuilder_module_entry
 */
zend_module_entry azalea_sqlbuilder_module_entry = {
	STANDARD_MODULE_HEADER,
	"azalea_sqlbuilder",
	NULL,
	PHP_MINIT(azalea_sqlbuilder),
	PHP_MSHUTDOWN(azalea_sqlbuilder),
	PHP_RINIT(azalea_sqlbuilder),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(azalea_sqlbuilder),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(azalea_sqlbuilder),
	PHP_AZALEA_SQLBUILDER_VERSION,
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_AZALEA_SQLBUILDER
#ifdef ZTS
ZEND_TSRMLS_CACHE_DEFINE()
#endif
ZEND_GET_MODULE(azalea_sqlbuilder)
#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
