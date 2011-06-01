/*
  +----------------------------------------------------------------------+
  | PHP Version 5                                                        |
  +----------------------------------------------------------------------+
  | Copyright (c) 1997-2008 The PHP Group                                |
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

/* $Id: header 252479 2008-02-07 19:39:50Z iliaa $ */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
#include "php_iptables.h"

/* Include netfilter files */
#include <netinet/ip.h>
#include <libiptc/libiptc.h>

/* If you declare any globals in php_iptables.h uncomment this:
ZEND_DECLARE_MODULE_GLOBALS(iptables)
*/

/* True global resources - no need for thread safety here */
static int le_iptables;

/* {{{ iptables_functions[]
 *
 * Every user visible function must have an entry in iptables_functions[].
 */
const zend_function_entry iptables_functions[] = {
	PHP_FE(ipt_get_policy, NULL)
	PHP_FE(ipt_set_policy, NULL)
	PHP_FE(suck_my_balls, NULL)
	PHP_FE(confirm_iptables_compiled,	NULL)		/* For testing, remove later. */
	{NULL, NULL, NULL}	/* Must be the last line in iptables_functions[] */
};
/* }}} */

/* {{{ iptables_module_entry
 */
zend_module_entry iptables_module_entry = {
#if ZEND_MODULE_API_NO >= 20010901
	STANDARD_MODULE_HEADER,
#endif
	"iptables",
	iptables_functions,
	PHP_MINIT(iptables),
	PHP_MSHUTDOWN(iptables),
	PHP_RINIT(iptables),		/* Replace with NULL if there's nothing to do at request start */
	PHP_RSHUTDOWN(iptables),	/* Replace with NULL if there's nothing to do at request end */
	PHP_MINFO(iptables),
#if ZEND_MODULE_API_NO >= 20010901
	"0.1", /* Replace with version number for your extension */
#endif
	STANDARD_MODULE_PROPERTIES
};
/* }}} */

#ifdef COMPILE_DL_IPTABLES
ZEND_GET_MODULE(iptables)
#endif

/* {{{ PHP_INI
 */
/* Remove comments and fill if you need to have entries in php.ini
PHP_INI_BEGIN()
    STD_PHP_INI_ENTRY("iptables.global_value",      "42", PHP_INI_ALL, OnUpdateLong, global_value, zend_iptables_globals, iptables_globals)
    STD_PHP_INI_ENTRY("iptables.global_string", "foobar", PHP_INI_ALL, OnUpdateString, global_string, zend_iptables_globals, iptables_globals)
PHP_INI_END()
*/
/* }}} */

/* {{{ php_iptables_init_globals
 */
/* Uncomment this function if you have INI entries
static void php_iptables_init_globals(zend_iptables_globals *iptables_globals)
{
	iptables_globals->global_value = 0;
	iptables_globals->global_string = NULL;
}
*/
/* }}} */

/* {{{ PHP_MINIT_FUNCTION
 */
PHP_MINIT_FUNCTION(iptables)
{
	/* If you have INI entries, uncomment these lines 
	REGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MSHUTDOWN_FUNCTION
 */
PHP_MSHUTDOWN_FUNCTION(iptables)
{
	/* uncomment this line if you have INI entries
	UNREGISTER_INI_ENTRIES();
	*/
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request start */
/* {{{ PHP_RINIT_FUNCTION
 */
PHP_RINIT_FUNCTION(iptables)
{
	return SUCCESS;
}
/* }}} */

/* Remove if there's nothing to do at request end */
/* {{{ PHP_RSHUTDOWN_FUNCTION
 */
PHP_RSHUTDOWN_FUNCTION(iptables)
{
	return SUCCESS;
}
/* }}} */

/* {{{ PHP_MINFO_FUNCTION
 */
PHP_MINFO_FUNCTION(iptables)
{
	php_info_print_table_start();
	php_info_print_table_header(2, "iptables support", "enabled");
	php_info_print_table_end();

	/* Remove comments if you have entries in php.ini
	DISPLAY_INI_ENTRIES();
	*/
}
/* }}} */

PHP_FUNCTION(ipt_get_policy)
{
	struct ipt_counters counters;
	struct iptc_handle *handle = NULL;
	const char *pol;
	char *name;
	int name_len;
	
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
							  &name, &name_len) == FAILURE) {
		//		php_error_docref(NULL TSRMLS_CC, E_WARNING, "expects a chain name");
		return;
	}
	const char *table = "filter";
	//	php_printf("table: %s\n", table);
	handle = iptc_init(table);
	if (handle == NULL) { // when not root, it happens when segfaults later at get_policy()
		php_printf("handle is null, exiting");
		return;
	}
	//	php_printf("checking policy for chain %s\n", name);
	pol = iptc_get_policy(name, &counters, handle);
	//	php_printf("policy= %s\n", pol);
	RETURN_STRING(pol, 1);
}

PHP_FUNCTION(ipt_set_policy) 
{
	struct ipt_counters *new_counters = NULL;
	struct ipt_counters counters;
	struct iptc_handle *handle = NULL;
	char *chain;
	int chain_len;
	char *policy;
	int policy_len;
	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "ss", 
							  &chain, &chain_len, &policy, &policy_len) == FAILURE) {
		return;
	}
	const char *table = "filter";
	handle = iptc_init(table);
	if (handle == NULL) {
		php_error_docref(NULL TSRMLS_CC, E_WARNING, "needs to run as root");
		return;
	}
	iptc_get_policy(chain, &counters, handle);
	// doesnt work:
	iptc_set_policy(chain, policy, new_counters, handle);
	RETURN_TRUE;
}

PHP_FUNCTION(suck_my_balls)
{
	char *name;
	int name_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s",
							  &name, &name_len) == FAILURE) {
		RETURN_FALSE;
	}
	php_printf("suck my balls %s!\n", name);
	RETURN_TRUE;
}


/* Remove the following function when you have succesfully modified config.m4
   so that your module can be compiled into PHP, it exists only for testing
   purposes. */

/* Every user-visible function in PHP should document itself in the source */
/* {{{ proto string confirm_iptables_compiled(string arg)
   Return a string to confirm that the module is compiled in */
PHP_FUNCTION(confirm_iptables_compiled)
{
	char *arg = NULL;
	int arg_len, len;
	char *strg;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &arg, &arg_len) == FAILURE) {
		return;
	}

	len = spprintf(&strg, 0, "Congratulations! You have successfully modified ext/%.78s/config.m4. Module %.78s is now compiled into PHP.", "iptables", arg);
	RETURN_STRINGL(strg, len, 0);
}
/* }}} */
/* The previous line is meant for vim and emacs, so it can correctly fold and 
   unfold functions in source code. See the corresponding marks just before 
   function definition, where the functions purpose is also documented. Please 
   follow this convention for the convenience of others editing your code.
*/


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: noet sw=4 ts=4 fdm=marker
 * vim<600: noet sw=4 ts=4
 */
