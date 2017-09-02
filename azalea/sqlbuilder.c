/*
 * azalea/sqlbuilder.c
 *
 * Created by Bun Wong on 17-8-23.
 */

#include "php.h"
#include "php_azalea_sqlbuilder.h"
#include "azalea/namespace.h"
#include "azalea/sqlbuilder.h"

#include "ext/standard/php_var.h"
#include "ext/standard/php_string.h"  // for php_trim, php_implode
#include "Zend/zend_smart_str.h"  // for smart_str_*

zend_class_entry *sqlBuilderCe;
zend_class_entry *sqlBuilderQueryInterfaceCe;

static zend_string * sqlBuilderGetWherePrefix(zend_bool whereGroupPrefix, zval *pWhere, const char *pType);

ZEND_BEGIN_ARG_INFO_EX(arginfo_sqlbuilder_construct, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, queryInstance, Azalea\\SqlBuilderQueryInterface, 1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(arginfo_sqlbuilderqueryinterface_query, 0, 0, 1)
	ZEND_ARG_INFO(0, sql)
ZEND_END_ARG_INFO()

/* {{{ class Azalea\SqlBuilder methods */
static zend_function_entry azalea_sqlbuilder_methods[] = {
	PHP_ME(azalea_sqlbuilder, __construct, arginfo_sqlbuilder_construct, ZEND_ACC_CTOR|ZEND_ACC_FINAL|ZEND_ACC_PUBLIC)
	ZEND_FENTRY(__toString, ZEND_MN(azalea_sqlbuilder_getSql), NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, where, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, orWhere, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, having, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, orHaving, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, whereGroupStart, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, orWhereGroupStart, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, notWhereGroupStart, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, orNotWhereGroupStart, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, whereGroupEnd, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, select, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, count, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, distinct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, from, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, join, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, limit, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, limitPage, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, orderBy, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, groupBy, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, insert, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, replace, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, update, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, delete, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, getSql, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(azalea_sqlbuilder, query, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ interface Azalea\SqlBuilderQueryInterface methods */
static zend_function_entry azalea_sqlbuilderqueryinterface_methods[] = {
	PHP_ABSTRACT_ME(azalea_sqlbuilderqueryinterface, query, arginfo_sqlbuilderqueryinterface_query)
	{NULL, NULL, NULL}
};
/* }}} */

/* {{{ module startup */
ZEND_MODULE_STARTUP_D(main)
{
	zend_class_entry ce1, ce2;

	// Azalea\SqlBuilder
	INIT_CLASS_ENTRY(ce1, AZALEA_NS_NAME(SqlBuilder), azalea_sqlbuilder_methods);
	sqlBuilderCe = zend_register_internal_class(&ce1);
	sqlBuilderCe->ce_flags |= ZEND_ACC_FINAL;
	zend_declare_property_null(sqlBuilderCe, ZEND_STRL("_queryInstance"), ZEND_ACC_PRIVATE);
	zend_declare_property_long(sqlBuilderCe, ZEND_STRL("_type"), SQLTYPE_SELECT, ZEND_ACC_PRIVATE);
	zend_declare_property_null(sqlBuilderCe, ZEND_STRL("_where"), ZEND_ACC_PRIVATE);
	zend_declare_property_bool(sqlBuilderCe, ZEND_STRL("_whereGroupPrefix"), 0, ZEND_ACC_PRIVATE);
	zend_declare_property_long(sqlBuilderCe, ZEND_STRL("_whereGroupDepth"), 0, ZEND_ACC_PRIVATE);
	zend_declare_property_null(sqlBuilderCe, ZEND_STRL("_select"), ZEND_ACC_PRIVATE);
	zend_declare_property_bool(sqlBuilderCe, ZEND_STRL("_distinct"), 0, ZEND_ACC_PRIVATE);
	zend_declare_property_null(sqlBuilderCe, ZEND_STRL("_from"), ZEND_ACC_PRIVATE);
	zend_declare_property_null(sqlBuilderCe, ZEND_STRL("_join"), ZEND_ACC_PRIVATE);
	zend_declare_property_null(sqlBuilderCe, ZEND_STRL("_by"), ZEND_ACC_PRIVATE);
	zend_declare_property_null(sqlBuilderCe, ZEND_STRL("_limit"), ZEND_ACC_PRIVATE);
	zend_declare_property_long(sqlBuilderCe, ZEND_STRL("_offset"), 0, ZEND_ACC_PRIVATE);
	zend_declare_property_null(sqlBuilderCe, ZEND_STRL("_set"), ZEND_ACC_PRIVATE);
	zend_declare_property_bool(sqlBuilderCe, ZEND_STRL("_ignore"), 0, ZEND_ACC_PRIVATE);
	zend_declare_property_bool(sqlBuilderCe, ZEND_STRL("_duplicateKeyUpdate"), 0, ZEND_ACC_PRIVATE);
	zend_declare_property_null(sqlBuilderCe, ZEND_STRL("_excludeFields"), ZEND_ACC_PRIVATE);

	// Azalea\SqlBuilderQueryInterface
	INIT_CLASS_ENTRY(ce2, AZALEA_NS_NAME(SqlBuilderQueryInterface), azalea_sqlbuilderqueryinterface_methods);
	sqlBuilderQueryInterfaceCe = zend_register_internal_interface(&ce2);

	return SUCCESS;
}
/* }}} */

/* {{{ proto sqlBuilderReset */
static void sqlBuilderReset(zval *this)
{
	zval *pValue;
	pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_where"), 1, NULL);
	zval_ptr_dtor(pValue);
	array_init(pValue);
	zend_update_property_bool(sqlBuilderCe, this, ZEND_STRL("_whereGroupPrefix"), 0);
	zend_update_property_long(sqlBuilderCe, this, ZEND_STRL("_whereGroupDepth"), 0);
	pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_select"), 1, NULL);
	zval_ptr_dtor(pValue);
	array_init(pValue);
	zend_update_property_bool(sqlBuilderCe, this, ZEND_STRL("_distinct"), 0);
	pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_from"), 1, NULL);
	zval_ptr_dtor(pValue);
	array_init(pValue);
	pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_join"), 1, NULL);
	zval_ptr_dtor(pValue);
	array_init(pValue);
	pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_by"), 1, NULL);
	zval_ptr_dtor(pValue);
	array_init(pValue);
	zend_update_property_null(sqlBuilderCe, this, ZEND_STRL("_limit"));
	zend_update_property_long(sqlBuilderCe, this, ZEND_STRL("_offset"), 0);
	pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_set"), 1, NULL);
	zval_ptr_dtor(pValue);
	array_init(pValue);
	zend_update_property_bool(sqlBuilderCe, this, ZEND_STRL("_ignore"), 0);
	zend_update_property_bool(sqlBuilderCe, this, ZEND_STRL("_duplicateKeyUpdate"), 0);
	pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_excludeFields"), 1, NULL);
	zval_ptr_dtor(pValue);
	array_init(pValue);
	zend_update_property_long(sqlBuilderCe, this, ZEND_STRL("_type"), SQLTYPE_SELECT);
}
/* }}} */

/* {{{ proto sqlBuilderEscapeStr */
zend_string * sqlBuilderEscapeStr(zend_string *val)
{
	zend_string *ret;
	char *result, *pResult, *p = ZSTR_VAL(val);
	size_t len = 0;

	result = ecalloc(sizeof(char), ZSTR_LEN(val) * 2);
	pResult = result;
	while (*p) {
		if (*p == '\\' || *p == '"' || *p == '\'') {
			*pResult++ = '\\';
			*pResult++ = *p;
			len += 2;
		} else if (*p == '\0') {
			*pResult++ = '\\';
			*pResult++ = '0';
			len += 2;
		} else if (*p == '\r') {
			*pResult++ = '\\';
			*pResult++ = 'r';
			len += 2;
		} else if (*p == '\n') {
			*pResult++ = '\\';
			*pResult++ = 'n';
			len += 2;
		} else {
			*pResult++ = *p;
			++len;
		}
		++p;
	}
	if (len == ZSTR_LEN(val)) {
		ret = zend_string_copy(val);
	} else {
		ret = zend_string_init(result, len, 0);
	}
	efree(result);
	return ret;
}
/* }}} */

/* {{{ proto sqlBuilderEscapeEx */
void sqlBuilderEscapeEx(zval *return_value, zval *val, zend_bool escapeValue)
{
	zend_ulong h;
	zend_string *key;
	zval *pData;

	switch (Z_TYPE_P(val)) {
		case IS_ARRAY:
			array_init(return_value);
			ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(val), h, key, pData) {
				{
					zval ret;
					sqlBuilderEscapeEx(&ret, pData, escapeValue);
					if (Z_TYPE(ret) == IS_STRING || Z_TYPE(ret) == IS_ARRAY || Z_TYPE(ret) == IS_NULL) {
						if (key) {
							key = sqlBuilderEscapeStr(key);
							add_assoc_zval_ex(return_value, ZSTR_VAL(key), ZSTR_LEN(key), &ret);
							zend_string_release(key);
						} else {
							add_index_zval(return_value, h, &ret);
						}
					} else {
						php_error_docref(NULL, E_ERROR, "expects parameter 2 to be a valid value");
						RETURN_FALSE;
					}
				}
			} ZEND_HASH_FOREACH_END();
			return;
		case IS_STRING:
			if (escapeValue) {
				RETURN_STR(sqlBuilderEscapeStr(Z_STR_P(val)));
			} else {
				RETURN_ZVAL(val, 1, 0);
			}
			return;
		case IS_LONG:
		case IS_DOUBLE:
			RETVAL_ZVAL(val, 1, 0);
			convert_to_string(return_value);
			return;
		case IS_TRUE:
			RETURN_STRINGL("1", 1);
		case IS_FALSE:
			RETURN_STRINGL("0", 1);
		case IS_NULL:
			RETURN_NULL();
	}
	RETURN_FALSE;
}
/* }}} */

/* {{{ proto sqlBuilderEscapeKeyword */
zend_string * sqlBuilderEscapeKeyword(zend_string *str)
{
	zend_string *ret;
	char *p = strchr(ZSTR_VAL(str), '.'), *prefix;
	size_t len;

	if (p) {
		len = p - ZSTR_VAL(str);
		if (len) {
			prefix = emalloc(len + 1);
			memcpy(prefix, ZSTR_VAL(str), len);
			*(prefix + len) = '\0';
			ret = strpprintf(0, "`%s`.`%s`", prefix, p + 1);
			efree(prefix);
		} else {
			ret = strpprintf(0, "`%s`", p + 1);
		}
	} else {
		ret = strpprintf(0, "`%s`", ZSTR_VAL(str));
	}
	return ret;
}
/* }}} */

/* {{{ proto sqlBuilderCompileBinds */
zend_string * sqlBuilderCompileBinds(zend_string *segment, zval *binds, zend_bool escapeValue)
{
	zend_ulong h;
	zend_string *key, *tstr, *delim, *field;
	zval args, *pData, inString, *pInString = NULL;
	char *p, *pos, *value;
	smart_str buf = {0};

	sqlBuilderEscapeEx(&args, binds, escapeValue);
	if (Z_TYPE(args) == IS_FALSE) {
		zval_ptr_dtor(&args);
		return zend_string_init(ZSTR_VAL(segment), ZSTR_LEN(segment), 0);
	}
	p = ZSTR_VAL(segment);
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL(args), h, key, pData) {
		pos = strchr(p, '?');
		if (pos) {
			// found
			smart_str_appendl(&buf, p, pos - p);
		} else {
			break;
		}
		if (*(pos + 1) == '?') {
			// keyword escape
			if (escapeValue) {
				tstr = sqlBuilderEscapeKeyword(Z_STR_P(pData));
			} else {
				// escape keyword field
				field = sqlBuilderEscapeStr(Z_STR_P(pData));
				tstr = sqlBuilderEscapeKeyword(field);
				zend_string_release(field);
			}
			p = pos + 2;
		} else {
			// value escape
			if (Z_TYPE_P(pData) == IS_ARRAY) {
				if (escapeValue) {
					delim = zend_string_init(ZEND_STRL("\",\""), 0);
				} else {
					delim = zend_string_init(ZEND_STRL(","), 0);
				}
				php_implode(delim, pData, &inString);
				zend_string_release(delim);
				value = Z_STRVAL(inString);
				pInString = &inString;
			} else if (Z_TYPE_P(pData) == IS_STRING) {
				value = Z_STRVAL_P(pData);
			} else {
				value = NULL;
			}
			if (key) {
				key = sqlBuilderEscapeKeyword(key);
				if (pInString) {
					if (escapeValue) {
						tstr = strpprintf(0, "%s IN (\"%s\")", ZSTR_VAL(key), value);
					} else {
						tstr = strpprintf(0, "%s IN (%s)", ZSTR_VAL(key), value);
					}
				} else if (value) {
					if (escapeValue) {
						tstr = strpprintf(0, "%s = \"%s\"", ZSTR_VAL(key), value);
					} else {
						tstr = strpprintf(0, "%s = %s", ZSTR_VAL(key), value);
					}
				} else {
					tstr = strpprintf(0, "%s IS NULL", ZSTR_VAL(key));
				}
				zend_string_release(key);
			} else {
				if (pInString) {
					if (escapeValue) {
						tstr = strpprintf(0, "(\"%s\")", value);
					} else {
						tstr = strpprintf(0, "(%s)", value);
					}
				} else if (value) {
					if (escapeValue) {
						tstr = strpprintf(0, "\"%s\"", value);
					} else {
						tstr = zend_string_copy(Z_STR_P(pData));
					}
				} else {
					tstr = zend_string_init(ZEND_STRL("NULL"), 0);
				}
			}
			if (pInString) {
				zval_ptr_dtor(pInString);
				pInString = NULL;
			}
			p = pos + 1;
		}
		smart_str_append(&buf, tstr);
		zend_string_release(tstr);
	} ZEND_HASH_FOREACH_END();
	zval_ptr_dtor(&args);

	if (p - ZSTR_VAL(segment) < ZSTR_LEN(segment)) {
		smart_str_appendl(&buf, p, ZSTR_LEN(segment) - (p - ZSTR_VAL(segment)));
	}
	smart_str_0(&buf);
	zend_string *ret = zend_string_copy(buf.s);
	smart_str_free(&buf);
	return ret;
}
/* }}} */

/* {{{ proto sqlBuilderWhereGroupStart */
static void sqlBuilderWhereGroupStart(zval *this, const char *pType)
{
	zend_long whereType = WHERETYPE_WHERE;
	zval *_where, *pWhere, *pWhereGroupPrefix, *pWhereGroupDepth, rec;
	zend_string *wherePrefix;

	_where = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_where"), 1, NULL);
	pWhere = zend_hash_index_find(Z_ARRVAL_P(_where), whereType);
	pWhereGroupPrefix = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_whereGroupPrefix"), 1, NULL);
	pWhereGroupDepth = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_WhereGroupDepth"), 1, NULL);
	// 不存在则初始化
	if (!pWhere) {
		pWhere = &rec;
		array_init(pWhere);
		zend_hash_index_add(Z_ARRVAL_P(_where), whereType, pWhere);
	}
	wherePrefix = sqlBuilderGetWherePrefix(Z_TYPE_P(pWhereGroupPrefix) == IS_TRUE, pWhere, pType);
	add_next_index_str(pWhere, strpprintf(0, "%s(", ZSTR_VAL(wherePrefix)));
	zend_string_release(wherePrefix);

	ZVAL_FALSE(pWhereGroupPrefix);
	++Z_LVAL_P(pWhereGroupDepth);
}
/* }}} */

/* {{{ proto sqlBuilderWhereGroupEnd */
static void sqlBuilderWhereGroupEnd(zval *this)
{
	zend_long whereType = WHERETYPE_WHERE;
	zval *_where, *pWhere, *pWhereGroupPrefix, *pWhereGroupDepth, rec;

	_where = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_where"), 1, NULL);
	pWhere = zend_hash_index_find(Z_ARRVAL_P(_where), whereType);
	pWhereGroupPrefix = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_whereGroupPrefix"), 1, NULL);
	pWhereGroupDepth = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_WhereGroupDepth"), 1, NULL);
	if (Z_LVAL_P(pWhereGroupDepth) == 0) {
		return;
	}
	// 不存在则初始化
	if (!pWhere) {
		pWhere = &rec;
		array_init(pWhere);
		zend_hash_index_add(Z_ARRVAL_P(_where), whereType, pWhere);
	}
	add_next_index_str(pWhere, zend_string_init(ZEND_STRL(")"), 0));

	ZVAL_TRUE(pWhereGroupPrefix);
	--Z_LVAL_P(pWhereGroupDepth);
}
/* }}} */

/* {{{ proto sqlBuilderCompileWhere */
zend_string * sqlBuilderCompileWhere(zval *this, zend_long whereType)
{
	zval *_where, *pWhere, *pWhereGroupDepth, wh;
	zend_string *delim, *ret;

	_where = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_where"), 1, NULL);
	pWhere = zend_hash_index_find(Z_ARRVAL_P(_where), whereType);
	if (!pWhere) {
		return NULL;
	}

	pWhereGroupDepth = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_WhereGroupDepth"), 1, NULL);
	while (Z_LVAL_P(pWhereGroupDepth) > 0) {
		sqlBuilderWhereGroupEnd(this);
	}

	delim = zend_string_init(ZEND_STRL(" "), 0);
	php_implode(delim, pWhere, &wh);
	zend_string_release(delim);
	ret = zend_string_copy(Z_STR(wh));
	zval_ptr_dtor(&wh);
	// reset after compiled
	zval_ptr_dtor(pWhere);
	array_init(pWhere);
	zend_update_property_bool(sqlBuilderCe, this, ZEND_STRL("_whereGroupPrefix"), 0);
	return ret;
}
/* }}} */

/* {{{ proto sqlBuilderGetWherePrefix */
static zend_string * sqlBuilderGetWherePrefix(zend_bool whereGroupPrefix, zval *pWhere, const char *pType)
{
	zend_string *wherePrefix;
	// check in whereGroupPrefix
	if (whereGroupPrefix == 0 || zend_hash_num_elements(Z_ARRVAL_P(pWhere)) == 0) {
		if (strstr(pType, "NOT")) {
			wherePrefix = zend_string_init(ZEND_STRL("NOT "), 0);
		} else {
			wherePrefix = ZSTR_EMPTY_ALLOC();
		}
	} else {
		// get type [AND, OR]
		if (pType && 0 == strcasecmp("OR", pType)) {
			wherePrefix = zend_string_init(ZEND_STRL("OR "), 0);
		} else if (pType && 0 == strcasecmp("AND NOT", pType)) {
			wherePrefix = zend_string_init(ZEND_STRL("AND NOT "), 0);
		} else if (pType && 0 == strcasecmp("OR NOT", pType)) {
			wherePrefix = zend_string_init(ZEND_STRL("OR NOT "), 0);
		} else {
			wherePrefix = zend_string_init(ZEND_STRL("AND "), 0);
		}
	}
	return wherePrefix;
}
/* }}} */

/* {{{ proto sqlBuilderWhere */
void sqlBuilderWhere(zval *this, zend_long whereType, zval *conditions, zval *value, const char *pType, zend_bool escapeValue)
{
	zval *_where, *pWhere, rec, cond, *pWhereGroupPrefix, *pData;
	zend_string *wherePrefix, *key, *op, *segment, *tstr;
	char *pOp;

	_where = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_where"), 1, NULL);
	pWhere = zend_hash_index_find(Z_ARRVAL_P(_where), whereType);
	pWhereGroupPrefix = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_whereGroupPrefix"), 1, NULL);

	if (!pWhere) {
		// 初始化 _where 数组
		pWhere = &rec;
		array_init(pWhere);
		zend_hash_index_add(Z_ARRVAL_P(_where), whereType, pWhere);
	}
	// 初始化 conditions
	if (Z_TYPE_P(conditions) != IS_ARRAY && Z_TYPE_P(conditions) != IS_STRING) {
		return;
	}
	if (Z_TYPE_P(conditions) == IS_STRING) {
		array_init(&cond);
		add_assoc_zval_ex(&cond, Z_STRVAL_P(conditions), Z_STRLEN_P(conditions), value);
		zval_add_ref(value);
		conditions = &cond;
	} else {
		zval_add_ref(conditions);
	}

	// 遍历 conditions
	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(conditions), key, pData) {
		if (!key) {
			continue;
		}
		zval array, binds;
		op = NULL;
		// 获取 where 前缀
		wherePrefix = sqlBuilderGetWherePrefix(Z_TYPE_P(pWhereGroupPrefix) == IS_TRUE, pWhere, pType);
		// 获取 condition 字段和运算符
		key = php_trim(key, ZEND_STRL(" "), 3);  // 去掉空格后 *key 指向的 zend_string, 记得要 release
		pOp = strchr(ZSTR_VAL(key), ' ');  // 通过空格分割条件
		if (pOp) {
			tstr = key;
			op = zend_string_init(pOp + 1, ZSTR_VAL(key) + ZSTR_LEN(key) - pOp - 1, 0);  // 空格后都属于运算符部分
			key = zend_string_init(ZSTR_VAL(key), pOp - ZSTR_VAL(key), 0);  // 空格之前都属于字段部分
			zend_string_release(tstr);  // 释放之前的 *key
			// 检查操作符是否合法
			if (strcmp(ZSTR_VAL(op), ">=") &&
					strcmp(ZSTR_VAL(op), "<=") &&
					strcmp(ZSTR_VAL(op), "<>") &&
					strcmp(ZSTR_VAL(op), "!=") &&
					strcmp(ZSTR_VAL(op), "=") &&
					strcmp(ZSTR_VAL(op), "<=>") &&
					strcmp(ZSTR_VAL(op), ">") &&
					strcmp(ZSTR_VAL(op), "<") &&
					strcasecmp(ZSTR_VAL(op), "IS") &&
					strcasecmp(ZSTR_VAL(op), "IS NOT") &&
					strcasecmp(ZSTR_VAL(op), "NOT IN") &&
					strcasecmp(ZSTR_VAL(op), "IN") &&
					strcasecmp(ZSTR_VAL(op), "LIKE") &&
					strcasecmp(ZSTR_VAL(op), "NOT LIKE")) {
				zend_string_release(op);
				op = NULL;
			} else {
				tstr = op;
				op = php_string_toupper(op);
				zend_string_release(tstr);
			}
		}
		if (!op) {
			// 默认操作符
			if (Z_TYPE_P(pData) == IS_NULL) {
				op = zend_string_init(ZEND_STRL("IS"), 0);
			} else if (Z_TYPE_P(pData) == IS_ARRAY) {
				op = zend_string_init(ZEND_STRL("IN"), 0);
			} else {
				op = zend_string_init(ZEND_STRL("="), 0);
			}
		}
		if ((0 == strcmp(ZSTR_VAL(op), "IN") || 0 == strcmp(ZSTR_VAL(op), "NOT IN")) && Z_TYPE_P(pData) != IS_ARRAY) {
			array_init(&array);
			add_next_index_zval(&array, pData);
			zval_add_ref(pData);
			pData = &array;
		} else {
			zval_add_ref(pData);
		}

		// build segment
		segment = strpprintf(0, "%s?? %s ?", ZSTR_VAL(wherePrefix), ZSTR_VAL(op));
		zend_string_release(op);
		zend_string_release(wherePrefix);
		array_init(&binds);
		add_next_index_str(&binds, zend_string_copy(key));
		add_next_index_zval(&binds, pData);
		add_next_index_str(pWhere, sqlBuilderCompileBinds(segment, &binds, escapeValue));
		zval_ptr_dtor(&binds);
		zend_string_release(segment);

		ZVAL_TRUE(pWhereGroupPrefix);
		zend_string_release(key);
	} ZEND_HASH_FOREACH_END();

	zval_ptr_dtor(conditions);
}
/* }}} */

/* {{{ proto __construct */
PHP_METHOD(azalea_sqlbuilder, __construct)
{
	zval *queryInstance = NULL, *this = getThis();
	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|o", &queryInstance) == FAILURE) {
		return;
	}
	if (queryInstance) {
		// 检查 *queryInstance 是否实现接口 Azalea\SqlBuilderQueryInterface
		// 已从 arg_info 判断, 因此不需要下面这段
//		if (!instanceof_function(Z_OBJCE_P(queryInstance), sqlBuilderQueryInterfaceCe)) {
//			php_error_docref(NULL, E_ERROR, "parameter 1 must be an implements of " AZALEA_NS_NAME(SqlBuilderQueryInterface));
//			RETURN_FALSE;
//		}
		zend_update_property(sqlBuilderCe, this, ZEND_STRL("_queryInstance"), queryInstance);
	}
	sqlBuilderReset(this);
}
/* }}} */

