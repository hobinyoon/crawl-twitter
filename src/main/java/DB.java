package crawltwitter;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.lang.InterruptedException;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.Date;
import com.mysql.jdbc.MysqlErrorNumbers;
import twitter4j.GeoLocation;


public class DB {
	static private Connection _conn_stream_seed_users = null;
	static private Connection _conn_crawl_tweets = null;
	static private PreparedStatement _ps_insert_seed_user = null;
	static private PreparedStatement _ps_insert_tweet = null;
	static private PreparedStatement _ps_set_user_crawled = null;
	static private PreparedStatement _ps_credential_rate_limited = null;

	static public void Init () {
		try {
			_conn_stream_seed_users = DriverManager.getConnection(Conf.db_url, Conf.db_user, Conf.db_pass);
			_conn_stream_seed_users.setAutoCommit(false);
			// This enables one connection read committed writes from the other connection.
			_conn_stream_seed_users.setTransactionIsolation(Connection.TRANSACTION_READ_COMMITTED);
			_conn_crawl_tweets = DriverManager.getConnection(Conf.db_url, Conf.db_user, Conf.db_pass);
			_conn_crawl_tweets.setAutoCommit(false);
			_conn_crawl_tweets.setTransactionIsolation(Connection.TRANSACTION_READ_COMMITTED);

			_ps_insert_seed_user = _conn_stream_seed_users.prepareStatement(
					"INSERT INTO users (id, gen, added_at, status) VALUES (?, -1, NOW(), ?)");
			_ps_insert_tweet = _conn_crawl_tweets.prepareStatement(
					"INSERT INTO tweets "
					+ "(id, uid, created_at, geo_lati, geo_longi, youtube_video_id, hashtags, rt_id, rt_uid, text, child_uids) "
					+ "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
			_ps_set_user_crawled = _conn_crawl_tweets.prepareStatement(
					"INSERT INTO users (id, gen, added_at, crawled_at, status, check_out_at, check_out_ip) "
					+ "VALUES (?, ?, NOW(), NOW(), 'C', NOW(), ?) "
					+ "ON DUPLICATE KEY UPDATE crawled_at=NOW(), gen=VALUES(gen), status='C'");
			_ps_credential_rate_limited = _conn_crawl_tweets.prepareStatement(
					"UPDATE credentials SET rate_limited_at=NOW(), sec_until_retry=(?) WHERE token=(?)");
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("Exception caught: " + e);
			System.exit(-1);
		}
	}

	static void Close() {
		try {
			if (_ps_insert_seed_user != null) _ps_insert_seed_user.close();
			if (_ps_insert_tweet != null) _ps_insert_tweet.close();
			if (_ps_credential_rate_limited != null) _ps_credential_rate_limited.close();

			if (_conn_stream_seed_users != null) _conn_stream_seed_users.close();
			if (_conn_crawl_tweets != null) _conn_crawl_tweets.close();
		} catch (Exception e) {
			StringWriter sw = new StringWriter();
			e.printStackTrace(new PrintWriter(sw));
			StdoutWriter.W(String.format("SQLException: %s\nStack trace: %s", e, sw));
			// Do not System.exit() yet. There are more to be cleaned up.
		}
	}

	// Twitter credential
	static public class TC {
		public String token;
		public String secret;
		public String consumerKey;
		public String consumerSecret;

		public TC(String t, String s, String ck, String cs) {
			token = t;
			secret = s;
			consumerKey = ck;
			consumerSecret = cs;
		}

		public String toString() {
			return token + " " + secret + " " + consumerKey + " " + consumerSecret;
		}
	}

	static TC GetTwitterCredForStream() throws SQLException {
		Statement stmt = null;
		try {
			stmt = _conn_stream_seed_users.createStatement();
			final String q = "SELECT * FROM credentials WHERE for_stream=true and (status is null or status !='I') LIMIT 1";
			ResultSet rs = stmt.executeQuery(q);
			long id = -1;
			if (! rs.next())
				throw new RuntimeException("No record");
			return new TC(
					rs.getString("token"),
					rs.getString("token_secret"),
					rs.getString("consumer_key"),
					rs.getString("consumer_secret"));
		} finally {
			if (stmt != null)
				stmt.close();
		}
	}

	static TC GetTwitterCred() throws SQLException, InterruptedException {
		Statement stmt = null;
		try {
			stmt = _conn_crawl_tweets.createStatement();
			String token = null;
			String token_secret = null;
			String consumer_key = null;
			String consumer_secret = null;

			while (true) {
				// begin tranction
				_conn_crawl_tweets.commit();
				{
					// wait for 1 hour when where has been more than 3 auth failures from this IP in the last hour
					final String q = String.format("SELECT count(*) as fail_cnt FROM cred_auth_history "
						+ "WHERE status = 'F' and TIMESTAMPDIFF(SECOND, time_, NOW()) < 3600 and ip = '%s'", Conf.ip);
					ResultSet rs = stmt.executeQuery(q);
					if (! rs.next())
						throw new RuntimeException("Unexpected");
					long fail_cnt = rs.getLong("fail_cnt");
					if (fail_cnt >= 3) {
						final String q1 = String.format("SELECT time_ FROM cred_auth_history "
							+ "WHERE status = 'F' and ip = '%s' ORDER BY time_ desc LIMIT 1", Conf.ip);
						ResultSet rs1 = stmt.executeQuery(q1);
						if (! rs1.next())
							throw new RuntimeException("Unexpected");
						long wait_milli = 3610000L + rs1.getTimestamp("time_").getTime() - (new Date()).getTime();
						if (wait_milli > 0) {
							StdoutWriter.W(String.format("%d auth failures from IP %s in the last 3600 secs. waiting for %d sec and retrying ...",
										fail_cnt, Conf.ip, wait_milli / 1000));
							Mon.Sleep(wait_milli);
						}
						continue;
					}
				}
				{
					// pick the oldest rate-limited credential
					final String q = String.format(
							"SELECT *, ADDDATE(rate_limited_at, INTERVAL sec_until_retry SECOND) as retry_after "
							+ "FROM credentials "
							+ "WHERE for_stream=false "
							+ "and (status is null or status != 'I') "	// valid one
							+ "and (check_out_at is null or TIMESTAMPDIFF(SECOND, check_out_at, NOW()) > 60) "	// not checked-out in the last 60 secs
							+ "and token not in (select distinct(token) from cred_auth_history where status='F' and TIMESTAMPDIFF(SECOND, time_, NOW()) < %d) "
							+ "order by retry_after "
							+ "LIMIT 1", Conf.cred_auth_fail_retry_wait_sec);
					ResultSet rs = stmt.executeQuery(q);
					long id = -1;
					if (! rs.next()) {
						StdoutWriter.W("No available credentials at this time. retrying in 10 mins ...");
						Mon.Sleep(600000);
						continue;
					}
					if (rs.getTimestamp("retry_after") != null) {
						long wait_milli = rs.getTimestamp("retry_after").getTime() + Conf.cred_rate_limit_wait_cushion_in_milli - (new Date()).getTime();
						if (wait_milli > 0) {
							// StdoutWriter.W(String.format("All credentials are rate-limited. waiting for %d ms and retrying ...", wait_milli));
							Mon.Sleep(wait_milli);
							continue;
						}
					}
					token = rs.getString("token");
					token_secret = rs.getString("token_secret");
					consumer_key = rs.getString("consumer_key");
					consumer_secret = rs.getString("consumer_secret");
				}
				{
					// Check out the credential
					final String q = String.format(
							"UPDATE credentials "
							+ "SET check_out_at=NOW(), check_out_ip='%s', num_reqs_before_rate_limited=0 "
							+ "WHERE token='%s' "
							+ "and (check_out_at is null or TIMESTAMPDIFF(SECOND, check_out_at, NOW()) > 60)",
							Conf.ip, token);
					int rows_updated = stmt.executeUpdate(q);
					if (rows_updated == 1) {
						_conn_crawl_tweets.commit();
						return new TC(token, token_secret, consumer_key, consumer_secret);
					}
				}
			}
		} finally {
			if (stmt != null)
				stmt.close();
		}
	}

	static void CredIncReqMade(String token) throws SQLException {
		Statement stmt = null;
		while (true) {
			try {
				stmt = _conn_crawl_tweets.createStatement();
				final String q = String.format(
						"UPDATE credentials "
						+ "SET num_reqs_before_rate_limited = num_reqs_before_rate_limited + 1 "
						+ "WHERE token='%s'", token);
				int rows_affected = stmt.executeUpdate(q);
				if (rows_affected == 1) {
					_conn_crawl_tweets.commit();
					return;
				}
			} finally {
				if (stmt != null)
					stmt.close();
			}
		}
	}

	static void CredSetRateLimited(String token, int sec_until_reset) throws SQLException {
		_ps_credential_rate_limited.setInt(1, sec_until_reset);
		_ps_credential_rate_limited.setString(2, token);
		_ps_credential_rate_limited.executeUpdate();
		_conn_crawl_tweets.commit();
	}

	static int CredGetNumReqsBeforeRateLimited(String token) throws SQLException {
		final String q = String.format(
				"SELECT num_reqs_before_rate_limited FROM credentials WHERE token='%s'", token);
		Statement stmt = _conn_crawl_tweets.createStatement();
		ResultSet rs = stmt.executeQuery(q);
		if (! rs.next())
			throw new RuntimeException(String.format("Unexpected. token=%s", token));
		return rs.getInt("num_reqs_before_rate_limited");
	}

	static void CredAuthFailed(String token) throws SQLException {
		Statement stmt = null;
		try {
			stmt = _conn_crawl_tweets.createStatement();
			final String q = String.format(
					"INSERT INTO cred_auth_history (time_, status, token, ip) VALUES (NOW(), 'F', '%s', '%s')",
					token, Conf.ip);
			stmt.executeUpdate(q);
			_conn_crawl_tweets.commit();
		} finally {
			if (stmt != null)
				stmt.close();
		}
	}

	static void AddSeedUserToCrawl(long uid) throws SQLException {
		try {
			_ps_insert_seed_user.setLong(1, uid);
			_ps_insert_seed_user.setString(2, "U");	// U: uncrawled
			_ps_insert_seed_user.executeUpdate();
			_conn_stream_seed_users.commit();
			Mon.num_users_to_crawl_streamed_new ++;
		} catch (SQLException e) {
			if (e.getErrorCode() == MysqlErrorNumbers.ER_DUP_ENTRY) {
				StdoutWriter.W(String.format("Dup user: %d", uid));
				Mon.num_users_to_crawl_streamed_dup ++;
			} else
				throw e;
		}
	}

	static void AddParentUserToCrawl(long uid, int gen) throws SQLException {
		Statement stmt = null;
		try {
			String status = null;
			stmt = _conn_crawl_tweets.createStatement();
			{
				final String q = String.format("SELECT status FROM users WHERE id=%d", uid);
				ResultSet rs = stmt.executeQuery(q);
				if (rs.next())
					status = rs.getString("status");
			}
			if (status == null) {
				// insert new parent uid
				final String q = String.format("INSERT INTO users (id, gen, added_at, status) "
						+ "VALUES (%d, %d, NOW(), 'UP')", uid, gen);
				stmt.executeUpdate(q);
				_conn_crawl_tweets.commit();
				Mon.num_users_to_crawl_parent_new ++;
			} else if (status.equals("C") || status.equals("P") || status.equals("NF")) {
				// The uid is already crawled or marked as uncrawlable.
				Mon.num_users_to_crawl_parent_dup ++;
			} else if (status.equals("UC") || status.equals("UP") || status.equals("U")) {
				// update status to 'UP' and added_at to NOW().
				final String q = String.format("UPDATE users SET status='UP', gen=%d, added_at=NOW() WHERE id=%d",
						gen, uid);
				stmt.executeUpdate(q);
				_conn_crawl_tweets.commit();
				Mon.num_users_to_crawl_parent_dup ++;
			} else
				throw new RuntimeException(String.format("Unexpected status=%s, uid=%d", status, uid));
		} finally {
			if (stmt != null) stmt.close();
		}
	}

	static void AddChildUsersToCrawl(long[] uids, int gen) throws SQLException {
		Statement stmt = null;
		try {
			stmt = _conn_crawl_tweets.createStatement();
			for (long uid: uids) {
				String status = null;
				{
					final String q = String.format("SELECT status FROM users WHERE id=%d", uid);
					ResultSet rs = stmt.executeQuery(q);
					if (rs.next())
						status = rs.getString("status");
				}
				if (status == null) {
					// insert new child uid
					final String q = String.format("INSERT INTO users (id, gen, added_at, status) "
							+ "VALUES (%d, %d, NOW(), 'UC')", uid, gen);
					stmt.executeUpdate(q);
					_conn_crawl_tweets.commit();
					Mon.num_users_to_crawl_child_new ++;
				} else if (status.equals("C") || status.equals("P") || status.equals("NF")) {
					// The uid is already crawled or marked as uncrawlable.
					Mon.num_users_to_crawl_child_dup ++;
				} else if (status.equals("UC") || status.equals("UP") || status.equals("U")) {
					// update status to 'UC' and added_at to NOW(). This steals user from
					// 'U', but won't be a issue, since we supply 'U' continuously.
					final String q = String.format("UPDATE users SET status='UC', gen=%d, added_at=NOW() WHERE id=%d",
							gen, uid);
					stmt.executeUpdate(q);
					_conn_crawl_tweets.commit();
					Mon.num_users_to_crawl_child_dup ++;
				} else
					throw new RuntimeException(String.format("Unexpected status=%s, uid=%d", status, uid));
			}
		} finally {
			if (stmt != null) stmt.close();
		}
	}

	public static class UserToCrawl {
		long id;
		int gen;

		public UserToCrawl(long id_, int gen_) {
			id = id_;
			gen = gen_;
		}
	}

	static UserToCrawl GetUserToCrawl() throws SQLException, InterruptedException {
		// returns uid with status UC (uncrawled child) or UP(uncrawled parent),
		// and U(uncrawled seeded), in the repective order. If none exists, wait a
		// bit and try again.  breath-first search.
		Statement stmt = null;
		try {
			stmt = _conn_crawl_tweets.createStatement();
			long id = -1;
			int gen = -1;
			while (true) {
				id = -1;
				_conn_crawl_tweets.commit();
				{
					// TODO: specify current gen
					final String q = String.format("SELECT * FROM users "
							+ "WHERE status IN('UC', 'UP') AND "
							+ "(check_out_at IS NULL OR check_out_ip='%s' OR TIMESTAMPDIFF(SECOND, check_out_at, NOW())>%d) "
							+ "ORDER BY added_at LIMIT 1",
							Conf.ip, Conf.NEXT_CHECK_OUT_AFTER_SEC);
					ResultSet rs = stmt.executeQuery(q);
					if (rs.next()) {
						id = rs.getLong("id");
						gen = rs.getInt("gen");
					}
				}
				if (id == -1) {
					final String q = String.format("SELECT * FROM users "
							+ "WHERE status='U' AND "
							+ "(check_out_at IS NULL OR check_out_ip='%s' OR TIMESTAMPDIFF(SECOND, check_out_at, NOW())>%d) "
							+ "ORDER BY added_at LIMIT 1",
							Conf.ip, Conf.NEXT_CHECK_OUT_AFTER_SEC);
					ResultSet rs = stmt.executeQuery(q);
					if (rs.next()) {
						id = rs.getLong("id");
					}
				}
				if (id == -1) {
					// StdoutWriter.W("No user to crawl. will try again in 1 sec.");
					Mon.Sleep(1000);
					continue;
				}
				{
					final String q = String.format("UPDATE users "
							+ "SET check_out_at=NOW(), check_out_ip='%s' "
							+ "WHERE id=%d AND "
							+ "(check_out_at IS NULL OR check_out_ip='%s' OR TIMESTAMPDIFF(SECOND, check_out_at, NOW())>%d) ",
							Conf.ip, id, Conf.ip, Conf.NEXT_CHECK_OUT_AFTER_SEC);
					int affected_rows = stmt.executeUpdate(q);
					if (affected_rows == 0)
						continue;
					_conn_crawl_tweets.commit();

					if (gen == -1) {
						final String q1 = "SELECT v_int FROM meta WHERE k='gen'";
						ResultSet rs = stmt.executeQuery(q1);
						if (! rs.next())
							throw new RuntimeException("Unexpected. gen doesn't exist");
						gen = rs.getInt("v_int");
					}
					return new UserToCrawl(id, gen);
				}
			}
		} finally {
			if (stmt != null) stmt.close();
		}
	}

	static void SetUserCrawled(UserToCrawl u) throws SQLException {
		_ps_set_user_crawled.setLong(1, u.id);
		_ps_set_user_crawled.setInt(2, u.gen);
		_ps_set_user_crawled.setString(3, Conf.ip);
		_ps_set_user_crawled.executeUpdate();
		_conn_crawl_tweets.commit();
		Mon.num_crawled_users ++;
		//StdoutWriter.W(String.format("crawled all tweets of user %d", u.id));
	}

	static void SetUserUnauthorized(UserToCrawl u) throws SQLException {
		Statement stmt = null;
		try {
			stmt = _conn_crawl_tweets.createStatement();
			final String q = String.format("UPDATE users SET status='P', gen=%d, crawled_at=NOW() WHERE id=%d",
					u.gen, u.id);
			stmt.executeUpdate(q);
			_conn_crawl_tweets.commit();
		} finally {
			if (stmt != null) stmt.close();
		}
	}

	static void SetUserNotFound(UserToCrawl u) throws SQLException {
		Statement stmt = null;
		try {
			stmt = _conn_crawl_tweets.createStatement();
			final String q = String.format("UPDATE users SET status='NF', gen=%d, crawled_at=NOW() WHERE id=%d",
					u.gen, u.id);
			stmt.executeUpdate(q);
			_conn_crawl_tweets.commit();
		} finally {
			if (stmt != null) stmt.close();
		}
	}

	static void AddTweet(long id, long uid, Date created_at, GeoLocation location,
			String youtube_video_id, String ht_string, long rt_id, long rt_uid, String text, String c_uids)
		throws SQLException {
		try {
			// StdoutWriter.W(String.format("%d %d %s %s %s %s %s", id, uid, created_at, location, youtube_video_id, ht_string, text));
			_ps_insert_tweet.setLong(1, id);
			_ps_insert_tweet.setLong(2, uid);
			_ps_insert_tweet.setTimestamp(3, new java.sql.Timestamp(created_at.getTime()));
			_ps_insert_tweet.setDouble(4, location.getLatitude());
			_ps_insert_tweet.setDouble(5, location.getLongitude());
			_ps_insert_tweet.setString(6, youtube_video_id);
			_ps_insert_tweet.setString(7, ht_string);
			_ps_insert_tweet.setLong(8, rt_id);
			_ps_insert_tweet.setLong(9, rt_uid);
			_ps_insert_tweet.setString(10, text);
			_ps_insert_tweet.setString(11, c_uids);
			_ps_insert_tweet.executeUpdate();
			_conn_crawl_tweets.commit();
			Mon.num_crawled_tweets_new ++;
		} catch (SQLException e) {
			if (e.getErrorCode() == MysqlErrorNumbers.ER_DUP_ENTRY) {
				StdoutWriter.W(String.format("Dup tweet: %d", id));
			} else
				throw e;
		}
	}

	static boolean ImportFromTwitter1(UserToCrawl u) throws SQLException {
		Statement stmt = null;
		try {
			stmt = _conn_crawl_tweets.createStatement();
			{
				String q = String.format("SELECT id FROM twitter.uids_to_crawl "
						+ "WHERE crawled_at >= '2014-09-21 12:13:02' AND status='C' AND id=%d", u.id);
				ResultSet rs = stmt.executeQuery(q);
				if (! rs.next())
					return false;
			}
			{
				String q = String.format("INSERT INTO twitter2.tweets "
						+ "SELECT * FROM twitter.tweets WHERE uid=%d", u.id);
				int rows_updated = stmt.executeUpdate(q);
				Mon.num_crawled_tweets_new += rows_updated;
				Mon.num_crawled_tweets_new_imported += rows_updated;
			}
			// commit is in this function
			SetUserCrawled(u);
			return true;
		} finally {
			if (stmt != null)
				stmt.close();
		}
	}
}
