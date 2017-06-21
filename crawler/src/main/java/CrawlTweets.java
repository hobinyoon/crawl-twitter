package crawltwitter;

import java.io.PrintWriter;
import java.io.StringWriter;
import java.lang.InterruptedException;
import java.lang.Math;
import java.util.Date;
import java.util.List;

import twitter4j.GeoLocation;
import twitter4j.HttpResponseCode;
import twitter4j.IDs;
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
				} catch (Exception e) {
					StringWriter sw = new StringWriter();
					e.printStackTrace(new PrintWriter(sw));
					StdoutWriter.W(String.format("Exception: %s\nStack: %s", e, sw));
				}
			}
		};
		_t.start();
		try {
			_t.join();
		} catch (InterruptedException e) {
			StringWriter sw = new StringWriter();
			e.printStackTrace(new PrintWriter(sw));
			StdoutWriter.W(String.format("Exception: %s\nStack: %s", e, sw));
		}
		_t = null;
	}

	public static void Stop() {
		try {
			_stop_requested = true;
			if (_t != null) {
				_t.join();
				StdoutWriter.W("CrawlTweets stopped.");
			}
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}

	static TwitterPool.T _tpt = null;

	static public String GetCredTokenLast4() {
		if (_tpt == null)
			return null;
		else
			return _tpt.GetTokenLast4();
	}

	static void _CrawlUserTweets() throws Exception {
		DB.UserToCrawl u;
		while (true) {
			u = DB.GetUserToCrawl();
			if (u.id == -1) {
				// No users to crawl
				if (_stop_requested) {
					StdoutWriter.W("No users to crawl. Stop requested.");
					return;
				} else {
					StdoutWriter.W("No users to crawl. Retrying in 1 sec.");
					Mon.Sleep(1000);
					continue;
				}
			} else {
				// We've got a user to crawl its timeline.
				Mon.user_being_crawled = u;
				break;
			}
		}

		// Init max_id so that all user timeline crawling has the same youngest time bound
		//   A tweet from Jul 21, 2017
		long max_id = 877553326690361348L;

		if (_tpt == null)
			_tpt = TwitterPool.GetTwitter();
		Paging p = new Paging();
		p.setCount(200);

		GeoLocation known_gl = null;	// youngest known location older then the current (unknown) one.
		while (! _stop_requested) {
			StdoutWriter.W(String.format("max_id=%d", max_id));
			p.setMaxId(max_id);
			List<Status> statuses = null;
			long sleep_time = 1000;
			while (true) {
				try {
					if (_stop_requested) return;
					statuses = _tpt.twitter.getUserTimeline(u.id, p);
					_tpt.IncReqMade();
					break;
				} catch (TwitterException e) {
					if (e.exceededRateLimitation()) {
						int sec_until_reset = e.getRateLimitStatus().getSecondsUntilReset();
						//StdoutWriter.W(String.format("rate-limited: %s", e));
						_tpt.SetRateLimitedAndWait(sec_until_reset);
						_tpt = TwitterPool.GetTwitter();
					} else if (e.getStatusCode() == HttpResponseCode.UNAUTHORIZED) {
						// 401: UNAUTHORIZED
						if (e.getErrorCode() == 89) {
							// "Invalid or expired token"
							StdoutWriter.W(String.format("uid=%d token=%s TwitterException: [%s]",
										u.id, _tpt.tc.token, e));
							System.exit(0);
						}
						DB.SetUserUnauthorized(u);
						return;
					} else if (e.getStatusCode() == HttpResponseCode.NOT_FOUND) {
						DB.SetUserNotFound(u);
						return;
					} else {
						// It can be 130 (Over capacity), 131 (Internal error), or anything.
						StdoutWriter.W(String.format("uid=%d token=%s TwitterException=[%s]\n"
									+ "Waiting %d ms and retrying ...",
									u.id, _tpt.tc.token, e, sleep_time));
						Mon.Sleep(sleep_time);
						sleep_time *= 2;
					}
				}
			}

			StdoutWriter.W(String.format("getUserTimeline: Got %d", statuses.size()));
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
				//StdoutWriter.W(String.format(" %d %d %s", u.id, id, s.getCreatedAt()));

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
				String youtube_video_id = null;
				for (URLEntity e: s.getURLEntities()) {
					youtube_video_id = Filter.YouTubeLink(e.getExpandedURL());
					if (youtube_video_id != null)
						break;
				}
				if (youtube_video_id == null)
					continue;

				long rt_id = -1;
				long rt_uid = -1;
				if (s.isRetweet()) {
					rt_id = s.getRetweetedStatus().getId();
					rt_uid = s.getRetweetedStatus().getUser().getId();
					DB.AddParentUserToCrawl(rt_uid, u.gen);
				}

				// s.isRetweeted() doesn't seem to work. use getRetweetCount() instead.
				int rt_cnt = s.getRetweetCount();
				IDs c_uids = null;
				if (rt_cnt > 0) {
					//StdoutWriter.W(String.format("The tweet is retweeted. Getting child users: id=%d rt_cnt=%d", id, rt_cnt));
					long cursor = -1;
					while (true) {
						long sleep_time1 = 1000;
						while (true) {
							try {
								if (_stop_requested) return;
								c_uids = _tpt.twitter.getRetweeterIds(id, 200, cursor);
								_tpt.IncReqMade();
								break;
							} catch (TwitterException e) {
								if (e.exceededRateLimitation()) {
									int sec_until_reset = e.getRateLimitStatus().getSecondsUntilReset();
									//StdoutWriter.W(String.format("rate-limited: %s", e));
									_tpt.SetRateLimitedAndWait(sec_until_reset);
									_tpt = TwitterPool.GetTwitter();
								} else if (e.getStatusCode() == HttpResponseCode.UNAUTHORIZED) {
									// 401: UNAUTHORIZED
									if (e.getErrorCode() == 89) {
										// "Invalid or expired token"
										StdoutWriter.W(String.format("uid=%d token=%s TwitterException: [%s]",
													u.id, _tpt.tc.token, e));
										System.exit(0);
									}
									DB.SetUserUnauthorized(u);
									return;
								} else if (e.getStatusCode() == HttpResponseCode.NOT_FOUND) {
									DB.SetUserNotFound(u);
									return;
								} else {
									// It can be 130 (Over capacity), 131 (Internal error), or anything.
									StdoutWriter.W(String.format("uid=%d token=%s TwitterException=[%s]\n"
												+ "Waiting %d ms and retrying ...",
												u.id, _tpt.tc.token, e, sleep_time1));
									Mon.Sleep(sleep_time1);
									sleep_time1 *= 2;
								}
							}
						}
						DB.AddChildUsersToCrawl(c_uids.getIDs(), u.gen);

						if (c_uids.hasNext()) {
							cursor = c_uids.getNextCursor();
						} else {
							break;
						}
					}
				}

				DB.AddTweet(id, u.id, ca, known_gl, youtube_video_id, rt_id, rt_uid);
			}
			if (statuses.size() == 0 || min_id == -1 || hit_oldest_date) {
				DB.SetUserCrawled(u);
				break;
			}
			//StdoutWriter.W(String.format("min_id=%d", min_id));
			max_id = min_id - 1;
		}
	}

	static String _IDsToStr(IDs c_uids) {
		if (c_uids == null)
			return null;
		long[] ids = c_uids.getIDs();
		if (ids.length == 0)
			return null;
		StringBuilder sb = new StringBuilder();
		boolean first = true;
		for(long i: ids) {
			if (first) {
				first = false;
			} else {
				sb.append(" ");
			}
			sb.append(Long.toString(i));
		}
		return sb.toString();
	}
}
