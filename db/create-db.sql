-- case sensitive:
--   http://stackoverflow.com/questions/4879846/how-to-configure-mysql-to-be-case-sensitive
-- CREATE DATABASE IF NOT EXISTS twitter4 CHARACTER SET utf8 COLLATE utf8_bin;

-- 4-byte UTF8. It is case sensitive too. There are some characters that causes problems like:
--   java.sql.SQLException: Incorrect string value: '\xF0\x9F\x91\x86wi...' for column 'text'
--   http://stackoverflow.com/questions/13653712/java-sql-sqlexception-incorrect-string-value-xf0-x9f-x91-xbd-xf0-x9f
CREATE DATABASE IF NOT EXISTS twitter4 CHARACTER SET utf8mb4 COLLATE utf8mb4_general_ci;

CREATE TABLE IF NOT EXISTS twitter4.credentials (
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
INSERT INTO twitter4.credentials
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

CREATE TABLE IF NOT EXISTS twitter4.cred_auth_history (
	time_ TIMESTAMP NULL DEFAULT NULL,
	status CHAR(1) NOT NULL,	-- S: auth succeeded, F: auth failed
	token VARCHAR(100) NOT NULL,
	ip VARCHAR(20) NOT NULL,
	INDEX (time_),
	INDEX (status));

-- When selecting, rows with 'UP' and 'UC' has priority over those with 'U',
-- which helps build bigger fan-out.
CREATE TABLE IF NOT EXISTS twitter4.users (
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

CREATE TABLE IF NOT EXISTS twitter4.tweets (
	id BIGINT,
	uid BIGINT NOT NULL,
	created_at TIMESTAMP NULL DEFAULT NULL,
	geo_lati DOUBLE NOT NULL,
	geo_longi DOUBLE NOT NULL,
	youtube_video_id VARCHAR(22) NOT NULL,
	PRIMARY KEY (id),
	INDEX (uid),
	INDEX (created_at),
	INDEX (youtube_video_id)
	);

CREATE TABLE IF NOT EXISTS twitter4.meta (
	k VARCHAR(30) NOT NULL,
	v_int INT,
	PRIMARY KEY (k));
