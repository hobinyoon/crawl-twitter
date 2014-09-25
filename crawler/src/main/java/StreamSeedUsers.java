package crawltwitter;

import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;
import twitter4j.FilterQuery;
import twitter4j.StallWarning;
import twitter4j.Status;
import twitter4j.StatusDeletionNotice;
import twitter4j.StatusListener;
import twitter4j.Twitter;
import twitter4j.TwitterStream;
import twitter4j.TwitterStreamFactory;
import twitter4j.User;
import twitter4j.URLEntity;
import twitter4j.json.DataObjectFactory;


public class StreamSeedUsers {
	static private TwitterStream _ts = null;

	static void Start() throws Exception {
		StatusListener listener = new StatusListener() {
			@Override
			public void onStatus(Status status) {
				try {
					//System.out.println(status);
					//System.out.println("@" + status.getUser().getScreenName() + " [" + status.getText() + "]");
					Mon.num_users_to_crawl_streamed ++;

					do {
						// skip young users
						if (status.getUser().getCreatedAt().after(Conf.user_ca_oldest_date))
							break;
						// we are interested in retweets only.
						if (! status.isRetweet())
							break;
						// Check if the parent tweet has location. Retweets does not have
						// geolocation.
						// https://dev.twitter.com/docs/streaming-apis/parameters#locations
						if (status.getRetweetedStatus().getGeoLocation() == null)
							break;

						String youtube_video_id = null;
						for (URLEntity e: status.getURLEntities()) {
							youtube_video_id = Filter.YouTubeLink(e.getExpandedURL());
							if (youtube_video_id != null)
								break;
						}
						if (youtube_video_id == null)
							continue;

						User u = status.getUser();
						long uid = u.getId();
						//StdoutWriter.W(String.format("%d %s %d %s", uid, u.getScreenName(), status.getId(), youtube_video_id));
						DB.AddSeedUserToCrawl(uid);

						//System.out.println(DataObjectFactory.getRawJSON(status));
						//System.out.println(status);

						//	System.out.println("@" + status.getUser().getScreenName()
						//	+ " " + status.getUser().getCreatedAt()
						//	+ " " + status.getGeoLocation()
						//	+ " [" + status.getText() + "]");
						return;
					} while (false);
				} catch (SQLException e) {
					StdoutWriter.W(e.toString());
					e.printStackTrace();
					System.exit(1);
				}
			}

			@Override
			public void onDeletionNotice(StatusDeletionNotice statusDeletionNotice) {
				StdoutWriter.W("Got a status deletion notice id:" + statusDeletionNotice.getStatusId());
			}

			@Override
			public void onTrackLimitationNotice(int numberOfLimitedStatuses) {
				StdoutWriter.W("Got track limitation notice:" + numberOfLimitedStatuses);
			}

			@Override
			public void onScrubGeo(long userId, long upToStatusId) {
				StdoutWriter.W("Got scrub_geo event userId:" + userId + " upToStatusId:" + upToStatusId);
			}

			@Override
			public void onStallWarning(StallWarning warning) {
				StdoutWriter.W("Got stall warning:" + warning);
			}

			@Override
			public void onException(Exception ex) {
				ex.printStackTrace();
			}
		};

		_ts = TwitterPool.GetTwitterStream();
		_ts.addListener(listener);
		FilterQuery fq = new FilterQuery();
		String[] keywordsArray = {"youtube"};
		fq.track(keywordsArray);
		// double[][] locations = { { 40.714623d, -74.006605d }, { 42.3583d, -71.0603d } };
		// fq.locations(locations);
		// Twitter doesn't filter both with keywords and locations.
		_ts.filter(fq);
	}

	static public void Stop() {
		if (_ts != null) {
			_ts.cleanUp();
			_ts.shutdown();
			StdoutWriter.W("StreamSeedUsers stopped.");
		}
	}
}
