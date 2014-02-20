CREATE TABLE `reports`
(
	`reporter` VARCHAR(16) NOT NULL,
	`reported` VARCHAR(16) NOT NULL,
	`reason`   TEXT,
	`time`     INTEGER     NOT NULL,
	`chat_log` TEXT        NOT NULL,
	
	PRIMARY KEY (`reporter`, `reported`, `time`)
);
