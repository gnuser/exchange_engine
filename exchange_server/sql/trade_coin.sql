
CREATE TABLE `kafka_deals` (
  `offset` int(11) NOT NULL,
  `time` bigint(20) DEFAULT NULL,
  `market` varchar(50) DEFAULT NULL,
  `ask_id` bigint(20) DEFAULT NULL,
  `bid_id` bigint(20) DEFAULT NULL,
  `ask_user_id` bigint(20) DEFAULT NULL,
  `bid_user_id` bigint(20) DEFAULT NULL,
  `price` decimal(30,9) DEFAULT NULL,
  `amount` decimal(30,8) DEFAULT NULL,
  `ask_fee` decimal(30,16) DEFAULT NULL,
  `bid_fee` decimal(30,16) DEFAULT NULL,
  `side` int(11) DEFAULT NULL,
  `id` bigint(20) DEFAULT NULL,
  `stock` char(10) DEFAULT NULL,
  `money` varchar(10) DEFAULT NULL,
  `ask_fee_rate` decimal(30,8) DEFAULT NULL,
  `bid_fee_rate` decimal(30,8) DEFAULT NULL,
  `ask_token` varchar(30) NOT NULL,
  `ask_discount` decimal(30,4) NOT NULL,
  `ask_token_rate` decimal(30,8) NOT NULL,
  `ask_asset_rate` decimal(30,8) NOT NULL,
  `ask_deal_token` decimal(30,16) NOT NULL,
  `bid_token` varchar(30) NOT NULL,
  `bid_discount` decimal(30,4) NOT NULL,
  `bid_token_rate` decimal(30,8) NOT NULL,
  `bid_asset_rate` decimal(30,8) NOT NULL,
  `bid_deal_token` decimal(30,16) NOT NULL,
  PRIMARY KEY (`offset`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;


CREATE TABLE `kafka_orders` (
  `offset` bigint(20) NOT NULL,
  `event` tinyint(4) NOT NULL,
  `money` varchar(50) COLLATE utf8_bin NOT NULL,
  `stock` varchar(50) COLLATE utf8_bin NOT NULL,
  `deal_money` varchar(50) COLLATE utf8_bin DEFAULT NULL,
  `id` bigint(20) NOT NULL,
  `user` bigint(20) NOT NULL,
  `deal_fee` varchar(50) COLLATE utf8_bin DEFAULT NULL,
  `market` varchar(50) COLLATE utf8_bin DEFAULT NULL,
  `side` tinyint(4) NOT NULL,
  `type` tinyint(4) NOT NULL,
  `taker_fee` varchar(50) COLLATE utf8_bin DEFAULT NULL,
  `price` varchar(50) COLLATE utf8_bin DEFAULT NULL,
  `ctime` bigint(20) DEFAULT NULL,
  `mtime` bigint(20) DEFAULT NULL,
  `left` varchar(50) COLLATE utf8_bin DEFAULT NULL,
  `amount` varchar(50) COLLATE utf8_bin DEFAULT NULL,
  `deal_stock` varchar(50) COLLATE utf8_bin DEFAULT NULL,
  `source` varchar(50) COLLATE utf8_bin DEFAULT NULL,
  `token` varchar(30) COLLATE utf8_bin NOT NULL,
  `discount` decimal(30,4) NOT NULL,
  `token_rate` decimal(30,8) NOT NULL,
  `asset_rate` decimal(30,8) NOT NULL,
  `deal_token` decimal(30,16) NOT NULL,
  PRIMARY KEY (`offset`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;


CREATE TABLE `system_coin_type` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `short_name` varchar(16) COLLATE utf8_bin DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;


CREATE TABLE `system_trade_type` (
  `id` int(11) unsigned NOT NULL AUTO_INCREMENT,
  `buy_coin_id` int(11) unsigned DEFAULT NULL,
  `sell_coin_id` int(11) unsigned DEFAULT NULL,
  `min_count` decimal(24,4) DEFAULT NULL,
  `status` int(11) unsigned DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_bin;


