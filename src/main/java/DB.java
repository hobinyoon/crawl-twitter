package crawltwitter;

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
	static private String _cs = "jdbc:mysql://localhost:3306/twitter";
	static private String _user = "twitter";
	static private String _password = "twitterpass";
	static private Connection _conn_stream_seed_users = null;
	static private Connection _conn_crawl_tweets = null;
	static private PreparedStatement _ps_insert_seed_user = null;
	static private PreparedStatement _ps_mark_seed_user_done = null;
	static private PreparedStatement _ps_insert_tweet = null;
	static private PreparedStatement _ps_credential_rate_limited = null;
	static private PreparedStatement _ps_credential_last_used = null;

	static {
		try {
			_conn_stream_seed_users = DriverManager.getConnection(_cs, _user, _password);
			_conn_stream_seed_users.setAutoCommit(false);
			// This enables one connection read committed writes from the other connection.
			_conn_stream_seed_users.setTransactionIsolation(Connection.TRANSACTION_READ_COMMITTED);
			_conn_crawl_tweets = DriverManager.getConnection(_cs, _user, _password);
			_conn_crawl_tweets.setAutoCommit(false);
			_conn_crawl_tweets.setTransactionIsolation(Connection.TRANSACTION_READ_COMMITTED);

			_ps_insert_seed_user = _conn_stream_seed_users.prepareStatement(
					"INSERT INTO uids_to_crawl (id, crawled_at, status) VALUES (?, NOW(), ?)");
			_ps_mark_seed_user_done = _conn_crawl_tweets.prepareStatement("UPDATE uids_to_crawl SET status = 'C' WHERE id=(?)");
			_ps_insert_tweet = _conn_crawl_tweets.prepareStatement(
					"INSERT INTO tweets "
					+ "(id, uid, created_at, geo_lati, geo_longi, youtube_link, hashtags, rt_id, rt_uid, text) "
					+ "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
			// TODO: replace last_used with last_checked_out. last_used is not a very
			// useful info
			_ps_credential_last_used = _conn_crawl_tweets.prepareStatement(
					"UPDATE credentials SET last_used = now() WHERE token=(?)");
			// TODO: add now many tweets it crawled before rate limited. it can be a
			// good indicator of IP rate limiting.
			_ps_credential_rate_limited = _conn_crawl_tweets.prepareStatement(
					"UPDATE credentials SET last_rate_limited=NOW(), sec_until_retry=(?), sec_until_rate_limited=(?), rate_limited_ip=(?)  WHERE token=(?)");
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("Exception caught: " + e);
			System.exit(-1);
		}
	}

	static void Close() {
		try {
			if (_ps_insert_seed_user != null) _ps_insert_seed_user.close();
			if (_ps_mark_seed_user_done != null) _ps_mark_seed_user_done.close();
			if (_ps_insert_tweet != null) _ps_insert_tweet.close();
			if (_ps_credential_last_used != null) _ps_credential_last_used.close();
			if (_ps_credential_rate_limited != null) _ps_credential_rate_limited.close();

			if (_conn_stream_seed_users != null) _conn_stream_seed_users.close();
			if (_conn_crawl_tweets != null) _conn_crawl_tweets.close();
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("Exception caught: " + e);
			System.exit(-1);
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

	static TC GetTwitterCredentialForStream() throws SQLException {
		Statement stmt = null;
		try {
			stmt = _conn_stream_seed_users.createStatement();
			final String q = "SELECT * FROM credentials WHERE for_stream=true and (status is null or status !='i') LIMIT 1";
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

	static TC GetTwitterCredential() throws SQLException, InterruptedException {
		Statement stmt = null;
		try {
			stmt = _conn_crawl_tweets.createStatement();
			final String q = "SELECT *, ADDDATE(last_rate_limited, INTERVAL sec_until_retry SECOND) as retry_after "
				+ "FROM credentials WHERE "
				+ "for_stream=false "
				+ "and (status is null or status != 'i') "	// valid one
				+ "order by retry_after "
				+ "LIMIT 1";
			ResultSet rs = stmt.executeQuery(q);
			long id = -1;
			if (! rs.next())
				throw new RuntimeException("Unexpected");
			long wait_milli = rs.getTimestamp("retry_after").getTime() + Conf.cred_rate_limit_wait_cushion_in_milli - (new Date()).getTime();
			if (wait_milli > 0) {
				StdoutWriter.W(String.format("wait for %d ms for the next available credential", wait_milli));
				Thread.sleep(wait_milli);
			}
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

	static void CredentialSetRateLimited(String token, int sec_until_reset,
			int sec_until_rate_limited) throws SQLException {
		_ps_credential_rate_limited.setInt(1, sec_until_reset);
		_ps_credential_rate_limited.setInt(2, sec_until_rate_limited);
		_ps_credential_rate_limited.setString(3, Conf.ip);
		_ps_credential_rate_limited.setString(4, token);
		_ps_credential_rate_limited.executeUpdate();
		_conn_crawl_tweets.commit();
	}

	static void CredentialSetLastUsed(String token) throws SQLException {
		_ps_credential_last_used.setString(1, token);
		_ps_credential_last_used.executeUpdate();
		_conn_crawl_tweets.commit();
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

	static void AddParentUserToCrawl(long uid) throws SQLException {
		Statement stmt = null;
		try {
			String status = null;
			stmt = _conn_crawl_tweets.createStatement();
			{
				final String q = String.format("SELECT status FROM uids_to_crawl WHERE id=%d", uid);
				ResultSet rs = stmt.executeQuery(q);
				if (rs.next())
					status = rs.getString("status");
			}
			if (status == null) {
				// insert new parent uid
				final String q = String.format("INSERT INTO uids_to_crawl (id, crawled_at, status) VALUES (%d, NOW(), 'UP')", uid);
				stmt.executeUpdate(q);
				_conn_crawl_tweets.commit();
				Mon.num_users_to_crawl_parent_new ++;
			} else if (status.equals("C") || status.equals("I")) {
				// the uid is already crawled. do nothing.
				// StdoutWriter.W(String.format("Parent uid %d is already crawled", uid));
				Mon.num_users_to_crawl_parent_dup ++;
			} else {
				// update status to 'UP' and crawled_at to NOW().
				final String q = String.format("UPDATE uids_to_crawl SET status='UP', crawled_at=NOW() WHERE id=%d", uid);
				stmt.executeUpdate(q);
				_conn_crawl_tweets.commit();
				Mon.num_users_to_crawl_parent_dup ++;
			}
		} finally {
			if (stmt != null) stmt.close();
		}
	}

	static void AddChildUsersToCrawl(long[] uids) throws SQLException {
		Statement stmt = null;
		try {
			stmt = _conn_crawl_tweets.createStatement();
			for (long uid: uids) {
				String status = null;
				{
					final String q = String.format("SELECT status FROM uids_to_crawl WHERE id=%d", uid);
					ResultSet rs = stmt.executeQuery(q);
					if (rs.next())
						status = rs.getString("status");
				}
				if (status == null) {
					// insert new child uid
					final String q = String.format("INSERT INTO uids_to_crawl (id, crawled_at, status) VALUES (%d, NOW(), 'UC')", uid);
					stmt.executeUpdate(q);
					_conn_crawl_tweets.commit();
					Mon.num_users_to_crawl_child_new ++;
				} else if (status.equals("C") || status.equals("I")) {
					// the uid is already crawled. do nothing.
					// StdoutWriter.W(String.format("Parent uid %d is already crawled", uid));
					Mon.num_users_to_crawl_child_dup ++;
				} else {
					// update status to 'UC' and crawled_at to NOW().
					final String q = String.format("UPDATE uids_to_crawl SET status='UC', crawled_at=NOW() WHERE id=%d", uid);
					stmt.executeUpdate(q);
					_conn_crawl_tweets.commit();
					Mon.num_users_to_crawl_child_dup ++;
				}
			}
		} finally {
			if (stmt != null) stmt.close();
		}
	}

	static long GetUserToCrawl() throws SQLException {
		// returns uid with status UC (uncrawled child), UP(uncrawled parent), or
		// U(uncrawled seeded), in the repective order. If none exists, return -1.
		// I prioritize children, which will help build big fan-out faster, I
		// guess. "ORDER BY crawled at" makes breath-first like graph traversal.
		Statement stmt = null;
		try {
			stmt = _conn_crawl_tweets.createStatement();
			{
				final String q = "SELECT * FROM uids_to_crawl WHERE status='UC' ORDER BY crawled_at LIMIT 1";
				ResultSet rs = stmt.executeQuery(q);
				if (rs.next())
					return rs.getLong("id");
			}
			{
				final String q = "SELECT * FROM uids_to_crawl WHERE status='UP' ORDER BY crawled_at LIMIT 1";
				ResultSet rs = stmt.executeQuery(q);
				if (rs.next())
					return rs.getLong("id");
			}
			{
				final String q = "SELECT * FROM uids_to_crawl WHERE status='U' LIMIT 1";
				ResultSet rs = stmt.executeQuery(q);
				if (! rs.next())
					return -1;
				return rs.getLong("id");
			}
		} finally {
			if (stmt != null) stmt.close();
		}
	}

	static void MarkUserCrawled(long uid) throws SQLException {
		_ps_mark_seed_user_done.setLong(1, uid);
		_ps_mark_seed_user_done.executeUpdate();
		_conn_crawl_tweets.commit();
		Mon.num_crawled_users ++;
		//StdoutWriter.W(String.format("crawled all tweets of user %d", uid));
	}

	static void MarkUserInvalid(long uid) throws SQLException {
		Statement stmt = null;
		try {
			stmt = _conn_crawl_tweets.createStatement();
			final String q = String.format("UPDATE uids_to_crawl SET status='I', crawled_at=NOW() WHERE id=%d", uid);
			stmt.executeUpdate(q);
			_conn_crawl_tweets.commit();
		} finally {
			if (stmt != null) stmt.close();
		}
	}

	static void AddTweet(long id, long uid, Date created_at, GeoLocation location,
			String youtube_link, String ht_string, long rt_id, long rt_uid, String text)
		throws SQLException {
		try {
			// StdoutWriter.W(String.format("%d %d %s %s %s %s %s", id, uid, created_at, location, youtube_link, ht_string, text));
			_ps_insert_tweet.setLong(1, id);
			_ps_insert_tweet.setLong(2, uid);
			_ps_insert_tweet.setTimestamp(3, new java.sql.Timestamp(created_at.getTime()));
			_ps_insert_tweet.setDouble(4, location.getLatitude());
			_ps_insert_tweet.setDouble(5, location.getLongitude());
			_ps_insert_tweet.setString(6, youtube_link);
			_ps_insert_tweet.setString(7, ht_string);
			_ps_insert_tweet.setLong(8, rt_id);
			_ps_insert_tweet.setLong(9, rt_uid);
			_ps_insert_tweet.setString(10, text);
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
}
