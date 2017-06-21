-- case sensitive:
--   http://stackoverflow.com/questions/4879846/how-to-configure-mysql-to-be-case-sensitive
-- CREATE DATABASE IF NOT EXISTS twitter3 CHARACTER SET utf8 COLLATE utf8_bin;

-- 4-byte UTF8. It is case sensitive too. There are some characters that causes problems like:
--   java.sql.SQLException: Incorrect string value: '\xF0\x9F\x91\x86wi...' for column 'text'
--   http://stackoverflow.com/questions/13653712/java-sql-sqlexception-incorrect-string-value-xf0-x9f-x91-xbd-xf0-x9f
CREATE DATABASE IF NOT EXISTS twitter3 CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;

CREATE TABLE IF NOT EXISTS twitter3.credentials (
	email VARCHAR (50) NOT NULL,
	email_pw VARCHAR (50) NOT NULL,
	twitter_pw VARCHAR (50) NOT NULL,
	twitter_username VARCHAR (50) NOT NULL,
	twitter_app_name VARCHAR (50) NOT NULL,
	token VARCHAR(100),
	token_secret VARCHAR(100) NOT NULL,
	consumer_key VARCHAR(100) NOT NULL,
	consumer_secret VARCHAR(100) NOT NULL,
	for_stream BOOL NOT NULL,
	status CHAR(1),	-- V: valid, I: invalid, null: unknown
	check_out_at TIMESTAMP NULL DEFAULT NULL,	-- permit null, no autoupdate
	check_out_ip VARCHAR(20) DEFAULT NULL,
	num_reqs_before_rate_limited INT DEFAULT 0,
	rate_limited_at TIMESTAMP NULL DEFAULT NULL,
	sec_until_retry INT DEFAULT NULL,
	PRIMARY KEY (token));

/*
-- credential insert template
INSERT INTO twitter3.credentials
(email, email_pw, twitter_pw, twitter_username, twitter_app_name, token, token_secret, consumer_key, consumer_secret)
VALUE (
'', -- email
'', -- email_pw
'', -- twitter_pw
'', -- twitter_username
'', -- twitter_app_name
'', -- token
'', -- token_secret
'', -- consumer_key
'' -- consumer_secret
);
*/

CREATE TABLE IF NOT EXISTS twitter3.cred_auth_history (
	time_ TIMESTAMP NULL DEFAULT NULL,
	status CHAR(1) NOT NULL,	-- S: auth succeeded, F: auth failed
	token VARCHAR(100) NOT NULL,
	ip VARCHAR(20) NOT NULL,
	INDEX (time_),
	INDEX (status));

