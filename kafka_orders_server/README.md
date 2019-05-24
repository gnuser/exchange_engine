This is a project for KAFKA messsage insert into MySQL db.

新增交易挖矿交易统计查询以及费率查询表

	CREATE TABLE `kafka_deals` (
			`offset` INT(11) NULL DEFAULT NULL,
			`time` BIGINT(20) NULL DEFAULT NULL,
			`market` VARCHAR(50) NULL DEFAULT NULL,
			`ask_id` BIGINT(20) NULL DEFAULT NULL,
			`bid_id` BIGINT(20) NULL DEFAULT NULL,
			`ask_user_id` BIGINT(20) NULL DEFAULT NULL,
			`bid_user_id` BIGINT(20) NULL DEFAULT NULL,
			`price` VARCHAR(50) NULL DEFAULT NULL,
			`amount` VARCHAR(256) NULL DEFAULT NULL,
			`ask_fee` VARCHAR(50) NULL DEFAULT NULL,
			`bid_fee` VARCHAR(50) NULL DEFAULT NULL,
			`side` INT(11) NULL DEFAULT NULL,
			`id` BIGINT(20) NOT NULL,
			`stock` CHAR(10) NULL DEFAULT NULL,
			`money` VARCHAR(10) NULL DEFAULT NULL,
			`ask_fee_rate` VARCHAR(50) NULL DEFAULT NULL,
			`bid_fee_rate` VARCHAR(50) NULL DEFAULT NULL,
			PRIMARY KEY (`id`)
			)
	COLLATE='latin1_swedish_ci'
	ENGINE=InnoDB
	;

	表的字段解析

	offset ： kafka 消息位置
	time ：订单完成时间戳 （精确到秒）
	market：交易对
	ask_id：卖单订单id
	bid_id：买单订单id
	ask_user_id：卖单用户id
	bid_user_id：买单用户id
	price：成交价格
	amount：成交数量
	ask_fee：卖单手续费
	bid_fee：买单手续费
	side：买/卖
	id：主键
	stock：左资产名称
	money：右资产名称
	ask_fee_rate：左资产汇率（右资产是CNY为0）
	bid_fee_rate：右资产汇率（右资产是CNY为0）
