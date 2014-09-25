package crawltwitter;

import java.util.ArrayList;
import java.util.List;
import twitter4j.IDs;
import twitter4j.Twitter;


public class Practice {
	//static void GetFriendsIDs() throws Exception {
	//	final long CRAWL_FRIENDS_MAX = 5000;
	//	long seed_twitter_id = 1337785291;
	//	long cursor = -1;
	//	System.out.println("Getting friends IDs ...");
	//	List<Long> fids = new ArrayList();

	//	Twitter twitter = TwitterHandlePool.GetNext();
	//	IDs ids;
	//	do {
	//		ids = twitter.getFriendsIDs(seed_twitter_id, cursor);
	//		//ids = twitter.getFollowersIDs(seed_twitter_id, cursor);
	//		for (long id : ids.getIDs()) {
	//			fids.add(id);
	//			if (fids.size() == CRAWL_FRIENDS_MAX)
	//				break;
	//		}
	//		if (fids.size() == CRAWL_FRIENDS_MAX)
	//			break;
	//	} while ((cursor = ids.getNextCursor()) != 0);

	//	for (long id: fids)
	//		System.out.printf("%d\n", id);
	//}
}
