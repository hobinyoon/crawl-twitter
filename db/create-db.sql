-- case sensitive: http://stackoverflow.com/questions/4879846/how-to-configure-mysql-to-be-case-sensitive
CREATE DATABASE IF NOT EXISTS twitter CHARACTER SET utf8 COLLATE utf8_bin;

CREATE TABLE IF NOT EXISTS twitter.credentials (
	token VARCHAR(100),
	token_secret VARCHAR(100) NOT NULL,
	consumer_key VARCHAR(100) NOT NULL,
	consumer_secret VARCHAR(100) NOT NULL,
	for_stream BOOL NOT NULL,
	status CHAR(1),	-- V: valid, I: invalid, null: unknown
	last_check_out TIMESTAMP NULL DEFAULT NULL,	-- NULL means permit null
	num_reqs_before_rate_limited INT DEFAULT 0,
	last_rate_limited TIMESTAMP NULL DEFAULT NULL,
	sec_until_retry INT DEFAULT NULL,
	rate_limited_ip VARCHAR(20) DEFAULT NULL,
	PRIMARY KEY (token));

CREATE TABLE IF NOT EXISTS twitter.cred_auth_history (
	time_ TIMESTAMP NOT NULL,
	status CHAR(1) NOT NULL,	-- S: auth succeeded, F: auth failed
	token VARCHAR(100) NOT NULL,
	INDEX (time_),
	INDEX (status));

-- When selecting, rows with 'UP' and 'UC' has priority over those with 'U',
-- which helps build bigger fan-out.
CREATE TABLE IF NOT EXISTS twitter.uids_to_crawl (
	id BIGINT,
	crawled_at TIMESTAMP NOT NULL,
	status VARCHAR(2) NOT NULL,	-- UP(uncrawled parent), UC(uncrawled child), U(uncrawled seed), C(crawled),
															-- I(invalid, such as non-existent user)
	PRIMARY KEY (id),
	INDEX (status, crawled_at));

CREATE TABLE IF NOT EXISTS twitter.tweets (
	id BIGINT,
	uid BIGINT NOT NULL,
	created_at TIMESTAMP NOT NULL,
	geo_lati DOUBLE NOT NULL,
	geo_longi DOUBLE NOT NULL,
	youtube_link VARCHAR(1024) NOT NULL,
	hashtags VARCHAR(450) NOT NULL,
	rt_id BIGINT NOT NULL,	-- -1 when not retweet
	rt_uid BIGINT NOT NULL,	-- -1 when not retweet
	text VARCHAR(450) NOT NULL,	-- give some cushion for utf8
	PRIMARY KEY (id),
	INDEX (uid),
	INDEX (rt_id),
	INDEX (rt_uid));

-- Get (up to 100) user IDs of retweets of a tweet.
-- https://dev.twitter.com/docs/api/1.1/get/statuses/retweeters/ids
-- need a separate table to store the user IDs.
-- may want to give priority to those (child) users when crawling child tweets

CREATE TABLE IF NOT EXISTS twitter.followers (
	id BIGINT NOT NULL,
	followers TEXT NOT NULL,
	PRIMARY KEY (id));

CREATE TABLE IF NOT EXISTS twitter.friends (
	id BIGINT NOT NULL,
	friends TEXT NOT NULL,
	PRIMARY KEY (id));
