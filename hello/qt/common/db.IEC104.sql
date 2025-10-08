
CREATE TABLE `YC_POINT_TABLE`
(
    `ID`             integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    `DATABASE_NAME`  TEXT    NOT NULL DEFAULT '',
    `TABLE_NAME`     text    NOT NULL DEFAULT '',
    `DEVICE_ATTR`    text    NOT NULL DEFAULT '',
    `CA`             integer NOT NULL DEFAULT -1,
    `IOA`            integer NOT NULL DEFAULT -1,
    `TYPE_ID`        integer NOT NULL DEFAULT -1,
    `NORMALIZED_MAX` real    NOT NULL DEFAULT 0,
    `SCALED_FACTOR`  real    NOT NULL DEFAULT 0,
    `PRECISION`      integer NOT NULL DEFAULT 1
);

CREATE TABLE `YX_POINT_TABLE`
(
    `ID`            integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    `DATABASE_NAME` TEXT    NOT NULL,
    `TABLE_NAME`    text    NOT NULL DEFAULT '',
    `DEVICE_ATTR`   text    NOT NULL DEFAULT '',
    `POINT_TYPE`    integer          DEFAULT 0,
    `CA`            integer NOT NULL DEFAULT -1,
    `IOA`           integer NOT NULL DEFAULT -1,
    `TYPE_ID`       integer NOT NULL DEFAULT -1
);
