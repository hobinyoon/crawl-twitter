package crawltwitter;

import java.util.ArrayList;
import java.util.List;

import twitter4j.IDs;
import twitter4j.Twitter;
import twitter4j.TwitterException;
import twitter4j.TwitterFactory;
import twitter4j.conf.ConfigurationBuilder;

public final class Crawl {
	static final long CRAWL_FRIENDS_MAX = 1000;

	public static void main(String[] args) {
		try {
			ConfigurationBuilder cb = new ConfigurationBuilder();
			TwitterCredential tc = new TwitterCredential();
			cb.setDebugEnabled(true)
				.setOAuthConsumerKey(tc.consumerKey)
				.setOAuthConsumerSecret(tc.consumerSecret)
				.setOAuthAccessToken(tc.token)
				.setOAuthAccessTokenSecret(tc.secret);
			Twitter twitter = new TwitterFactory(cb.build()).getInstance();
			long seed_twitter_id = 1337785291;
			long cursor = -1;
			System.out.println("Listing following ids.");
			List<Long> fids = new ArrayList();
			{
				IDs ids;
				do {
					//ids = twitter.getFriendsIDs(seed_twitter_id, cursor);
					ids = twitter.getFollowersIDs(seed_twitter_id, cursor);
					for (long id : ids.getIDs()) {
						fids.add(id);
						if (fids.size() == CRAWL_FRIENDS_MAX)
							break;
					}
					if (fids.size() == CRAWL_FRIENDS_MAX)
						break;
				} while ((cursor = ids.getNextCursor()) != 0);
			}
			//for (long id: fids)
			//	System.out.printf("%d\n", id);
		} catch (TwitterException te) {
			te.printStackTrace();
			System.out.println("Failed to get friends' ids: " + te.getMessage());
			System.exit(-1);
		}
	}
}
