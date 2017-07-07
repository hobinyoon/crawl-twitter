package crawltwitter;

import java.lang.InterruptedException;
import java.sql.SQLException;
import java.util.Date;
import twitter4j.conf.ConfigurationBuilder;
import twitter4j.Twitter;
import twitter4j.TwitterFactory;
import twitter4j.TwitterStream;
import twitter4j.TwitterStreamFactory;


// The crawler doesn't really keep a pool of Twitter handles. The DB keeps a pool of credentials and reuse them when one is
// rate-limited.

public class TwitterPool {
  static public class T {
    DB.TC tc;
    Twitter twitter;

    public void SetRateLimitedAndWait(int sec_until_reset) throws SQLException, InterruptedException {
      DB.CredSetRateLimited(tc.token, sec_until_reset);
      //int num_reqs_before_rate_limited = DB.CredGetNumReqsBeforeRateLimited(tc.token);
      //if (num_reqs_before_rate_limited < 3) {
      //	StdoutWriter.W(String.format("IP %s may be rate-limited. wait for %s sec ...", Conf.ip, sec_until_reset));
      //	Thread.sleep(sec_until_reset * 1000);
      //}
    }

    public void IncReqMade() throws SQLException {
      DB.CredIncReqMade(tc.token);
    }

    public void AuthFailed() throws SQLException {
      DB.CredAuthFailed(tc.token);
    }

    public String GetTokenLast4() {
      return tc.token.substring(tc.token.length() - 4, tc.token.length());
    }
  }

  public static T GetTwitter() throws Exception {
    T t = new T();
    t.tc = DB.GetTwitterCred(false);
    ConfigurationBuilder cb = new ConfigurationBuilder();
    cb.setDebugEnabled(true)
      .setJSONStoreEnabled(true)
      .setIncludeMyRetweetEnabled(true)
      .setOAuthConsumerKey(t.tc.consumerKey)
      .setOAuthConsumerSecret(t.tc.consumerSecret)
      .setOAuthAccessToken(t.tc.token)
      .setOAuthAccessTokenSecret(t.tc.secret);
    t.twitter = new TwitterFactory(cb.build()).getInstance();
    //StdoutWriter.W(String.format("Got a new credential. token=%s", t.tc.token));
    Mon.num_credentials_used ++;
    return t;
  }

  static public class TS {
    DB.TC tc;
    TwitterStream ts;
    long last_heartbeat = -1;

    // Send heartbeat every 5 seconds
    public void SendHeartbeat() throws SQLException {
      if (last_heartbeat == -1) {
        DB.CredStreamSendHeartbeat(tc);
        last_heartbeat = System.nanoTime();
      } else {
        long time_since_last_heartbeat = System.nanoTime() - last_heartbeat;
        if (time_since_last_heartbeat > 5 * 1000 * 1000 * 1000) {
          DB.CredStreamSendHeartbeat(tc);
          last_heartbeat = System.nanoTime();
        }
      }
    }
  }

  public static TS GetTwitterStream() throws Exception {
    TS ts = new TS();
    ts.tc = DB.GetTwitterCred(true);
    ConfigurationBuilder cb = new ConfigurationBuilder();
    cb.setDebugEnabled(true)
      .setJSONStoreEnabled(true)
      .setOAuthConsumerKey(ts.tc.consumerKey)
      .setOAuthConsumerSecret(ts.tc.consumerSecret)
      .setOAuthAccessToken(ts.tc.token)
      .setOAuthAccessTokenSecret(ts.tc.secret);
    ts.ts = new TwitterStreamFactory(cb.build()).getInstance();
    Mon.num_credentials_used ++;
    return ts;
  }
}
