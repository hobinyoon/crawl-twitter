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

CREATE TABLE IF NOT EXISTS twitter.followers (
	id BIGINT NOT NULL,
	followers TEXT NOT NULL,
	PRIMARY KEY (id));

CREATE TABLE IF NOT EXISTS twitter.friends (
	id BIGINT NOT NULL,
	friends TEXT NOT NULL,
	PRIMARY KEY (id));

CREATE TABLE IF NOT EXISTS twitter.child_ids_to_crawl (
	id BIGINT,
	status CHAR(1) NOT NULL DEFAULT 'u',	-- u: unused, c: crawled
	PRIMARY KEY (id));
