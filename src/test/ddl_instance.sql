/* This file was generated by ODB, object-relational mapping (ORM)
 * compiler for C++.
 */

DROP TABLE IF EXISTS `TestLog_images`;

DROP TABLE IF EXISTS `TestLog`;

DROP TABLE IF EXISTS `TestLogImage`;

DROP TABLE IF EXISTS `LfiDaeSlowVoltage_columns`;

DROP TABLE IF EXISTS `LfiDaeSlowVoltage`;

CREATE TABLE `LfiDaeSlowVoltage` (
  `name` TEXT NOT NULL,
  `version` SMALLINT NOT NULL,
  `dbUserId` TEXT NOT NULL,
  `creationDate` DATETIME,
  `das_id` BIGINT NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `relative_path` TEXT NOT NULL,
  `runId` TEXT NOT NULL,
  `startTime` BIGINT NOT NULL,
  `endTime` BIGINT NOT NULL,
  `apid` SMALLINT NOT NULL,
  `type` TINYINT NOT NULL,
  `subtype` TINYINT NOT NULL,
  `pi1Val` SMALLINT NOT NULL,
  `pi2Val` SMALLINT NOT NULL)
 ENGINE=InnoDB;

CREATE TABLE `LfiDaeSlowVoltage_columns` (
  `object_id` BIGINT NOT NULL,
  `key` TEXT NOT NULL,
  `value_size` BIGINT NOT NULL,
  `value_type` TEXT NOT NULL,
  `value_fname` TEXT NOT NULL,

  CONSTRAINT `LfiDaeSlowVoltage_columns_object_id_fk`
    FOREIGN KEY (`object_id`)
    REFERENCES `LfiDaeSlowVoltage` (`das_id`)
    ON DELETE CASCADE)
 ENGINE=InnoDB;

CREATE INDEX `object_id_i`
  ON `LfiDaeSlowVoltage_columns` (`object_id`);

CREATE TABLE `TestLogImage` (
  `name` TEXT NOT NULL,
  `version` SMALLINT NOT NULL,
  `dbUserId` TEXT NOT NULL,
  `creationDate` DATETIME,
  `das_id` BIGINT NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `naxis1` INT NOT NULL,
  `naxis2` INT NOT NULL,
  `format` TEXT NOT NULL)
 ENGINE=InnoDB;

CREATE TABLE `TestLog` (
  `name` TEXT NOT NULL,
  `version` SMALLINT NOT NULL,
  `dbUserId` TEXT NOT NULL,
  `creationDate` DATETIME,
  `das_id` BIGINT NOT NULL PRIMARY KEY AUTO_INCREMENT,
  `runId` TEXT NOT NULL,
  `startTime` BIGINT NOT NULL,
  `endTime` BIGINT NOT NULL,
  `log` TEXT NOT NULL)
 ENGINE=InnoDB;

CREATE TABLE `TestLog_images` (
  `object_id` BIGINT NOT NULL,
  `value` BIGINT NOT NULL,

  CONSTRAINT `TestLog_images_object_id_fk`
    FOREIGN KEY (`object_id`)
    REFERENCES `TestLog` (`das_id`)
    ON DELETE CASCADE,

  CONSTRAINT `TestLog_images_value_fk`
    FOREIGN KEY (`value`)
    REFERENCES `TestLogImage` (`das_id`)
 )
 ENGINE=InnoDB;

CREATE INDEX `object_id_i`
  ON `TestLog_images` (`object_id`);