-- When selecting, rows with 'UP' and 'UC' has priority over those with 'U',
-- which helps build bigger fan-out.
CREATE TABLE IF NOT EXISTS twitter3.users (
	id BIGINT,
	gen INT NOT NULL,	-- generation
	added_at TIMESTAMP NULL DEFAULT NULL,
	-- crawled_at is useful when playing with the Tweet (YouTube request)
	-- density.
	crawled_at TIMESTAMP NULL DEFAULT NULL,
	status VARCHAR(2) NOT NULL, -- U(uncrawled seeded)
															-- UC(uncrawled child), UP(uncrawled parent)
															-- C(crawled)
															-- P(unauthorized. the user's tweets are protected)
															-- NF(the user is not found)
	check_out_at TIMESTAMP NULL DEFAULT NULL,
	check_out_ip VARCHAR(20),
	PRIMARY KEY (id),
	INDEX (status),
	INDEX (crawled_at),
	INDEX (status, crawled_at),
	INDEX (status, added_at),
	INDEX (gen),
	INDEX (status, gen),
	INDEX (status, gen, added_at),
	INDEX (check_out_at),
	INDEX (check_out_at, check_out_ip));

CREATE TABLE IF NOT EXISTS twitter3.tweets (
	id BIGINT,
	uid BIGINT NOT NULL,
	created_at TIMESTAMP NULL DEFAULT NULL,
	geo_lati DOUBLE NOT NULL,
	geo_longi DOUBLE NOT NULL,
	youtube_video_id VARCHAR(22) NOT NULL,
	hashtags VARCHAR(450) NOT NULL,
	rt_id BIGINT NOT NULL,	-- -1 when not retweet
	rt_uid BIGINT NOT NULL,	-- -1 when not retweet
	text VARCHAR(450) NOT NULL,	-- give some cushion for utf8
	child_uids VARCHAR(2200),	-- child tweets (retweet of this tweets)' uids
														-- max 2200 = (10 + 1) * 200
														--   example uid 2740470659
														--               0123456789
	PRIMARY KEY (id),
	INDEX (uid),
	INDEX (created_at),
	INDEX (rt_id),
	INDEX (rt_uid),
	INDEX (youtube_video_id),
	INDEX (child_uids));

CREATE TABLE IF NOT EXISTS twitter3.meta (
	k VARCHAR(30) NOT NULL,
	v_int INT,
	PRIMARY KEY (k));

-- -- case sensitive: http://stackoverflow.com/questions/4879846/how-to-configure-mysql-to-be-case-sensitive
-- CREATE DATABASE IF NOT EXISTS twitter CHARACTER SET utf8 COLLATE utf8_bin;
--
-- CREATE TABLE IF NOT EXISTS twitter.credentials (
-- 	email VARCHAR (50) NOT NULL,
-- 	email_pw VARCHAR (50) NOT NULL,
-- 	twitter_pw VARCHAR (50) NOT NULL,
-- 	twitter_username VARCHAR (50) NOT NULL,
-- 	twitter_app_name VARCHAR (50) NOT NULL,
-- 	token VARCHAR(100),
-- 	token_secret VARCHAR(100) NOT NULL,
-- 	consumer_key VARCHAR(100) NOT NULL,
-- 	consumer_secret VARCHAR(100) NOT NULL,
-- 	for_stream BOOL NOT NULL,
-- 	status CHAR(1),	-- V: valid, I: invalid, null: unknown
-- 	last_check_out TIMESTAMP NULL DEFAULT NULL,	-- permit null, no autoupdate
-- 	num_reqs_before_rate_limited INT DEFAULT 0,
-- 	last_rate_limited TIMESTAMP NULL DEFAULT NULL,
-- 	sec_until_retry INT DEFAULT NULL,
-- 	rate_limited_ip VARCHAR(20) DEFAULT NULL,
-- 	PRIMARY KEY (token));
--
-- CREATE TABLE IF NOT EXISTS twitter.cred_auth_history (
-- 	time_ TIMESTAMP NULL DEFAULT NULL,
-- 	status CHAR(1) NOT NULL,	-- S: auth succeeded, F: auth failed
-- 	token VARCHAR(100) NOT NULL,
-- 	ip VARCHAR(20) NOT NULL,
-- 	INDEX (time_),
-- 	INDEX (status));
--
-- -- When selecting, rows with 'UP' and 'UC' has priority over those with 'U',
-- -- which helps build bigger fan-out.
-- CREATE TABLE IF NOT EXISTS twitter.uids_to_crawl (
-- 	id BIGINT,
-- 	added_at TIMESTAMP NULL DEFAULT NULL,
-- 	crawled_at TIMESTAMP NULL DEFAULT NULL,
-- 	status VARCHAR(2) NOT NULL, -- U(uncrawled seeded)
-- 															-- UC(uncrawled child), UP(uncrawled parent)
-- 															-- C(crawled)
-- 															-- P(unauthorized. the user's tweets are protected)
-- 															-- NF(the user is not found)
-- 	PRIMARY KEY (id),
-- 	INDEX (status),
-- 	INDEX (crawled_at),
-- 	INDEX (status, crawled_at),
-- 	INDEX (status, added_at));
--
-- CREATE TABLE IF NOT EXISTS twitter.tweets (
-- 	id BIGINT,
-- 	uid BIGINT NOT NULL,
-- 	created_at TIMESTAMP NULL DEFAULT NULL,
-- 	geo_lati DOUBLE NOT NULL,
-- 	geo_longi DOUBLE NOT NULL,
-- 	youtube_video_id VARCHAR(22) NOT NULL,
-- 	hashtags VARCHAR(450) NOT NULL,
-- 	rt_id BIGINT NOT NULL,	-- -1 when not retweet
-- 	rt_uid BIGINT NOT NULL,	-- -1 when not retweet
-- 	text VARCHAR(450) NOT NULL,	-- give some cushion for utf8
-- 	child_uids VARCHAR(2200),	-- child tweets (retweet of this tweets)' uids
-- 														-- max 2200 = (10 + 1) * 200
-- 														--   example uid 2740470659
-- 														--               0123456789
-- 	PRIMARY KEY (id),
-- 	INDEX (uid),
-- 	INDEX (created_at),
-- 	INDEX (rt_id),
-- 	INDEX (rt_uid),
-- 	INDEX (youtube_video_id),
-- 	INDEX (child_uids));
--
-- -- Get (up to 100) user IDs of retweets of a tweet.
-- -- https://dev.twitter.com/docs/api/1.1/get/statuses/retweeters/ids
-- -- need a separate table to store the user IDs.
-- -- may want to give priority to those (child) users when crawling child tweets
--
-- CREATE TABLE IF NOT EXISTS twitter.followers (
-- 	id BIGINT NOT NULL,
-- 	followers TEXT NOT NULL,
-- 	PRIMARY KEY (id));
--
-- CREATE TABLE IF NOT EXISTS twitter.friends (
-- 	id BIGINT NOT NULL,
-- 	friends TEXT NOT NULL,
-- 	PRIMARY KEY (id));