/* {{{ proto where */
PHP_METHOD(azalea_sqlbuilder, where)
{
	zval *conditions, *value, *this = getThis();
	zend_bool escapeValue = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|zb", &conditions, &value, &escapeValue) == FAILURE) {
		return;
	}

	sqlBuilderWhere(this, WHERETYPE_WHERE, conditions, value, "AND", escapeValue);
	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto orWhere */
PHP_METHOD(azalea_sqlbuilder, orWhere)
{
	zval *conditions, *value, *this = getThis();
	zend_bool escapeValue = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|zb", &conditions, &value, &escapeValue) == FAILURE) {
		return;
	}

	sqlBuilderWhere(this, WHERETYPE_WHERE, conditions, value, "OR", escapeValue);
	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto having */
PHP_METHOD(azalea_sqlbuilder, having)
{
	zval *conditions, *value, *this = getThis();
	zend_bool escapeValue = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|zb", &conditions, &value, &escapeValue) == FAILURE) {
		return;
	}

	sqlBuilderWhere(this, WHERETYPE_HAVING, conditions, value, "AND", escapeValue);
	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto orHaving */
PHP_METHOD(azalea_sqlbuilder, orHaving)
{
	zval *conditions, *value, *this = getThis();
	zend_bool escapeValue = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|zb", &conditions, &value, &escapeValue) == FAILURE) {
		return;
	}

	sqlBuilderWhere(this, WHERETYPE_HAVING, conditions, value, "OR", escapeValue);
	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto whereGroupStart */
