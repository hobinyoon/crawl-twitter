package crawltwitter;

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


public class GetSeedUsers {
	static void Run() throws Exception {
		StatusListener listener = new StatusListener(){
			int cnt_all = 0;
			int cnt_needed = 0;

			@Override
			public void onStatus(Status status) {
				//System.out.println(status);
				//System.out.println("@" + status.getUser().getScreenName() + " [" + status.getText() + "]");

				cnt_all ++;
				if (status.getGeoLocation() == null) {
					Util.ClearLine();
					System.out.printf("%d %d", cnt_needed, cnt_all);
				} else {
					cnt_needed ++;
					Util.ClearLine();
					System.out.println("@" + status.getUser().getScreenName() + " " + status.getGeoLocation() + " [" + status.getText() + "]");
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
