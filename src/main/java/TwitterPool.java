package crawltwitter;

import java.sql.SQLException;
import twitter4j.conf.ConfigurationBuilder;
import twitter4j.Twitter;
import twitter4j.TwitterFactory;
import twitter4j.TwitterStream;
import twitter4j.TwitterStreamFactory;


public class TwitterPool {
	static public class T {
		DB.TC tc;
		Twitter twitter;

		public void SetRateLimited() throws SQLException {
			DB.CredentialSetRateLimited(tc.token);
		}

		public void SetLastUsed() throws SQLException {
			DB.CredentialSetLastUsed(tc.token);
		}
	}

	public static T GetTwitter() throws Exception {
		T t = new T();
		t.tc = DB.GetTwitterCredential();
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
		DB.TC tc = DB.GetTwitterCredentialForStream();
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