PHP_METHOD(azalea_sqlbuilder, whereGroupStart)
{
	zval *this = getThis();
	sqlBuilderWhereGroupStart(this, "AND");
	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto orWhereGroupStart */
PHP_METHOD(azalea_sqlbuilder, orWhereGroupStart)
{
	zval *this = getThis();
	sqlBuilderWhereGroupStart(this, "OR");
	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto notWhereGroupStart */
PHP_METHOD(azalea_sqlbuilder, notWhereGroupStart)
{
	zval *this = getThis();
	sqlBuilderWhereGroupStart(this, "AND NOT");
	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto orNotWhereGroupStart */
PHP_METHOD(azalea_sqlbuilder, orNotWhereGroupStart)
{
	zval *this = getThis();
	sqlBuilderWhereGroupStart(this, "OR NOT");
	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto whereGroupEnd */
PHP_METHOD(azalea_sqlbuilder, whereGroupEnd)
{
	zval *this = getThis();
	sqlBuilderWhereGroupEnd(this);
	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto select */
PHP_METHOD(azalea_sqlbuilder, select)
{
	zval selects, *ret, *pSelect, *this = getThis(), *pData, escaped;
	zend_string *select, *delim;
	zend_bool escapeValue = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|b", &select, &escapeValue) == FAILURE) {
		return;
	}

	array_init(&selects);
	delim = zend_string_init(ZEND_STRL(","), 0);
	php_explode(delim, select, &selects, ZEND_LONG_MAX);
	zend_string_release(delim);

	// 转义
	if (escapeValue) {
		sqlBuilderEscapeEx(&escaped, &selects, 1);
		zval_ptr_dtor(&selects);
		ret = &escaped;
	} else {
		ret = &selects;
	}
	// 遍历写入 _select
	pSelect = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_select"), 1, NULL);
	ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(ret), pData) {
		add_next_index_str(pSelect, php_trim(Z_STR_P(pData), ZEND_STRL(" "), 3));
	} ZEND_HASH_FOREACH_END();
	zval_ptr_dtor(ret);

	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto count */
PHP_METHOD(azalea_sqlbuilder, count)
{
	zval *this = getThis(), *pSelect;
	zend_string *field = NULL, *alias = NULL, *value;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|SS", &alias, &field) == FAILURE) {
		return;
	}

	if (!field) {
		field = zend_string_init(ZEND_STRL("1"), 0);
	} else {
		field = php_trim(field, ZEND_STRL(" "), 3);
	}
	pSelect = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_select"), 1, NULL);
	if (alias) {
		alias = php_trim(alias, ZEND_STRL(" "), 3);
		value = strpprintf(0, "COUNT(%s) AS `%s`", ZSTR_VAL(field), ZSTR_VAL(alias));
		zend_string_release(alias);
	} else {
		value = strpprintf(0, "COUNT(%s)", ZSTR_VAL(field));
	}
	add_next_index_str(pSelect, value);
	zend_string_release(field);

	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto distinct */
PHP_METHOD(azalea_sqlbuilder, distinct)
{
	zval *pDistinct, *this = getThis();
	zend_bool distinct = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "|b", &distinct) == FAILURE) {
		return;
	}

	pDistinct = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_distinct"), 1, NULL);
	ZVAL_BOOL(pDistinct, distinct);

	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto sqlBuilderFrom */
void sqlBuilderFrom(zval *this, zval *from, zend_string *joinType, zend_string *joinCondition)
{
	zval froms, *pFrom, *pJoin, *pData;
	zend_ulong h;
	zend_string *key, *tableName, *value, *tstr, *delim;
	char *pos;

	// 表名
	if (Z_TYPE_P(from) == IS_STRING) {
		delim = zend_string_init(ZEND_STRL(","), 0);
		array_init(&froms);
		php_explode(delim, Z_STR_P(from), &froms, ZEND_LONG_MAX);
		zend_string_release(delim);
		from = &froms;
	} else {
		zval_add_ref(from);
	}
	// 遍历写入 _from
	pFrom = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_from"), 1, NULL);
	pJoin = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_join"), 1, NULL);
	ZEND_HASH_FOREACH_KEY_VAL(Z_ARRVAL_P(from), h, key, pData) {
		if (Z_TYPE_P(pData) != IS_STRING) {
			continue;
		}
		if (key) {
			// 'alias' => 'tableName'
			key = php_trim(key, ZEND_STRL(" "), 3);
			tstr = php_trim(Z_STR_P(pData), ZEND_STRL(" "), 3);
			tableName = sqlBuilderEscapeKeyword(tstr);
			zend_string_release(tstr);
			value = strpprintf(0, "%s AS `%s`", ZSTR_VAL(tableName), ZSTR_VAL(key));
			zend_string_release(key);
			zend_string_release(tableName);
		} else {
			tableName = php_trim(Z_STR_P(pData), ZEND_STRL(" "), 3);
			pos = strchr(ZSTR_VAL(tableName), ' ');
			if (pos) {
				// 0 => 'tableName alias'
				tstr = zend_string_init(pos + 1, ZSTR_VAL(tableName) + ZSTR_LEN(tableName) - pos - 1, 0);
				key = php_trim(tstr, ZEND_STRL(" "), 1);
				zend_string_release(tstr);
				tstr= zend_string_init(ZSTR_VAL(tableName), pos - ZSTR_VAL(tableName), 0);
				zend_string_release(tableName);
				tableName = sqlBuilderEscapeKeyword(tstr);
				zend_string_release(tstr);
				value = strpprintf(0, "%s AS `%s`", ZSTR_VAL(tableName), ZSTR_VAL(key));
				zend_string_release(key);
			} else {
				// 0 => 'tableName'
				tstr = php_trim(Z_STR_P(pData), ZEND_STRL(" "), 3);
				value = sqlBuilderEscapeKeyword(tstr);
				zend_string_release(tstr);
			}
			zend_string_release(tableName);
		}
		if (!joinType && !joinCondition && zend_hash_num_elements(Z_ARRVAL_P(pFrom)) == 0) {
			// from
			add_next_index_str(pFrom, value);
		} else {
			// join
			tstr = value;
			if (joinType && joinCondition) {
				value = strpprintf(0, "%s %s %s", ZSTR_VAL(joinType), ZSTR_VAL(value), ZSTR_VAL(joinCondition));
			} else  {
				value = strpprintf(0, "CROSS JOIN %s", ZSTR_VAL(value));
			}
			zend_string_release(tstr);
			add_next_index_str(pJoin, value);
		}
	} ZEND_HASH_FOREACH_END();
	zval_ptr_dtor(from);
}
/* }}} */

/* {{{ proto from */
PHP_METHOD(azalea_sqlbuilder, from)
{
	zval *from, *this = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z", &from) == FAILURE) {
		return;
	}
	if (Z_TYPE_P(from) != IS_ARRAY && Z_TYPE_P(from) != IS_STRING) {
		php_error_docref(NULL, E_ERROR, "expects parameter 1 to be string or array");
		return;
	}
	sqlBuilderFrom(this, from, NULL, NULL);

	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto join */
PHP_METHOD(azalea_sqlbuilder, join)
{
	zval *join, *this = getThis();
	zend_string *condition, *type = NULL;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "zS|S", &join, &condition, &type) == FAILURE) {
		return;
	}
	if (Z_TYPE_P(join) != IS_ARRAY && Z_TYPE_P(join) != IS_STRING) {
		php_error_docref(NULL, E_ERROR, "expects parameter 1 to be string or array");
		return;
	}

	// 连接条件, 记得 release
	if (strchr(ZSTR_VAL(condition), '=')) {
		condition = strpprintf(0, "ON (%s)", ZSTR_VAL(condition));
	} else {
		condition = strpprintf(0, "USING (%s)", ZSTR_VAL(condition));
	}
	// 连接类型, 记得 release
	if (type && 0 == strcasecmp(ZSTR_VAL(type), "left")) {
		type = zend_string_init(ZEND_STRL("LEFT JOIN"), 0);
	} else if (type && 0 == strcasecmp(ZSTR_VAL(type), "right")) {
		type = zend_string_init(ZEND_STRL("RIGHT JOIN"), 0);
	} else {
		type = zend_string_init(ZEND_STRL("INNER JOIN"), 0);
	}
	sqlBuilderFrom(this, join, type, condition);
	zend_string_release(condition);
	zend_string_release(type);

	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto limit */
PHP_METHOD(azalea_sqlbuilder, limit)
{
	zend_long limit, offset = 0;
	zval *pValue, *this = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|l", &limit, &offset) == FAILURE) {
		return;
	}

	if (limit >= 0) {
		pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_limit"), 1, NULL);
		ZVAL_LONG(pValue, limit);
		if (offset >= 0) {
			pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_offset"), 1, NULL);
			ZVAL_LONG(pValue, offset);
		}
	}

	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto limitPage */
PHP_METHOD(azalea_sqlbuilder, limitPage)
{
	zend_long limit, page = 1, offset;
	zval *pValue, *this = getThis();

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "l|l", &limit, &page) == FAILURE) {
		return;
	}

	if (limit >= 0) {
		pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_limit"), 1, NULL);
		ZVAL_LONG(pValue, limit);
		if (page >= 1) {
			offset = limit * (page - 1);
			pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_offset"), 1, NULL);
			ZVAL_LONG(pValue, offset);
		}
	}

	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto sqlBuilderBy */
void sqlBuilderBy(zval *this, zend_long byType, zval *byValue, zend_bool escapeValue)
{
	zval byValues, escaped, rec, *pData, *_by, *pBy = NULL;
	zend_string *delim;
	zend_ulong h;

	// 组装数组
	if (Z_TYPE_P(byValue) == IS_STRING) {
		delim = zend_string_init(ZEND_STRL(","), 0);
		array_init(&byValues);
		php_explode(delim, Z_STR_P(byValue), &byValues, ZEND_LONG_MAX);
		zend_string_release(delim);
		byValue = &byValues;
	} else {
		zval_add_ref(byValue);
	}
	// 转义
	if (escapeValue) {
		sqlBuilderEscapeEx(&escaped, byValue, 1);
		zval_ptr_dtor(byValue);
		byValue = &escaped;
	}
	// foreach
	_by = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_by"), 1, NULL);
	pBy = zend_hash_index_find(Z_ARRVAL_P(_by), byType);
	if (!pBy) {
		// 初始化 _by 数组
		pBy = &rec;
		array_init(pBy);
		zend_hash_index_add(Z_ARRVAL_P(_by), byType, pBy);
	}
	ZEND_HASH_FOREACH_NUM_KEY_VAL(Z_ARRVAL_P(byValue), h, pData) {
		if (Z_TYPE_P(pData) != IS_STRING) {
			continue;
		}
		add_next_index_str(pBy, php_trim(Z_STR_P(pData), ZEND_STRL(" "), 3));
	} ZEND_HASH_FOREACH_END();
	zval_ptr_dtor(byValue);
}
/* }}} */

/* {{{ proto orderBy */
PHP_METHOD(azalea_sqlbuilder, orderBy)
{
	zval *orderBy, *this = getThis();
	zend_bool escapeValue = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|b", &orderBy, &escapeValue) == FAILURE) {
		return;
	}
	if (Z_TYPE_P(orderBy) != IS_ARRAY && Z_TYPE_P(orderBy) != IS_STRING) {
		php_error_docref(NULL, E_ERROR, "expects parameter 1 to be string or array");
		return;
	}
	sqlBuilderBy(this, BYTYPE_ORDER, orderBy, escapeValue);

	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto groupBy */
PHP_METHOD(azalea_sqlbuilder, groupBy)
{
	zval *groupBy, *this = getThis();
	zend_bool escapeValue = 1;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "z|b", &groupBy, &escapeValue) == FAILURE) {
		return;
	}
	if (Z_TYPE_P(groupBy) != IS_ARRAY && Z_TYPE_P(groupBy) != IS_STRING) {
		php_error_docref(NULL, E_ERROR, "expects parameter 1 to be string or array");
		return;
	}
	sqlBuilderBy(this, BYTYPE_GROUP, groupBy, escapeValue);

	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto sqlBuilderSet */
