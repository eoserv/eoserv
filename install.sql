
CREATE TABLE IF NOT EXISTS `accounts`
(
	`username`   VARCHAR(16) NOT NULL,
	`password`   CHAR(64)    NOT NULL,
	`fullname`   VARCHAR(64) NOT NULL,
	`location`   VARCHAR(64) NOT NULL,
	`email`      VARCHAR(64) NOT NULL,
	`computer`   VARCHAR(64) NOT NULL,
	`hdid`       INTEGER     NOT NULL,
	`regip`      VARCHAR(15) NOT NULL,
	`lastip`     VARCHAR(15)          DEFAULT NULL,
	`created`    INTEGER     NOT NULL,
	`lastused`   INTEGER              DEFAULT NULL,

	PRIMARY KEY (`username`)
);

CREATE TABLE IF NOT EXISTS `characters`
(
	`name`        VARCHAR(16) NOT NULL,
	`account`     VARCHAR(16)          DEFAULT NULL,
	`title`       VARCHAR(32)          DEFAULT NULL,
	`home`        VARCHAR(32)          DEFAULT NULL,
	`fiance`      VARCHAR(16)          DEFAULT NULL,
	`partner`     VARCHAR(16)          DEFAULT NULL,
	`admin`       INTEGER     NOT NULL DEFAULT 0,
	`class`       INTEGER     NOT NULL DEFAULT 0,
	`gender`      INTEGER     NOT NULL DEFAULT 0,
	`race`        INTEGER     NOT NULL DEFAULT 0,
	`hairstyle`   INTEGER     NOT NULL DEFAULT 0,
	`haircolor`   INTEGER     NOT NULL DEFAULT 0,
	`map`         INTEGER     NOT NULL DEFAULT 192,
	`x`           INTEGER     NOT NULL DEFAULT 7,
	`y`           INTEGER     NOT NULL DEFAULT 6,
	`direction`   INTEGER     NOT NULL DEFAULT 2,
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
	`statpoints`  INTEGER     NOT NULL DEFAULT 0,
	`skillpoints` INTEGER     NOT NULL DEFAULT 0,
	`karma`       INTEGER     NOT NULL DEFAULT 1000,
	`sitting`     INTEGER     NOT NULL DEFAULT 0,
	`bankmax`     INTEGER     NOT NULL DEFAULT 0,
	`goldbank`    INTEGER     NOT NULL DEFAULT 0,
	`usage`       INTEGER     NOT NULL DEFAULT 0,
	`inventory`   TEXT        NOT NULL,
	`bank`        TEXT        NOT NULL,
	`paperdoll`   TEXT        NOT NULL,
	`spells`      TEXT        NOT NULL,
	`guild`       CHAR(3)              DEFAULT NULL,
	`guild_rank`  INTEGER              DEFAULT NULL,
	`quest`       TEXT        NOT NULL,
	`vars`        TEXT        NOT NULL,

	PRIMARY KEY (`name`)
);

CREATE TABLE IF NOT EXISTS `guilds`
(
	`tag`         CHAR(3)     NOT NULL,
	`name`        VARCHAR(32) NOT NULL,
	`description` TEXT        NOT NULL,
	`created`     INTEGER     NOT NULL,
	`ranks`       TEXT        NOT NULL,
	`bank`        INTEGER     NOT NULL DEFAULT 0,

	PRIMARY KEY (`tag`),
	UNIQUE      (`name`)
);

CREATE TABLE IF NOT EXISTS `bans`
(
	`ip`       INTEGER              DEFAULT NULL,
	`hdid`     INTEGER              DEFAULT NULL,
	`username` VARCHAR(16)          DEFAULT NULL,
	`setter`   VARCHAR(16)          DEFAULT NULL,
	`expires`  INTEGER     NOT NULL DEFAULT 0
);

CREATE INDEX `character_account_index` ON `characters` (`account`);
CREATE INDEX `character_guild_index` ON `characters` (`guild`);
CREATE INDEX `ban_ip_index` ON `bans` (`ip`);
CREATE INDEX `ban_hdid_index` ON `bans` (`hdid`);
CREATE INDEX `ban_username_index` ON `bans` (`username`);
