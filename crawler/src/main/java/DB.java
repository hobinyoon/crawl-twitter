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
  static private PreparedStatement _ps_insert_tweet = null;
  static private PreparedStatement _ps_set_user_crawled2 = null;
  static private PreparedStatement _ps_credential_rate_limited = null;

  static private void _InitUtf8mb4() throws SQLException {
    Statement stmt = null;
    try {
      stmt = _conn_crawl_tweets.createStatement();
      final String q = "SET NAMES 'utf8mb4'";
      stmt.executeUpdate(q);
      _conn_crawl_tweets.commit();
    } finally {
      if (stmt != null)
        stmt.close();
    }
  }

  static public void Init() {
    try {
      _conn_stream_seed_users = DriverManager.getConnection(Conf.db_url, Conf.db_user, Conf.db_pass);
      _conn_stream_seed_users.setAutoCommit(false);
      // This enables one connection read committed writes from the other connection.
      _conn_stream_seed_users.setTransactionIsolation(Connection.TRANSACTION_READ_COMMITTED);
      _conn_crawl_tweets = DriverManager.getConnection(Conf.db_url, Conf.db_user, Conf.db_pass);
      _conn_crawl_tweets.setAutoCommit(false);
      _conn_crawl_tweets.setTransactionIsolation(Connection.TRANSACTION_READ_COMMITTED);

      _InitUtf8mb4();

      _ps_insert_tweet = _conn_crawl_tweets.prepareStatement(
          "INSERT INTO tweets "
          + "(id, uid, created_at, geo_lati, geo_longi, youtube_video_id) "
          + "VALUES (?, ?, ?, ?, ?, ?)");

      _ps_set_user_crawled2 = _conn_crawl_tweets.prepareStatement(
          "UPDATE meta SET v_int = v_int + 1 WHERE k = 'C_cnt'");

      _ps_credential_rate_limited = _conn_crawl_tweets.prepareStatement(
          "UPDATE credentials SET rate_limited_at=NOW(), sec_until_retry=(?) WHERE token=(?)");

      _InitGenIfNotExists();
    } catch (Exception e) {
      e.printStackTrace();
      System.out.println("Exception caught: " + e);
      System.exit(-1);
    }
  }

  static void Close() {
    try {
      if (_ps_insert_tweet != null) _ps_insert_tweet.close();
      if (_ps_set_user_crawled2 != null) _ps_set_user_crawled2.close();
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

  static TC GetTwitterCred(boolean checked_out_for_streaming) throws SQLException, InterruptedException {
    Statement stmt = null;
    try {
      stmt = _conn_crawl_tweets.createStatement();
      String token = null;
      String token_secret = null;
      String consumer_key = null;
      String consumer_secret = null;

      while (true) {
        // Begin transaction
        _conn_crawl_tweets.commit();
        {
          // Wait for 1 hour when there has been more than 3 auth failures from this IP in the last hour
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
          // Pick the oldest rate-limited credential
          final String q = String.format(
              "SELECT *, ADDDATE(rate_limited_at, INTERVAL sec_until_retry SECOND) as retry_after "
              + "FROM credentials "
              + "WHERE "
              + "(status is null or status != 'I') "  // One with a valid status
              + "and (checked_out_at is null or TIMESTAMPDIFF(SECOND, checked_out_at, NOW()) > 60) "  // One not checked-out in the last 60 secs
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
              + "SET checked_out_at=NOW(), checked_out_ip='%s', checked_out_for_streaming=%d, num_reqs_before_rate_limited=0 "
              + "WHERE token='%s' "
              + "and (checked_out_at is null or TIMESTAMPDIFF(SECOND, checked_out_at, NOW()) > 60)"
              , Conf.ip
              , (checked_out_for_streaming ? 1 : 0)
              , token);
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

  static void CredStreamSendHeartbeat(TC tc) throws SQLException {
    Statement stmt = null;
    while (true) {
      try {
        stmt = _conn_crawl_tweets.createStatement();
        final String q = String.format(
            "UPDATE credentials "
            + "SET checked_out_ip='%s', checked_out_at=NOW(), sec_until_retry=300 "
            + "WHERE token='%s'", Conf.ip, tc.token);
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

  static void AddSeedUserToCrawl(long uid) throws SQLException {
    Statement stmt = null;
    try {
      stmt = _conn_stream_seed_users.createStatement();
      {
        long cnt = 0;
        final String q = String.format("SELECT count(*) as cnt FROM users_crawled WHERE id=%d", uid);
        ResultSet rs = stmt.executeQuery(q);
        if (rs.next())
          cnt = rs.getLong("cnt");
        if (cnt > 0) {
          Mon.num_users_to_crawl_streamed_dup ++;
          return;
        }
      }

      try {
        final String q = String.format("INSERT INTO users (id, gen, added_at, status) VALUES (%d, -1, NOW(), 'U')", uid);
        int affected_rows = stmt.executeUpdate(q);
        if (affected_rows == 1) {
          _conn_stream_seed_users.commit();
          Mon.num_users_to_crawl_streamed_new ++;
          return;
        }
      } catch (SQLException e) {
        if (e.getErrorCode() == MysqlErrorNumbers.ER_DUP_ENTRY)
          Mon.num_users_to_crawl_streamed_dup ++;
        else
          throw e;
      }
    } finally {
      if (stmt != null) stmt.close();
    }
  }

  static void AddParentUserToCrawl(long uid, int gen) throws SQLException {
    Statement stmt = null;
    try {
      stmt = _conn_crawl_tweets.createStatement();
      {
        long cnt = 0;
        final String q = String.format("SELECT count(*) as cnt FROM users_crawled WHERE id=%d", uid);
        ResultSet rs = stmt.executeQuery(q);
        if (rs.next())
          cnt = rs.getLong("cnt");
        if (cnt > 0) {
          Mon.num_users_to_crawl_parent_dup ++;
          return;
        }
      }

      String status = null;
      {
        final String q = String.format("SELECT status FROM users WHERE id=%d", uid);
        ResultSet rs = stmt.executeQuery(q);
        if (rs.next())
          status = rs.getString("status");
      }
      if (status == null) {
        try {
          // Insert a new parent uid.
          final String q = String.format("INSERT INTO users (id, gen, added_at, status) "
              + "VALUES (%d, %d, NOW(), 'UP')", uid, gen);
          stmt.executeUpdate(q);
          _conn_crawl_tweets.commit();
          Mon.num_users_to_crawl_parent_new ++;
        } catch (SQLException e) {
          // There can be a race here. It's okay to ignore.
          if (e.getErrorCode() == MysqlErrorNumbers.ER_DUP_ENTRY)
            Mon.num_users_to_crawl_parent_dup ++;
          else
            throw e;
        }
      } else if (status.equals("C") || status.equals("U") || status.equals("P") || status.equals("NF")) {
        Mon.num_users_to_crawl_parent_dup ++;
      } else if (status.equals("UC") || status.equals("UP")) {
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
        {
          long cnt = 0;
          final String q = String.format("SELECT count(*) as cnt FROM users_crawled WHERE id=%d", uid);
          ResultSet rs = stmt.executeQuery(q);
          if (rs.next())
            cnt = rs.getLong("cnt");
          if (cnt > 0) {
            Mon.num_users_to_crawl_child_dup ++;
            return;
          }
        }

        String status = null;
        {
          final String q = String.format("SELECT status FROM users WHERE id=%d", uid);
          ResultSet rs = stmt.executeQuery(q);
          if (rs.next())
            status = rs.getString("status");
        }
        if (status == null) {
          try {
            // Insert a new child uid
            final String q = String.format("INSERT INTO users (id, gen, added_at, status) "
                + "VALUES (%d, %d, NOW(), 'UC')", uid, gen);
            stmt.executeUpdate(q);
            _conn_crawl_tweets.commit();
            Mon.num_users_to_crawl_child_new ++;
          } catch (SQLException e) {
            // There can be a race here. It's okay to ignore.
            if (e.getErrorCode() == MysqlErrorNumbers.ER_DUP_ENTRY)
              Mon.num_users_to_crawl_child_dup ++;
            else
              throw e;
          }
        } else if (status.equals("C") || status.equals("U") || status.equals("P") || status.equals("NF")) {
          Mon.num_users_to_crawl_child_dup ++;
        } else if (status.equals("UC") || status.equals("UP")) {
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

    public String toString() {
      return String.format("%d,%d", id, gen);
    }
  }

  static UserToCrawl GetUserToCrawl() throws SQLException, InterruptedException {
    //if (true) {
    //  // java.sql.SQLException: Incorrect string value: '\xF0\x9F\x91\x86wi...' for column 'text' at row 1
    //  return new UserToCrawl(84947814, -1);

    //  // contains UTF8 strings
    //  return new UserToCrawl(348878135, -1);
    //}

    // returns uid with status UC (uncrawled child) or UP(uncrawled parent),
    // and U(uncrawled seeded), in the repective order. If none exists, wait a
    // bit and try again.  breath-first search.
    Statement stmt = null;
    try {
      stmt = _conn_crawl_tweets.createStatement();
      while (true) {
        long id = -1;
        int gen = -1;
        _conn_crawl_tweets.commit();
        {
          final String q = String.format("SELECT * FROM users "
              + "WHERE status IN('UC', 'UP') "
              + "AND gen=(select v_int FROM meta WHERE k='gen') "
              + "AND (checked_out_at IS NULL OR checked_out_ip='%s' OR TIMESTAMPDIFF(SECOND, checked_out_at, NOW())>%d) "
              + "ORDER BY added_at LIMIT 1",
              Conf.ip, Conf.NEXT_CHECK_OUT_AFTER_SEC);
          ResultSet rs = stmt.executeQuery(q);
          if (rs.next()) {
            id = rs.getLong("id");
            gen = rs.getInt("gen");
          }
        }

        // Pick one from 'U's when you have more than 1000 of those.
        // Otherwise, pick one randomly from 'UP's and 'UC's.
        if (id == -1) {
          final String q = "SELECT COUNT(*) as cnt FROM users WHERE status='U'";
          ResultSet rs = stmt.executeQuery(q);
          if (! rs.next())
            throw new RuntimeException("Unexpected");
          if (rs.getLong("cnt") > 1000) {
            final String q1 = String.format("SELECT * FROM users "
                + "WHERE status='U' "
                + "AND (checked_out_at IS NULL OR checked_out_ip='%s' OR TIMESTAMPDIFF(SECOND, checked_out_at, NOW())>%d) "
                + "ORDER BY added_at LIMIT 1",
                Conf.ip, Conf.NEXT_CHECK_OUT_AFTER_SEC);
            ResultSet rs1 = stmt.executeQuery(q1);
            if (rs1.next()) {
              id = rs1.getLong("id");
            }
          } else {
            // This doesn't seem to return random id. Maybe a MySQL bug. No idea.
            //final String q1 = "SELECT MAX(id) as max_id FROM users "
            //  + "WHERE status IN ('UP', 'UC') "
            //  + "AND id < (RAND() * (SELECT MAX(id) FROM users))";

            final String q1 = "SELECT id FROM users WHERE status IN ('UP', 'UC', 'U') "
              + "ORDER BY RAND() LIMIT 1";
            ResultSet rs1 = stmt.executeQuery(q1);
            if (rs1.next()) {
              id = rs1.getLong("id");
            }
          }
        }

        if (id == -1) {
          return new UserToCrawl(-1, -1);
        }

        {
          // Check out the user id. This prevents races between crawlers.
          final String q = String.format("UPDATE users "
              + "SET checked_out_at=NOW(), checked_out_ip='%s' "
              + "WHERE id=%d AND "
              + "(checked_out_at IS NULL OR checked_out_ip='%s' OR TIMESTAMPDIFF(SECOND, checked_out_at, NOW())>%d) ",
              Conf.ip, id, Conf.ip, Conf.NEXT_CHECK_OUT_AFTER_SEC);
          try {
            int affected_rows = stmt.executeUpdate(q);
            if (affected_rows == 0)
              continue;
            _conn_crawl_tweets.commit();
          } catch (SQLException e) {
            if (e.getErrorCode() == MysqlErrorNumbers.ER_LOCK_WAIT_TIMEOUT) {
              StdoutWriter.W(String.format("Lock wait timeout while checking out uid=%d. rolling back and retrying ...", id));
              _conn_crawl_tweets.rollback();
              Mon.Sleep(1000);
              continue;
            } else
              throw e;
          }

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

  static void SetUserCrawled(UserToCrawl u) throws SQLException, InterruptedException {
    _SetUserCrawled(u);
    _IncGen();
  }

  static void _SetUserCrawled(UserToCrawl u) throws SQLException, InterruptedException {
    while (true) {
      Statement stmt = null;
      try {
        stmt = _conn_crawl_tweets.createStatement();
        String q = String.format(
            "INSERT INTO users_crawled (id, crawled_at) VALUES (%d, NOW()) "
            + "ON DUPLICATE KEY UPDATE crawled_at=NOW() "
            , u.id);
        int affected_rows = stmt.executeUpdate(q);
        // 1 when inserted, 2 when updated an existing row
        if (affected_rows == 0)
          throw new RuntimeException(String.format("Unexpected: q=[%s]", q));

        q = String.format("DELETE FROM users WHERE id=%d ", u.id);
        affected_rows = stmt.executeUpdate(q);
        if (affected_rows != 1)
          throw new RuntimeException("Unexpected");

        _ps_set_user_crawled2.executeUpdate();

        _conn_crawl_tweets.commit();
        Mon.num_crawled_users ++;
        //StdoutWriter.W(String.format("crawled all tweets of user %d", u.id));

        if (stmt != null)
          stmt.close();
        break;
      } catch (SQLException e) {
        if (e.getErrorCode() == MysqlErrorNumbers.ER_LOCK_WAIT_TIMEOUT) {
          StdoutWriter.W(String.format("Lock wait timeout while setting user crawled uid=%d. rolling back and retrying ...", u.id));
          _conn_crawl_tweets.rollback();
          Mon.Sleep(1000);
          continue;
        } else
          throw e;
      }
    }
  }

  private static int _num_uncrawled_users = -1;
  private static int _num_calls_uncrawled_users = 0;

  public static synchronized int GetNumUncrawledUsers() throws SQLException {
    _num_calls_uncrawled_users ++;
    if (_num_uncrawled_users == -1) {
      _num_uncrawled_users = _GetNumUncrawledUsers();
    } else {
      if (_num_calls_uncrawled_users % 100 == 0)
        _num_uncrawled_users = _GetNumUncrawledUsers();
    }
    return _num_uncrawled_users;
  }

  private static int _GetNumUncrawledUsers() throws SQLException {
    Statement stmt = null;
    try {
      stmt = _conn_crawl_tweets.createStatement();
      final String q = "SELECT count(*) AS cnt FROM users WHERE status IN ('U', 'UC', 'UP')";
      ResultSet rs = stmt.executeQuery(q);
      if (rs.next())
        return rs.getInt("cnt");
      throw new RuntimeException("Unexpected");
    } finally {
      if (stmt != null)
        stmt.close();
    }
  }

  private static void _InitGenIfNotExists() throws SQLException {
    Statement stmt = null;
    try {
      stmt = _conn_crawl_tweets.createStatement();

      boolean need_init = false;
      long cur_gen = -1;
      {
        final String q = "SELECT v_int AS CNT FROM meta WHERE k='gen'";
        ResultSet rs = stmt.executeQuery(q);
        if (rs.next()) {
          cur_gen = rs.getLong("cnt");
        } else {
          need_init = true;
        }
      }

      if (need_init) {
        {
          final String q = "INSERT into meta (k, v_int) VALUES ('users_C_cnt_when_gen_inc', 0)";
          int affected_rows = stmt.executeUpdate(q);
          if (affected_rows != 1)
            throw new RuntimeException("Unexpected");
        }

        {
          final String q = "INSERT into meta (k, v_int) VALUES ('gen', 0)";
          int affected_rows = stmt.executeUpdate(q);
          if (affected_rows != 1)
            throw new RuntimeException("Unexpected");
        }
      }

      _conn_crawl_tweets.commit();
    } finally {
      if (stmt != null) stmt.close();
    }
  }

  static void _IncGen() throws SQLException {
    if (Mon.num_crawled_users % 400 != 0)
      return;

    Statement stmt = null;
    try {
      stmt = _conn_crawl_tweets.createStatement();

      long cur_gen = -1;
      {
        final String q = "SELECT v_int AS CNT FROM meta WHERE k='gen'";
        ResultSet rs = stmt.executeQuery(q);
        if (! rs.next())
          throw new RuntimeException("Unexpected");
        cur_gen = rs.getLong("cnt");
      }

      long prev_c_cnt = -1;
      long c_cnt = -1;
      {
        final String q = "SELECT v_int AS cnt FROM meta WHERE k='users_C_cnt_when_gen_inc'";
        ResultSet rs = stmt.executeQuery(q);
        if (! rs.next())
          throw new RuntimeException("Unexpected");
        prev_c_cnt = rs.getLong("cnt");
      }
      {
        final String q = "SELECT count(*) AS cnt FROM users_crawled";
        ResultSet rs = stmt.executeQuery(q);
        if (! rs.next())
          throw new RuntimeException("Unexpected");
        c_cnt = rs.getLong("cnt");
      }

      if ((prev_c_cnt / 2000) == (c_cnt / 2000))
        return;

      {
        // Increase gen. Having cur_gen prevents multiple crawlers incrementing
        // gen at the same time.
        final String q = String.format("UPDATE meta SET v_int=v_int+1 "
            + "WHERE k='gen' and v_int=%d", cur_gen);
        int affected_rows = stmt.executeUpdate(q);
        if (affected_rows != 1)
          return;
      }
      {
        // Update users_C_cnt_when_gen_inc.
        final String q = String.format("UPDATE meta SET v_int=%d "
            + "WHERE k='users_C_cnt_when_gen_inc' AND v_int=%d ",
            c_cnt, prev_c_cnt);
        int affected_rows = stmt.executeUpdate(q);
        if (affected_rows != 1)
          throw new RuntimeException("Unexpected");
      }
      {
        final String q = "SELECT v_int AS CNT FROM meta WHERE k='gen'";
        ResultSet rs = stmt.executeQuery(q);
        if (! rs.next())
          throw new RuntimeException("Unexpected");
        long gen = rs.getLong("cnt");
        StdoutWriter.W(String.format("Increase meta.gen to %d", gen));
      }

      _conn_crawl_tweets.commit();
    } finally {
      if (stmt != null) stmt.close();
    }
  }

  // We just add the user to users_crawled table. No need to keep them separate for now.
  static void SetUserUnauthorized(UserToCrawl u) throws SQLException, InterruptedException {
    _SetUserCrawled(u);
  }

  // We just add the user to users_crawled table. No need to keep them separate for now.
  static void SetUserNotFound(UserToCrawl u) throws SQLException, InterruptedException {
    _SetUserCrawled(u);
  }

  static void AddTweet(long id, long uid, Date created_at, GeoLocation location, String youtube_video_id)
    throws SQLException {
    try {
      // StdoutWriter.W(String.format("%d %d %s %s %s", id, uid, created_at, location, youtube_video_id));
      _ps_insert_tweet.setLong(1, id);
      _ps_insert_tweet.setLong(2, uid);
      _ps_insert_tweet.setTimestamp(3, new java.sql.Timestamp(created_at.getTime()));
      _ps_insert_tweet.setDouble(4, location.getLatitude());
      _ps_insert_tweet.setDouble(5, location.getLongitude());
      _ps_insert_tweet.setString(6, youtube_video_id);
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