void sqlBuilderSet(zval *this, zval *set)
{
	zval *pSet, *pData;
	zend_string *key, *tstr;

	pSet = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_set"), 1, NULL);
	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(set), key, pData) {
		if (!key) {
			continue;
		}
		zval value;
		key = sqlBuilderEscapeStr(key);
		if (Z_TYPE_P(pData) == IS_ARRAY) {
			// escape & value
			zval *escape, *pValue;
			if (!(pValue = zend_hash_str_find(Z_ARRVAL_P(pData), ZEND_STRL("value")))) {
				continue;
			}
			if ((escape = zend_hash_str_find(Z_ARRVAL_P(pData), ZEND_STRL("escape")))) {
				convert_to_boolean(escape);
			}
			sqlBuilderEscapeEx(&value, pValue, !escape || Z_TYPE_P(escape) == IS_TRUE);
			tstr = zend_string_copy(Z_STR(value));
		} else {
			// string
			sqlBuilderEscapeEx(&value, pData, 1);
			tstr = strpprintf(0, "\"%s\"", Z_STRVAL(value));
		}
		add_assoc_str_ex(pSet, ZSTR_VAL(key), ZSTR_LEN(key), tstr);
		zend_string_release(key);
		zval_ptr_dtor(&value);
	} ZEND_HASH_FOREACH_END();
}
/* }}} */

