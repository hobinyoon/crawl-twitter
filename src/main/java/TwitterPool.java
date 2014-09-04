package crawltwitter;

import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;
import twitter4j.conf.ConfigurationBuilder;
import twitter4j.Twitter;
import twitter4j.TwitterFactory;
import twitter4j.TwitterStream;
import twitter4j.TwitterStreamFactory;


public class TwitterPool {
	public static Twitter GetNextTwitter() {
		// TODO: make a pool
		ConfigurationBuilder cb = new ConfigurationBuilder();
		TwitterCredential tc = new TwitterCredential();
		cb.setDebugEnabled(true)
			.setOAuthConsumerKey(tc.consumerKey)
			.setOAuthConsumerSecret(tc.consumerSecret)
			.setOAuthAccessToken(tc.token)
			.setOAuthAccessTokenSecret(tc.secret);
		Twitter twitter = new TwitterFactory(cb.build()).getInstance();
		return twitter;
	}


	public static TwitterStream GetNextTwitterStream() {
		// Not sure if a stream is ever throttled.
		ConfigurationBuilder cb = new ConfigurationBuilder();
		TwitterCredential tc = new TwitterCredential();
		cb.setDebugEnabled(true)
			.setOAuthConsumerKey(tc.consumerKey)
			.setOAuthConsumerSecret(tc.consumerSecret)
			.setOAuthAccessToken(tc.token)
			.setOAuthAccessTokenSecret(tc.secret);
		TwitterStream ts = new TwitterStreamFactory(cb.build()).getInstance();
		return ts;
	}
}


class TwitterCredential {
	public String token;
	public String secret;
  public String consumerKey;
	public String consumerSecret;

	public TwitterCredential() {
		Load();
	}

	void Load() {
		// http://examples.javacodegeeks.com/core-java/json/java-json-parser-example/
		try {
			String fn_twitter_credential = System.getProperty("user.home") + "/private/.twitter_auth/screen-names";
			FileReader reader = new FileReader(fn_twitter_credential);

			JSONParser jsonParser = new JSONParser();
			JSONObject jsonObject = (JSONObject) jsonParser.parse(reader);

			token = (String) jsonObject.get("TOKEN");
			secret = (String) jsonObject.get("TOKEN_SECRET");
			consumerKey = (String) jsonObject.get("CONSUMER_KEY");
			consumerSecret = (String) jsonObject.get("CONSUMER_SECRET");
			//System.out.println("token: " + token);
			//System.out.println("secret: " + secret);
			//System.out.println("consumerKey: " + consumerKey);
			//System.out.println("consumerSecret: " + consumerSecret);
		} catch (FileNotFoundException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		} catch (ParseException e) {
			e.printStackTrace();
		}
	}
}
