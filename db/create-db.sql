-- case sensitive: http://stackoverflow.com/questions/4879846/how-to-configure-mysql-to-be-case-sensitive
CREATE DATABASE IF NOT EXISTS twitter CHARACTER SET utf8 COLLATE utf8_bin;

CREATE TABLE IF NOT EXISTS twitter.credentials (
	token VARCHAR(100),
	token_secret VARCHAR(100) NOT NULL,
	consumer_key VARCHAR(100) NOT NULL,
	consumer_secret VARCHAR(100) NOT NULL,
	for_stream BOOL NOT NULL,
	status CHAR(1),	-- v: valid, i: invalid, null: unknown
	last_used TIMESTAMP NULL DEFAULT NULL,	-- permit null
	last_rate_limited TIMESTAMP NULL DEFAULT NULL,	-- permit null
	PRIMARY KEY (token));

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
	text VARCHAR(450) NOT NULL,	-- give some cushion for utf8
	PRIMARY KEY (id),
	INDEX (uid),
	INDEX (rt_id));

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