/* {{{ proto insert */
PHP_METHOD(azalea_sqlbuilder, insert)
{
	zval *set, *this = getThis(), *excludeFields, from, *pSet, *pExcludeFields, *pData;
	zend_bool ignoreErrors = 0, duplicateKeyUpdate = 0;
	zend_string *tableName;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Sa|bbz", &tableName, &set, &ignoreErrors, &duplicateKeyUpdate, &excludeFields) == FAILURE) {
		return;
	}
	// reset
	sqlBuilderReset(this);
	// from
	ZVAL_STR(&from, zend_string_copy(tableName));
	sqlBuilderFrom(this, &from, NULL, NULL);
	zval_ptr_dtor(&from);
	// set
	sqlBuilderSet(this, set);
	// exclude
	if (excludeFields && Z_TYPE_P(excludeFields) == IS_ARRAY) {
		pSet = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_set"), 1, NULL);
		pExcludeFields = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_excludeFields"), 1, NULL);
		ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(excludeFields), pData) {
			if (Z_TYPE_P(pData) != IS_STRING) {
				continue;
			}
			if (zend_hash_str_exists(Z_ARRVAL_P(pSet), Z_STRVAL_P(pData), Z_STRLEN_P(pData))) {
				// 排除字段存在 _set 中
				add_next_index_zval(pExcludeFields, pData);
				zval_add_ref(pData);
			}
		} ZEND_HASH_FOREACH_END();
	}
	// others
	zend_update_property_long(sqlBuilderCe, this, ZEND_STRL("_type"), SQLTYPE_INSERT);
	zend_update_property_bool(sqlBuilderCe, this, ZEND_STRL("_ignore"), ignoreErrors);
	zend_update_property_bool(sqlBuilderCe, this, ZEND_STRL("_duplicateKeyUpdate"), duplicateKeyUpdate);

	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto replace */
