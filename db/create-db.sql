-- case sensitive: http://stackoverflow.com/questions/4879846/how-to-configure-mysql-to-be-case-sensitive
CREATE DATABASE IF NOT EXISTS twitter CHARACTER SET utf8 COLLATE utf8_bin;

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
