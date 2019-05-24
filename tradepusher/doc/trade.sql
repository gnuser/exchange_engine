CREATE DATABASE IF NOT EXISTS `tradepusher`;
USE `tradepusher`;

CREATE TABLE `kafka_offset` (
  `id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT '主键',
  `offset` bigint(20) NOT NULL COMMENT 'kafka offset',
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