PHP_METHOD(azalea_sqlbuilder, replace)
{
	zval *set, *this = getThis(), from;
	zend_string *tableName;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Sa", &tableName, &set) == FAILURE) {
		return;
	}
	// reset
	sqlBuilderReset(this);
	// from
	ZVAL_STR(&from, zend_string_copy(tableName));
	sqlBuilderFrom(this, &from, NULL, NULL);
	zval_ptr_dtor(&from);
	// set
	sqlBuilderSet(this, set);
	// others
	zend_update_property_long(sqlBuilderCe, this, ZEND_STRL("_type"), SQLTYPE_REPLACE);

	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto sqlBuilderUpdateDeleteWhere */
void sqlBuilderUpdateDeleteWhere(zval *this, zval *where)
{
	zval zKey, *pData;
	zend_string *key;

	ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(where), key, pData) {
		if (!key) {
			continue;
		}
		ZVAL_STR(&zKey, zend_string_copy(key));
		if (Z_TYPE_P(pData) == IS_ARRAY) {
			// escape & value
			zval *escape, *pValue;
			if (!(pValue = zend_hash_str_find(Z_ARRVAL_P(pData), ZEND_STRL("value")))) {
				pValue = pData;
			}
			if ((escape = zend_hash_str_find(Z_ARRVAL_P(pData), ZEND_STRL("escape")))) {
				convert_to_boolean(escape);
			}
			sqlBuilderWhere(this, WHERETYPE_WHERE, &zKey, pValue, "AND", !escape || Z_TYPE_P(escape) == IS_TRUE);
		} else {
			// string
			sqlBuilderWhere(this, WHERETYPE_WHERE, &zKey, pData, "AND", 1);
		}
		zval_ptr_dtor(&zKey);
	} ZEND_HASH_FOREACH_END();
}
/* }}} */

