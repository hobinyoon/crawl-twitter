package crawltwitter;

import twitter4j.conf.ConfigurationBuilder;
import twitter4j.FilterQuery;
import twitter4j.StallWarning;
import twitter4j.Status;
import twitter4j.StatusDeletionNotice;
import twitter4j.StatusListener;
import twitter4j.TwitterStream;
import twitter4j.TwitterStreamFactory;


public class StreamTweets {
	static private TwitterStream _ts = null;
	static private int status_cnt = 0;
	static private boolean stop_requested = false;

	static void Start() throws Exception {
		StatusListener listener = new StatusListener() {
			@Override
			public void onStatus(Status status) {
				if (stop_requested)
					return;

				System.out.println(status.getUser().getScreenName() + " " + status.getText());
				status_cnt ++;
				if (status_cnt == 3) {
					System.out.println("The credential is working.");
					Stop();
				}
			}

			@Override
			public void onDeletionNotice(StatusDeletionNotice statusDeletionNotice) {
				System.out.println("Got a status deletion notice id:" + statusDeletionNotice.getStatusId());
			}

			@Override
			public void onTrackLimitationNotice(int numberOfLimitedStatuses) {
				System.out.println("Got track limitation notice:" + numberOfLimitedStatuses);
			}

			@Override
			public void onScrubGeo(long userId, long upToStatusId) {
				System.out.println("Got scrub_geo event userId:" + userId + " upToStatusId:" + upToStatusId);
			}

			@Override
			public void onStallWarning(StallWarning warning) {
				System.out.println("Got stall warning:" + warning);
			}

			@Override
			public void onException(Exception ex) {
				ex.printStackTrace();
			}
		};

		_ts = GetTwitterStream();
		_ts.addListener(listener);
		FilterQuery fq = new FilterQuery();
		String[] keywordsArray = {"youtube"};
		fq.track(keywordsArray);
		_ts.filter(fq);
	}

	static public void Stop() {
		stop_requested = true;
		if (_ts != null) {
			_ts.cleanUp();
			_ts.shutdown();
			System.out.println("StreamTweets stopped.");
		}
	}

	static TwitterStream GetTwitterStream() throws Exception {
		ConfigurationBuilder cb = new ConfigurationBuilder();
		cb.setDebugEnabled(true)
			.setJSONStoreEnabled(true)
			.setOAuthConsumerKey(Conf.consumer)
			.setOAuthConsumerSecret(Conf.consumer_sec)
			.setOAuthAccessToken(Conf.token)
			.setOAuthAccessTokenSecret(Conf.token_sec);
		TwitterStream ts = new TwitterStreamFactory(cb.build()).getInstance();
		return ts;
	}
}
