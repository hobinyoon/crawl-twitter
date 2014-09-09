package crawltwitter;

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
	static private Connection _conn_crawl_seed_users = null;
	static private Connection _conn_crawl_child_tweets = null;
	static private PreparedStatement _ps_add_seed_user = null;
	static private PreparedStatement _ps_mark_seed_user_done = null;
	static private PreparedStatement _ps_insert_child_tweet = null;
	static private PreparedStatement _ps_credential_rate_limited = null;
	static private PreparedStatement _ps_credential_last_used = null;

	static {
		try {
			_conn_crawl_seed_users = DriverManager.getConnection(_cs, _user, _password);
			_conn_crawl_seed_users.setAutoCommit(false);
			_conn_crawl_child_tweets = DriverManager.getConnection(_cs, _user, _password);
			_conn_crawl_child_tweets.setAutoCommit(false);

			_ps_add_seed_user = _conn_crawl_seed_users.prepareStatement("INSERT INTO uids_to_crawl (id, status) VALUES (?, ?)");
			_ps_mark_seed_user_done = _conn_crawl_child_tweets.prepareStatement("UPDATE uids_to_crawl SET status = 'C' WHERE id=(?)");
			_ps_insert_child_tweet = _conn_crawl_child_tweets.prepareStatement(
					"INSERT INTO tweets "
					+ "(id, uid, created_at, geo_lati, geo_longi, youtube_link, hashtags, rt_id, text) "
					+ "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
			_ps_credential_last_used = _conn_crawl_child_tweets.prepareStatement(
					"UPDATE credentials SET last_used = now() WHERE token=(?)");
			_ps_credential_rate_limited = _conn_crawl_child_tweets.prepareStatement(
					"UPDATE credentials SET last_rate_limited = now() WHERE token=(?)");
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("Exception caught: " + e);
			System.exit(-1);
		}
	}

	static void Close() {
		try {
			if (_ps_add_seed_user != null) _ps_add_seed_user.close();
			if (_ps_mark_seed_user_done != null) _ps_mark_seed_user_done.close();
			if (_ps_insert_child_tweet != null) _ps_insert_child_tweet.close();
			if (_ps_credential_last_used != null) _ps_credential_last_used.close();
			if (_ps_credential_rate_limited != null) _ps_credential_rate_limited.close();

			if (_conn_crawl_seed_users != null) _conn_crawl_seed_users.close();
			if (_conn_crawl_child_tweets != null) _conn_crawl_child_tweets.close();
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
			stmt = _conn_crawl_seed_users.createStatement();
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

	static TC GetTwitterCredential() throws SQLException {
		Statement stmt = null;
		try {
			stmt = _conn_crawl_child_tweets.createStatement();
			final String q = "SELECT * FROM credentials WHERE "
				+ "for_stream=false "
				+ "and (status is null or status!='i') "	// valid one
				// if rate-limited, it should have been more than 15 mins ago
				+ "and (last_rate_limited is null or TIMESTAMPDIFF(MINUTE, last_rate_limited, NOW()) > 16) "
				+ "order by last_used "	// pick the oldest one
				+ "LIMIT 1";
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

	static void CredentialSetRateLimited(String token) throws SQLException {
		_ps_credential_rate_limited.setString(1, token);
		_ps_credential_rate_limited.executeUpdate();
		_conn_crawl_child_tweets.commit();
	}

	static void CredentialSetLastUsed(String token) throws SQLException {
		_ps_credential_last_used.setString(1, token);
		_ps_credential_last_used.executeUpdate();
		_conn_crawl_child_tweets.commit();
	}

	static boolean AddUserToCrawl(long uid, String status) throws SQLException {
		try {
			_ps_add_seed_user.setLong(1, uid);
			_ps_add_seed_user.setString(2, status);
			_ps_add_seed_user.executeUpdate();
			_conn_crawl_seed_users.commit();
			return true;
		} catch (SQLException e) {
			if (e.getErrorCode() == MysqlErrorNumbers.ER_DUP_ENTRY) {
				StdoutWriter.W(String.format("Dup user: %d", uid));
			} else
				throw e;
		}
		return false;
	}

	static long GetUserToCrawl() throws SQLException {
		// return uid with UP(uncrawled parent), US(uncrawled sibling), or
		// U(uncrawled seeded), in the repective order. If none exists, return -1.
		Statement stmt = null;
		try {
			stmt = _conn_crawl_child_tweets.createStatement();
			{
				final String q = "SELECT * FROM uids_to_crawl WHERE status='UP' LIMIT 1";
				ResultSet rs = stmt.executeQuery(q);
				if (rs.next())
					return rs.getInt("id");
			}
			{
				final String q = "SELECT * FROM uids_to_crawl WHERE status='US' LIMIT 1";
				ResultSet rs = stmt.executeQuery(q);
				if (rs.next())
					return rs.getInt("id");
			}
			{
				final String q = "SELECT * FROM uids_to_crawl WHERE status='U' LIMIT 1";
				ResultSet rs = stmt.executeQuery(q);
				if (! rs.next())
					return -1;
				return rs.getInt("id");
			}
		} finally {
			if (stmt != null) stmt.close();
		}
	}

	static void MarkUserCrawled(long uid) throws SQLException {
		_ps_mark_seed_user_done.setLong(1, uid);
		_ps_mark_seed_user_done.executeUpdate();
		_conn_crawl_child_tweets.commit();
		Mon.num_crawled_child_users ++;
		//StdoutWriter.W(String.format("crawled all tweets of user %d", uid));
	}

	static void AddChildTweet(long id, long uid, Date created_at, GeoLocation location,
			String youtube_link, String ht_string, long rt_id, String text)
		throws SQLException {
		try {
			// StdoutWriter.W(String.format("%d %d %s %s %s %s %s", id, uid, created_at, location, youtube_link, ht_string, text));
			_ps_insert_child_tweet.setLong(1, id);
			_ps_insert_child_tweet.setLong(2, uid);
			_ps_insert_child_tweet.setTimestamp(3, new java.sql.Timestamp(created_at.getTime()));
			_ps_insert_child_tweet.setDouble(4, location.getLatitude());
			_ps_insert_child_tweet.setDouble(5, location.getLongitude());
			_ps_insert_child_tweet.setString(6, youtube_link);
			_ps_insert_child_tweet.setString(7, ht_string);
			_ps_insert_child_tweet.setLong(8, rt_id);
			_ps_insert_child_tweet.setString(9, text);
			_ps_insert_child_tweet.executeUpdate();
			_conn_crawl_child_tweets.commit();
			Mon.num_crawled_tweets_inserted_to_db ++;
		} catch (SQLException e) {
			if (e.getErrorCode() == MysqlErrorNumbers.ER_DUP_ENTRY) {
				StdoutWriter.W(String.format("Dup tweet: %d", id));
			} else
				throw e;
		}
	}
}