/* {{{ proto update */
PHP_METHOD(azalea_sqlbuilder, update)
{
	zval *set, *where = NULL, *this = getThis(), from;
	zend_string *tableName;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "Sa|a", &tableName, &set, &where) == FAILURE) {
		return;
	}
	// reset
	sqlBuilderReset(this);
	// from
	ZVAL_STR(&from, zend_string_copy(tableName));
	sqlBuilderFrom(this, &from, NULL, NULL);
	zval_ptr_dtor(&from);
	// set
	sqlBuilderSet(this, set);
	// where
	if (where) {
		sqlBuilderUpdateDeleteWhere(this, where);
	}
	// others
	zend_update_property_long(sqlBuilderCe, this, ZEND_STRL("_type"), SQLTYPE_UPDATE);

	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ proto delete */
PHP_METHOD(azalea_sqlbuilder, delete)
{
	zval *where = NULL, *this = getThis(), from, zKey, *pData;
	zend_string *tableName, *key;

	if (zend_parse_parameters(ZEND_NUM_ARGS(), "S|a", &tableName, &where) == FAILURE) {
		return;
	}
	// reset
	sqlBuilderReset(this);
	// from
	ZVAL_STR(&from, zend_string_copy(tableName));
	sqlBuilderFrom(this, &from, NULL, NULL);
	zval_ptr_dtor(&from);
	// where
	if (where) {
		sqlBuilderUpdateDeleteWhere(this, where);
	}
	// others
	zend_update_property_long(sqlBuilderCe, this, ZEND_STRL("_type"), SQLTYPE_DELETE);

	RETURN_ZVAL(this, 1, 0);
}
/* }}} */

/* {{{ sqlBuilderCompileSql */
static zend_string * sqlBuilderCompileSql(zval *this)
{
	zval *pParentNode, *pValue, *pSet, *pData, setValues, dummy;
	smart_str buf = {0}, *sql = &buf;
	zend_string *tstr, *delimComma, *delimSpace, *key, *ret;
	zend_long sqlType;

	delimComma = zend_string_init(ZEND_STRL(","), 0);
	delimSpace = zend_string_init(ZEND_STRL(" "), 0);
	pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_type"), 1, NULL);
	sqlType = Z_LVAL_P(pValue);
	// 生成不同类型的 SQL
	switch (sqlType) {
		case SQLTYPE_INSERT:
		case SQLTYPE_REPLACE:
		case SQLTYPE_UPDATE:
			if (sqlType == SQLTYPE_INSERT) {
				smart_str_appendl_ex(sql, ZEND_STRL("INSERT"), 0);
				// ignore
				pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_ignore"), 1, NULL);
				if (Z_TYPE_P(pValue) == IS_TRUE) {
					smart_str_appendl_ex(sql, ZEND_STRL(" IGNORE"), 0);
				}
			} else if (sqlType == SQLTYPE_REPLACE) {
				smart_str_appendl_ex(sql, ZEND_STRL("REPLACE"), 0);
			} else {
				smart_str_appendl_ex(sql, ZEND_STRL("UPDATE "), 0);
				goto _from;
			}
			smart_str_appendl_ex(sql, ZEND_STRL(" INTO "), 0);
_from:
			// from
			pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_from"), 1, NULL);
			if (Z_TYPE_P(pValue) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(pValue)) > 0) {
				php_implode(delimComma, pValue, &dummy);
				smart_str_append(sql, Z_STR(dummy));
				zval_ptr_dtor(&dummy);
			}
			smart_str_appendl_ex(sql, ZEND_STRL(" SET"), 0);
			// set
			pSet = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_set"), 1, NULL);
			if (Z_TYPE_P(pSet) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(pSet)) > 0) {
				array_init(&setValues);
				ZEND_HASH_FOREACH_STR_KEY_VAL(Z_ARRVAL_P(pSet), key, pData) {
					key = sqlBuilderEscapeKeyword(key);
					tstr = strpprintf(0, " %s = %s", ZSTR_VAL(key), Z_STRVAL_P(pData));
					add_next_index_str(&setValues, tstr);
					zend_string_release(key);
				} ZEND_HASH_FOREACH_END();
				php_implode(delimComma, &setValues, &dummy);
				smart_str_append(sql, Z_STR(dummy));
				zval_ptr_dtor(&setValues);
				zval_ptr_dtor(&dummy);
			}
			if (sqlType == SQLTYPE_INSERT) {
				// onDuplicateKeyUpdate
				pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_duplicateKeyUpdate"), 1, NULL);
				if (Z_TYPE_P(pValue) == IS_TRUE) {
					pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_excludeFields"), 1, NULL);
					if (Z_TYPE_P(pValue) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(pValue)) < zend_hash_num_elements(Z_ARRVAL_P(pSet))) {
						smart_str_appendl_ex(sql, ZEND_STRL(" ON DUPLICATE KEY UPDATE"), 0);
						array_init(&setValues);
						ZEND_HASH_FOREACH_VAL(Z_ARRVAL_P(pValue), pData) {
							key = sqlBuilderEscapeKeyword(Z_STR_P(pData));
							tstr = strpprintf(0, " %s = VALUES(%s)", ZSTR_VAL(key), ZSTR_VAL(key));
							add_next_index_str(&setValues, tstr);
							zend_string_release(key);
						} ZEND_HASH_FOREACH_END();
						php_implode(delimComma, &setValues, &dummy);
						smart_str_append(sql, Z_STR(dummy));
						zval_ptr_dtor(&setValues);
						zval_ptr_dtor(&dummy);
					}
				}
			} else if (sqlType == SQLTYPE_UPDATE) {
				goto _where;
			}
			break;
		case SQLTYPE_DELETE:
			smart_str_appendl_ex(sql, ZEND_STRL("DELETE FROM "), 0);
			// from
			pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_from"), 1, NULL);
			if (Z_TYPE_P(pValue) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(pValue)) > 0) {
				php_implode(delimComma, pValue, &dummy);
				smart_str_append(sql, Z_STR(dummy));
				zval_ptr_dtor(&dummy);
			}
