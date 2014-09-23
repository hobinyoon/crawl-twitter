package crawltwitter;

import java.lang.InterruptedException;
import java.sql.SQLException;
import java.util.Date;
import twitter4j.conf.ConfigurationBuilder;
import twitter4j.Twitter;
import twitter4j.TwitterFactory;
import twitter4j.TwitterStream;
import twitter4j.TwitterStreamFactory;


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
	}

	public static T GetTwitter() throws Exception {
		T t = new T();
		t.tc = DB.GetTwitterCred();
		// Note: maybe I should make a pool of the twitter and reuse?
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

	public static TwitterStream GetTwitterStream() throws Exception {
		// A pool doesn't seem to be needed here. One handle is enough.
		DB.TC tc = DB.GetTwitterCredForStream();
		ConfigurationBuilder cb = new ConfigurationBuilder();
		cb.setDebugEnabled(true)
			.setJSONStoreEnabled(true)
			.setOAuthConsumerKey(tc.consumerKey)
			.setOAuthConsumerSecret(tc.consumerSecret)
			.setOAuthAccessToken(tc.token)
			.setOAuthAccessTokenSecret(tc.secret);
		TwitterStream ts = new TwitterStreamFactory(cb.build()).getInstance();
		return ts;
	}
}
