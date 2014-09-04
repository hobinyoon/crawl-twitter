package crawltwitter;

import java.util.ArrayList;
import java.util.Calendar;
import java.util.Date;
import java.util.List;
import twitter4j.FilterQuery;
import twitter4j.StallWarning;
import twitter4j.Status;
import twitter4j.StatusDeletionNotice;
import twitter4j.StatusListener;
import twitter4j.Twitter;
import twitter4j.TwitterStream;
import twitter4j.TwitterStreamFactory;
import twitter4j.json.DataObjectFactory;


public class GetSeedUsers {
	static void Run() throws Exception {
		StatusListener listener = new StatusListener() {
			int cnt_all = 0;
			int cnt_needed = 0;
			Date oldest_date;

			{
				Calendar cal = Calendar.getInstance();
				cal.set(2013, 8, 1, 0, 0, 0);	// 8 means September
				oldest_date = cal.getTime();
				System.out.println(oldest_date);
			}

			@Override
			public void onStatus(Status status) {
				//System.out.println(status);
				//System.out.println("@" + status.getUser().getScreenName() + " [" + status.getText() + "]");

				cnt_all ++;

				do {
					// skip young users (created after the oldest_date)
					if (status.getUser().getCreatedAt().after(oldest_date))
						break;
					// we are interested in retweets only.
					if (! status.isRetweet())
						break;
					// Check if the parent tweet has location. Retweets does not have
					// geolocation.
					// https://dev.twitter.com/docs/streaming-apis/parameters#locations
					if (status.getRetweetedStatus().getGeoLocation() == null)
						break;

					cnt_needed ++;
					Util.ClearLine();
					User u = status.getUser();
					System.out.printf("%d %s\n", u.getId(), u.getScreenName());

					//System.out.println(DataObjectFactory.getRawJSON(status));
					//System.out.println(status);

					//	System.out.println("@" + status.getUser().getScreenName()
					//	+ " " + status.getUser().getCreatedAt()
					//	+ " " + status.getGeoLocation()
					//	+ " [" + status.getText() + "]");
					return;
				} while (false);

				Util.ClearLine();
				System.out.printf("%d %d", cnt_needed, cnt_all);
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

		TwitterStream ts = TwitterPool.GetNextTwitterStream();
		ts.addListener(listener);
		FilterQuery fq = new FilterQuery();
		String[] keywordsArray = {"youtube"};
		fq.track(keywordsArray);
		// double[][] locations = { { 40.714623d, -74.006605d }, { 42.3583d, -71.0603d } };
		// fq.locations(locations);
		// Twitter doesn't filter both with keywords and locations.
		ts.filter(fq);
	}
}
