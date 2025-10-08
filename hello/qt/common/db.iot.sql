CREATE TABLE `IOT_YC_POINT_TABLE`
(
    `ID`                integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    `DATABASE_NAME`     TEXT    NOT NULL DEFAULT '',
    `TABLE_NAME`        text    NOT NULL DEFAULT '',
    `DEVICE_ATTR`       text    NOT NULL DEFAULT '',
    `CA`                integer NOT NULL DEFAULT -1,
    `IOA`               integer NOT NULL DEFAULT -1,
    `TYPE_ID`           integer NOT NULL DEFAULT -1,
    `NORMALIZED_MAX`    real    NOT NULL DEFAULT 0,
    `SCALED_FACTOR`     real    NOT NULL DEFAULT 0,
    `PRECISION`         integer NOT NULL DEFAULT 1,
    `MODEL`             text    NOT NULL DEFAULT '',
    `MODEL_ATTR`        text    NOT NULL DEFAULT '',
    `MODEL_ATTR_TYPE`   text    NOT NULL DEFAULT '',
    `MODEL_GUID`        text    NOT NULL DEFAULT '',
    `MODEL_DEAD_ZONE`   text    NOT NULL DEFAULT '',
    `MODEL_RATIO`       text    NOT NULL DEFAULT '',
    `MODEL_IS_REPORT`   text    NOT NULL DEFAULT '',
    `MODEL_USER_DEFINE` text    NOT NULL DEFAULT '',
    `RES1`              text    NOT NULL DEFAULT '',
    `RES2`              text    NOT NULL DEFAULT '',
    `RES3`              text    NOT NULL DEFAULT ''
);


CREATE TABLE `IOT_YX_POINT_TABLE`
(
    `ID`                integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    `DATABASE_NAME`     TEXT    NOT NULL,
    `TABLE_NAME`        text    NOT NULL DEFAULT '',
    `DEVICE_ATTR`       text    NOT NULL DEFAULT '',
    `POINT_TYPE`        integer NOT NULL DEFAULT -1,
    `CA`                integer NOT NULL DEFAULT -1,
    `IOA`               integer NOT NULL DEFAULT -1,
    `TYPE_ID`           integer NOT NULL DEFAULT -1,
    `EVENT_TYPE`        integer NOT NULL DEFAULT -1,
    `MODEL`             text    NOT NULL DEFAULT '',
    `MODEL_ATTR`        text    NOT NULL DEFAULT '',
    `MODEL_ATTR_TYPE`   text    NOT NULL DEFAULT '',
    `MODEL_EVENT_TYPE`  text    NOT NULL DEFAULT '',
    `MODEL_EVENT_PARAM` text    NOT NULL DEFAULT '',
    `MODEL_GUID`        text    NOT NULL DEFAULT '',
    `MODEL_DEAD_ZONE`   text    NOT NULL DEFAULT '',
    `MODEL_RATIO`       text    NOT NULL DEFAULT '',
    `MODEL_IS_REPORT`   text    NOT NULL DEFAULT '',
    `MODEL_USER_DEFINE` text    NOT NULL DEFAULT '',
    `RES1`              text    NOT NULL DEFAULT '',
    `RES2`              text    NOT NULL DEFAULT '',
    `RES3`              text    NOT NULL DEFAULT ''
);


CREATE TABLE `IOT_DEVICE_TABLE`
(
    `ID`                integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    `PARENT_DEVICE_ID`  text    default '',
    `NODE_ID`           text    default '',
    `DEVICE_ID`         text    default '',
    `TOKEN`             text    default '',
    `NAME`              text    default '',
    `CODE`              text    default '',
    `DESCRIPTION`       text    default '',
    `MANUFACTURER_ID`   text    default '',
    `MODEL`             text    default '',
    `PRODUCT_ID`        text    default '',
    `FW_VERSION`        integer default -1,
    `SW_VERSION`        integer default -1,
    `IOT_VERSION`       integer default -1,
    `WRITE_TIME`        integer default -1,
    `STATUS`            integer default -1,
    `EXTENSION_INFO`    text    default '',
    `APP_NAME`          text    default '',
    `DEVICE_ADDRESS`    text    default '',
    `ONLINE_FLAG`       text    default '',
    `SLAVE`             text    default '',
    `MANUFACTURER_NAME` text    default '',
    `DEVICE_PORT`       text    default '',
    `DEV`               text    default '',
    `PROTOCOL_TYPE`     text    default '',
    `IS_REPORT`         text    default ''
);


