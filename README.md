# AZALEA-SQLBUILDER
A SqlBuilder implemented as a C extension for PHP, serialed Azalea, to help you to build some SQL syntax like SELECT, INSERT, UPDATE, DELETE...

Welcome to join QQ group **576346826** to communicate with me.

---

## Azalea\SqlBuilder

### Azalea\SqlBuilder::__construct
构造函数，可接受一个 `Azalea\SqlBuilderQueryInterface` 的实现类对象（适配者），并在调用 `Azalea\SqlBuilder::query` 时，自动请求该对象上的 `query` 方法

### Azalea\SqlBuilder::select
设置 SELECT

### Azalea\SqlBuilder::distinct
设置 DISTINCT

### Azalea\SqlBuilder::count
设置 COUNT

### Azalea\SqlBuilder::from
设置 FROM

### Azalea\SqlBuilder::join
设置 JOIN

### Azalea\SqlBuilder::where
设置 WHERE

### Azalea\SqlBuilder::orWhere
同上

### Azalea\SqlBuilder::whereGroupStart
设置 WHERE 嵌套

### Azalea\SqlBuilder::orWhereGroupStart
同上

### Azalea\SqlBuilder::notWhereGroupStart
同上

### Azalea\SqlBuilder::orNotWhereGroupStart
同上

### Azalea\SqlBuilder::whereGroupEnd
关闭 WHERE 嵌套

### Azalea\SqlBuilder::having
设置 HAVING

### Azalea\SqlBuilder::orHaving
同上

### Azalea\SqlBuilder::limit
设置 LIMIT

### Azalea\SqlBuilder::limitPage
设置 LIMIT，根据页码

### Azalea\SqlBuilder::orderBy
设置 ORDER BY

### Azalea\SqlBuilder::groupBy
设置 GROUP BY

### Azalea\SqlBuilder::insert
INSERT 语法

### Azalea\SqlBuilder::replace
REPLACE 语法

### Azalea\SqlBuilder::update
UPDATE 语法

### Azalea\SqlBuilder::delete
DELETE 语法

### Azalea\SqlBuilder::getSql
构建 SQL 语句

### Azalea\SqlBuilder::query
执行 SQL 语句，必须在对象构造时传入 `Azalea\SqlBuilderQueryInterface` 实现

---

## Azalea\SqlBuilderQueryInterface

### Azalea\SqlBuilderQueryInterface::query
