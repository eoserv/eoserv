
CREATE TABLE IF NOT EXISTS `accounts`
(
	`username`   VARCHAR(16) NOT NULL,
	`password`   CHAR(64)    NOT NULL,
	`fullname`   VARCHAR(64),
	`location`   VARCHAR(64),
	`email`      VARCHAR(64),
	`computer`   VARCHAR(64),
	`hdid`       CHAR(10),
	`regip`      VARCHAR(15),
	`lastip`     VARCHAR(15),

	PRIMARY KEY (`username`)
);

CREATE TABLE IF NOT EXISTS `characters`
(
	`name`        VARCHAR(12) NOT NULL,
	`account`     VARCHAR(16)          DEFAULT NULL,
	`title`       VARCHAR(32),
	`home`        VARCHAR(32),
	`partner`     VARCHAR(16)          DEFAULT NULL,
	`admin`       INTEGER     NOT NULL DEFAULT 0,
	`class`       INTEGER     NOT NULL DEFAULT 0,
	`gender`      INTEGER     NOT NULL DEFAULT 0,
	`race`        INTEGER     NOT NULL DEFAULT 0,
	`hairstyle`   INTEGER     NOT NULL DEFAULT 0,
	`haircolor`   INTEGER     NOT NULL DEFAULT 0,
	`map`         INTEGER     NOT NULL DEFAULT 192,
	`x`           INTEGER     NOT NULL DEFAULT 6,
	`y`           INTEGER     NOT NULL DEFAULT 6,
	`direction`   INTEGER     NOT NULL DEFAULT 2,
	`spawnmap`    INTEGER     NOT NULL DEFAULT 192,
	`spawnx`      INTEGER     NOT NULL DEFAULT 6,
	`spawny`      INTEGER     NOT NULL DEFAULT 6,
	`level`       INTEGER     NOT NULL DEFAULT 0,
	`exp`         INTEGER     NOT NULL DEFAULT 0,
	`hp`          INTEGER     NOT NULL DEFAULT 10,
	`tp`          INTEGER     NOT NULL DEFAULT 10,
	`str`         INTEGER     NOT NULL DEFAULT 0,
	`int`         INTEGER     NOT NULL DEFAULT 0,
	`wis`         INTEGER     NOT NULL DEFAULT 0,
	`agi`         INTEGER     NOT NULL DEFAULT 0,
	`con`         INTEGER     NOT NULL DEFAULT 0,
	`cha`         INTEGER     NOT NULL DEFAULT 0,
	`statpoints`  INTEGER     NOT NULL DEFAULT 6,
	`skillpoints` INTEGER     NOT NULL DEFAULT 6,
	`karma`       INTEGER     NOT NULL DEFAULT 1000,
	`sitting`     INTEGER     NOT NULL DEFAULT 0,
	`bankmax`     INTEGER     NOT NULL DEFAULT 30,
	`goldbank`    INTEGER     NOT NULL DEFAULT 0,
	`usage`       INTEGER     NOT NULL DEFAULT 0,
	`inventory`   TEXT        NOT NULL,
	`bank`        TEXT        NOT NULL,
	`paperdoll`   TEXT        NOT NULL,
	`spells`      TEXT        NOT NULL,
	`guild`       CHAR(3)              DEFAULT NULL,
	
	PRIMARY KEY (`name`),
	INDEX       (`account`),
	INDEX       (`guild`)
);

CREATE TABLE IF NOT EXISTS `guilds`
(
	`tag`         CHAR(3)     NOT NULL,
	`name`        VARCHAR(32) NOT NULL,
	`description` TEXT        NOT NULL,
	
	PRIMARY KEY (`tag`),
	UNIQUE      (`name`)
);