CREATE TABLE `IOT_MQTT_LOG`
(
    `ID`       integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    `TOKEN`    text    default '',
    `CODE`     text    default '',
    `TOPIC`    text    default '',
    `MESSAGE`  text    default '',
    `VERSION`  integer default -1,
    `LOG_TIME` integer default -1
);


-- INSERT INTO IOT_DEVICE_TABLE `(`ID`, `PARENT_DEVICE_ID`, `NODE_ID`, `DEVICE_ID`, `TOKEN`, `NAME`, `CODE`, `DESCRIPTION`, `MANUFACTURER_ID`, `MODEL`, `PRODUCT_ID`, `FW_VERSION`, `SW_VERSION`, `WRITE_TIME`, `STATUS`, `EXTENSION_INFO`) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
-- UPDATE `IOT_DEVICE_TABLE` SET `PARENT_DEVICE_ID` = ?, `NODE_ID` = ?, `DEVICE_ID` = ?, `TOKEN` = ?, `NAME` = ?, `CODE` = ?, `DESCRIPTION` = ?, `MANUFACTURER_ID` = ?, `MODEL` = ?, `PRODUCT_ID` = ?, `FW_VERSION` = ?, `SW_VERSION` = ?, `WRITE_TIME` = ?, `STATUS` = ?, `EXTENSION_INFO` = ? WHERE `ID` = ?;
-- UPDATE "IOT_DEVICE_TABLE" SET "PARENT_DEVICE_ID" = '', "NODE_ID" = '', "DEVICE_ID" = '', "TOKEN" = '', "NAME" = '', "CODE" = '', "DESCRIPTION" = '', "MANUFACTURER_ID" = '', "MODEL" = '', "PRODUCT_ID" = '', "FW_VERSION" = -1, "SW_VERSION" = -1, "WRITE_TIME" = -1, "STATUS" = -1, "EXTENSION_INFO" = '' WHERE "ID" = 1;
-- SELECT `ID`, `PARENT_DEVICE_ID`, `NODE_ID`, `DEVICE_ID`, `TOKEN`, `NAME`, `CODE`, `DESCRIPTION`, `MANUFACTURER_ID`, `MODEL`, `PRODUCT_ID`, `FW_VERSION`, `SW_VERSION`, `WRITE_TIME`, `STATUS`, `EXTENSION_INFO` FROM `IOT_DEVICE_TABLE`;

-- INSERT INTO `DEVICE_SYNC_LOG`(`ID`, `TOKEN`, `CODE`, `TOPIC`, `MESSAGE`, `VERSION`, `LOG_TIME`) VALUES (1, '', '', '', '', -1, -1);
-- UPDATE "DEVICE_SYNC_LOG" SET "TOKEN" = 'a', "CODE" = '', "TOPIC" = '', "MESSAGE" = '', "VERSION" = -1, "LOG_TIME" = -1 WHERE "ID" = 1;
-- SELECT `ID`, `TOKEN`, `CODE`, `TOPIC`, `MESSAGE`, `VERSION`, `LOG_TIME` FROM `DEVICE_SYNC_LOG`


CREATE TABLE `IOT_APP_TABLE`
(
    `ID`                integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    `APP_NAME`          text default '',
    `PROTOCOL_TYPE`     int  default -1,
    `COMMUNICATION_WAY` int  default -1,
    `SERIAL_PORT_NAME`  text default '',
    `SERIAL_BAUD_RATE`  int  default -1,
    `SERIAL_DATA_BIT`   int  default -1,
    `SERIAL_STOP_BIT`   int  default -1,
    `SERIAL_CHECK`      int  default -1
);


CREATE TABLE `IOT_REG_TABLE`
(
    `ID`        integer NOT NULL PRIMARY KEY AUTOINCREMENT,
    `DEVICE_ID` text default '',
    `OBJECT_ID` text default '',
    `CONTENT`   text default ''
);

INSERT INTO `IOT_DEVICE_TABLE`(`ID`, `PARENT_DEVICE_ID`, `NODE_ID`, `DEVICE_ID`, `TOKEN`, `NAME`, `CODE`, `DESCRIPTION`,
                               `MANUFACTURER_ID`, `MODEL`, `PRODUCT_ID`, `FW_VERSION`, `SW_VERSION`, `IOT_VERSION`,
                               `WRITE_TIME`, `STATUS`, `EXTENSION_INFO`, `APP_NAME`, `DEVICE_ADDRESS`, `ONLINE_FLAG`,
                               `SLAVE`)
VALUES (1, '1', '', '', '', '', '', '', '', '', '', -1, -1, -1, -1, -1, '', '', '', '', '');

