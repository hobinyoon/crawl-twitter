// http://examples.javacodegeeks.com/core-java/json/java-json-parser-example/

package crawltwitter;

import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.parser.JSONParser;
import org.json.simple.parser.ParseException;


public class TwitterCredential {
	public String token;
	public String secret;
  public String consumerKey;
	public String consumerSecret;

	public TwitterCredential() {
		Load();
	}

	void Load() {
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
