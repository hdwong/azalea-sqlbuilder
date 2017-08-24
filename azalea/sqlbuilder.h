/*
 * azalea/sqlbuilder.h
 *
 * Created by Bun Wong on 17-8-23.
 */

#ifndef AZALEA_SQLBUILDER_H_
#define AZALEA_SQLBUILDER_H_

#define FROMTYPE_FROM 0  // from
#define FROMTYPE_JOIN 1  // join

#define WHERETYPE_WHERE  0	// where
#define WHERETYPE_HAVING 1  // having

#define BYTYPE_ORDER 0  // order by
#define BYTYPE_GROUP 1  // group by

ZEND_MODULE_STARTUP_D(main);

PHP_METHOD(azalea_sqlbuilder, __construct);
PHP_METHOD(azalea_sqlbuilder, where);
PHP_METHOD(azalea_sqlbuilder, orWhere);
PHP_METHOD(azalea_sqlbuilder, having);
PHP_METHOD(azalea_sqlbuilder, orHaving);
PHP_METHOD(azalea_sqlbuilder, whereGroupStart);
PHP_METHOD(azalea_sqlbuilder, orWhereGroupStart);
PHP_METHOD(azalea_sqlbuilder, notWhereGroupStart);
PHP_METHOD(azalea_sqlbuilder, orNotWhereGroupStart);
PHP_METHOD(azalea_sqlbuilder, whereGroupEnd);
PHP_METHOD(azalea_sqlbuilder, select);
PHP_METHOD(azalea_sqlbuilder, distinct);
PHP_METHOD(azalea_sqlbuilder, from);
PHP_METHOD(azalea_sqlbuilder, join);
PHP_METHOD(azalea_sqlbuilder, limit);
PHP_METHOD(azalea_sqlbuilder, limitPage);
PHP_METHOD(azalea_sqlbuilder, orderBy);
PHP_METHOD(azalea_sqlbuilder, groupBy);
PHP_METHOD(azalea_sqlbuilder, getSql);
PHP_METHOD(azalea_sqlbuilder, query);

#endif /* AZALEA_SQLBUILDER_H_ */
