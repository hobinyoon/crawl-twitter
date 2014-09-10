package crawltwitter;

import java.util.Calendar;
import java.util.Date;

public final class Conf {
	public static String dn_children = "/mnt/mdc-data/pbr/twitter/children/concise/children";
	public static String db_user = "twitter";
	public static String db_pass = "twitterpass";

	public static String fn_twitter_credential_stream = System.getProperty("user.home") + "/private/.twitter_auth/crawl-seed-users";
	public static String fn_twitter_credentials = System.getProperty("user.home") + "/private/.twitter_auth/credentials";

	public static Date user_ca_oldest_date = null;
	public static Date tweet_oldest_date = null;
	public static Date tweet_youngest_date = null;

	public static long cred_rate_limit_wait_cushion_in_milli = 5000;

	static {
		Calendar cal = Calendar.getInstance();
		cal.set(2013, 8, 1, 0, 0, 0);	// 8 is September
		user_ca_oldest_date = cal.getTime();
		//System.out.println(user_ca_oldest_date);

		tweet_oldest_date = user_ca_oldest_date;
		cal.set(2014, 8, 1, 0, 0, 0);
		tweet_youngest_date = cal.getTime();
	}
}
