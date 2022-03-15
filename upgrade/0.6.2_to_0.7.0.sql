CREATE TABLE `reports`
(
	`reporter` VARCHAR(16) NOT NULL,
	`reported` VARCHAR(16) NOT NULL,
	`reason`   TEXT,
	`time`     INTEGER     NOT NULL,
	`chat_log` TEXT        NOT NULL,

	PRIMARY KEY (`reporter`, `reported`, `time`)
);
ALTER TABLE `characters`
    ADD COLUMN `hidden` INTEGER DEFAULT 0 NOT NULL,
    ADD COLUMN `nointeract` INTEGER DEFAULT 0 NOT NULL,
    ADD COLUMN `guild_rank_string` VARCHAR(16) DEFAULT NULL;