_where:
			// where
			tstr = sqlBuilderCompileWhere(this, WHERETYPE_WHERE);
			if (tstr && ZSTR_LEN(tstr)) {
				smart_str_appendl_ex(sql, ZEND_STRL(" WHERE "), 0);
				smart_str_append(sql, tstr);
				zend_string_release(tstr);
			}
			break;
		default:
			smart_str_appendl_ex(sql, ZEND_STRL("SELECT"), 0);
			// distinct
			pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_distinct"), 1, NULL);
			if (Z_TYPE_P(pValue) == IS_TRUE) {
				smart_str_appendl_ex(sql, ZEND_STRL(" DISTINCT"), 0);
			}
			// select
			pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_select"), 1, NULL);
			if (Z_TYPE_P(pValue) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(pValue)) > 0) {
				php_implode(delimComma, pValue, &dummy);
				smart_str_appendc(sql, ' ');
				smart_str_append(sql, Z_STR(dummy));
				zval_ptr_dtor(&dummy);
			} else {
				smart_str_appendl_ex(sql, ZEND_STRL(" *"), 0);
			}
			// from
			pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_from"), 1, NULL);
			if (Z_TYPE_P(pValue) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(pValue)) > 0) {
				php_implode(delimComma, pValue, &dummy);
				smart_str_appendl_ex(sql, ZEND_STRL(" FROM "), 0);
				smart_str_append(sql, Z_STR(dummy));
				zval_ptr_dtor(&dummy);
			}
			// join
			pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_join"), 1, NULL);
			if (Z_TYPE_P(pValue) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(pValue)) > 0) {
				php_implode(delimSpace, pValue, &dummy);
				smart_str_appendc(sql, ' ');
				smart_str_append(sql, Z_STR(dummy));
				zval_ptr_dtor(&dummy);
			}
			// where
			tstr = sqlBuilderCompileWhere(this, WHERETYPE_WHERE);
			if (tstr && ZSTR_LEN(tstr)) {
				smart_str_appendl_ex(sql, ZEND_STRL(" WHERE "), 0);
				smart_str_append(sql, tstr);
				zend_string_release(tstr);
			}
			// groupby
			pParentNode = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_by"), 1, NULL);
			pValue = zend_hash_index_find(Z_ARRVAL_P(pParentNode), BYTYPE_GROUP);
			if (pValue && Z_TYPE_P(pValue) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(pValue)) > 0) {
				php_implode(delimComma, pValue, &dummy);
				smart_str_appendl_ex(sql, ZEND_STRL(" GROUP BY "), 0);
				smart_str_append(sql, Z_STR(dummy));
				zval_ptr_dtor(&dummy);
			}
			// orderby
			pValue = zend_hash_index_find(Z_ARRVAL_P(pParentNode), BYTYPE_ORDER);
			if (pValue && Z_TYPE_P(pValue) == IS_ARRAY && zend_hash_num_elements(Z_ARRVAL_P(pValue)) > 0) {
				php_implode(delimComma, pValue, &dummy);
				smart_str_appendl_ex(sql, ZEND_STRL(" ORDER BY "), 0);
				smart_str_append(sql, Z_STR(dummy));
				zval_ptr_dtor(&dummy);
			}
			// having
			tstr = sqlBuilderCompileWhere(this, WHERETYPE_HAVING);
			if (tstr && ZSTR_LEN(tstr)) {
				smart_str_appendl_ex(sql, ZEND_STRL(" HAVING "), 0);
				smart_str_append(sql, tstr);
				zend_string_release(tstr);
			}
			// limit & offset
			pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_limit"), 1, NULL);
			if (Z_TYPE_P(pValue) == IS_LONG) {
				zend_long limit = Z_LVAL_P(pValue);
				smart_str_appendl_ex(sql, ZEND_STRL(" LIMIT "), 0);
				pValue = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_offset"), 1, NULL);
				if (Z_TYPE_P(pValue) == IS_LONG && Z_LVAL_P(pValue) > 0) {
					smart_str_append_long(sql, Z_LVAL_P(pValue));
					smart_str_appendc(sql, ',');
				}
				smart_str_append_long(sql, limit);
			}
	}
	smart_str_0(sql);
	ret = zend_string_copy(buf.s);

	zend_string_release(delimComma);
	zend_string_release(delimSpace);
	smart_str_free(sql);
	sqlBuilderReset(this);
	return ret;
}
/* }}} */

/* {{{ proto getSql */
PHP_METHOD(azalea_sqlbuilder, getSql)
{
	RETURN_STR(sqlBuilderCompileSql(getThis()));
}
/* }}} */

/* {{{ proto query */
PHP_METHOD(azalea_sqlbuilder, query)
{
	zval *this = getThis(), *queryInstance, functionName, *callArgs = NULL, *params = NULL, *arg;
	zend_string *sql;
	uint32_t paramsCount = 0, i;

	if ((queryInstance = zend_read_property(sqlBuilderCe, this, ZEND_STRL("_queryInstance"), 1, NULL))) {
		if (zend_parse_parameters(ZEND_NUM_ARGS(), "*", &params, &paramsCount) == FAILURE) {
			return;
		}

		// 如果构造时已传入接口实例, 则调用 $this->_queryInstance->query()
		ZVAL_NEW_STR(&functionName, zend_string_init(ZEND_STRL("query"), 0));
		sql = sqlBuilderCompileSql(this);
		// 动态构造参数
		callArgs = safe_emalloc(sizeof(zval), 1, 0);
		ZVAL_STR(callArgs, sql);
		for (i = 0; i < paramsCount; ++i) {
			// 复制后续参数
			ZVAL_COPY_VALUE(callArgs + i + 1, params + i);
		}
		if (SUCCESS != call_user_function(&(sqlBuilderQueryInterfaceCe->function_table), queryInstance, &functionName, return_value, paramsCount + 1, callArgs)) {
			RETVAL_FALSE;
		}
		// 释放
		zval_ptr_dtor(&functionName);
		for (i = 0; i <= paramsCount; ++i) {
			zval_ptr_dtor(callArgs + i);
		}
		efree(callArgs);
		return;
	}
	RETURN_FALSE;
}
/* }}} */
