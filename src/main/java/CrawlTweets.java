package crawltwitter;

import java.lang.InterruptedException;
import java.lang.Math;
import java.util.Date;
import java.util.List;
import twitter4j.GeoLocation;
import twitter4j.HashtagEntity;
import twitter4j.Paging;
import twitter4j.Status;
import twitter4j.Twitter;
import twitter4j.TwitterException;
import twitter4j.URLEntity;
import twitter4j.json.DataObjectFactory;


public class CrawlTweets {
	private static Thread _t = null;
	private static volatile boolean _stop_requested = false;

	static void Run() {
		_t = new Thread() {
			public void run() {
				try {
					while (! _stop_requested) {
						_CrawlUserTweets();
					}
				} catch (InterruptedException e) {
					;
				} catch (Exception e) {
					e.printStackTrace();
					StdoutWriter.W(e.toString());
					System.exit(1);
				}
			}
		};
		_t.start();
	}

	public static void Stop() {
		try {
			_stop_requested = true;
			_t.interrupt();
			_t.join();
			StdoutWriter.W("CrawlTweets stopped.");
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

	static TwitterPool.T _tpt = null;

	static void _CrawlUserTweets() throws Exception {
		long uid = -1;
		while (true) {
			uid = DB.GetUserToCrawl();
			Mon.current_uid = uid;
			if (uid != -1)
				break;
			//StdoutWriter.W("No user to crawl. will try again in 1 sec.");
			Thread.sleep(1000);
		}

		// one on Sep 2. Not the best tight bound.
		long max_id = 506827318132629504L;

		if (_tpt == null)
			_tpt = TwitterPool.GetTwitter();
		Paging p = new Paging();
		p.setCount(200);

		GeoLocation known_gl = null;	// youngest known location older then the current (unknown) one.
		while (! _stop_requested) {
			//StdoutWriter.W(String.format("max_id=%d", max_id));
			p.setMaxId(max_id);
			List<Status> statuses = null;
			do {
				try {
					statuses = _tpt.twitter.getUserTimeline(uid, p);
					_tpt.SetLastUsed();
					break;
				} catch (TwitterException e) {
					int sc = e.getStatusCode();
					if (sc == 429) {
						StdoutWriter.W("Twitter credential is rate-limited. Switching to a new one.");
						_tpt.SetRateLimited();
						_tpt = TwitterPool.GetTwitter();
					} else if (sc == 404) {
						StdoutWriter.W(String.format("Invalid user request. uid=%d", uid));
						DB.MarkUserInvalid(uid);
						return;
					} else
						throw e;
				}
			} while (true);

			//StdoutWriter.W(String.format("getUserTimeline: Got %d", statuses.size()));
			long min_id = -1;
			boolean hit_oldest_date = false;
			for (Status s : statuses) {
				//StdoutWriter.W(DataObjectFactory.getRawJSON(s));
				long id = s.getId();
				if (min_id == -1)
					min_id = id;
				else
					min_id = Math.min(min_id, id);

				Mon.num_crawled_tweets ++;
				//StdoutWriter.W(String.format(" %s %d %s", uid, id, s.getCreatedAt()));

				// locations; for the current tweet and for interpolation of others
				GeoLocation gl = s.getGeoLocation();
				if (gl != null)
					known_gl = gl;

				// statuses are decreasingly ordered by ca
				Date ca = s.getCreatedAt();
				if (ca.after(Conf.tweet_youngest_date))
					continue;
				if (ca.before(Conf.tweet_oldest_date)) {
					hit_oldest_date = true;
					break;
				}

				if (known_gl == null)
					continue;

				// filter ones with youtube link
				String youtube_link = null;
				for (URLEntity e: s.getURLEntities()) {
					if (e.getExpandedURL().toLowerCase().contains("youtube.")) {
						youtube_link = e.getExpandedURL();
						youtube_link = youtube_link.substring(youtube_link.indexOf("/watch?") + 7);
						break;
					}
				}
				if (youtube_link == null)
					continue;

				// filter ones with hashtags. space is not allowed in them and and is
				// used to delimit multiple of them in DB.
				HashtagEntity[] hashtags = s.getHashtagEntities();
				if (hashtags.length == 0)
					continue;
				StringBuilder ht_string = new StringBuilder();
				for (int i = 0; i < hashtags.length; i ++) {
					if (i != 0)
						ht_string.append(" ");
					ht_string.append(hashtags[i].getText());
				}

				long rt_id = -1;
				if (s.isRetweet()) {
					rt_id = s.getRetweetedStatus().getId();
					DB.AddParentUserToCrawl(rt_id);
				}

				if (s.isRetweeted()) {
					// TODO: find user IDs of retweets of this tweet and add to
					// uids_to_crawl. their statuses are 'UC'
				}

				DB.AddTweet(id, uid, ca, known_gl, youtube_link, ht_string.toString(), rt_id, s.getText());
			}
			if (statuses.size() == 0 || min_id == -1 || hit_oldest_date) {
				DB.MarkUserCrawled(uid);
				break;
			}
			//StdoutWriter.W(String.format("min_id=%d", min_id));
			max_id = min_id - 1;
		}
	}
}
