package crawltwitter;

import java.sql.SQLException;
import java.util.ArrayList;
import java.util.List;
import twitter4j.FilterQuery;
import twitter4j.GeoLocation;
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
  static private TwitterPool.TS _ts = null;

  static void Start() throws Exception {
    StatusListener listener = new StatusListener() {
      @Override
      public void onStatus(Status status) {
        try {
          do {
            //StdoutWriter.W(status.toString());
            //StdoutWriter.W("@" + status.getUser().getScreenName() + " [" + status.getText() + "]");
            Mon.num_users_to_crawl_streamed ++;

            // Set a heartbeat to indicate the Twitter credential is being used.
            _ts.SendHeartbeat();

            // Do not skip young (recently joined) users. Used to skip.
            //if (status.getUser().getCreatedAt().after(Conf.user_ca_oldest_date))
            //	break;

            if (false) {
              // We are interested in retweets only. Hmm. Not sure. It doesn't have to.
              if (! status.isRetweet())
                break;

              // Check if the parent tweet has location. Retweets does not have geolocation.
              // https://dev.twitter.com/docs/streaming-apis/parameters#locations
              if (status.getRetweetedStatus().getGeoLocation() == null)
                break;
            }

            // When there are enough seeded users (more than 50000), only add users with Tweets that have a Youtube link.
            // When there are not enough seeded users, add users with or without Tweets that have a Youtube link.
            String youtube_video_id = null;
            if (DB.GetNumUncrawledUsers() > 1000) {
              for (URLEntity e: status.getURLEntities()) {
                youtube_video_id = Filter.YouTubeLink(e.getExpandedURL());
                if (youtube_video_id != null)
                  break;
              }
              if (youtube_video_id == null)
                continue;
            }

            if (false) {
              // Geolocation is null even when it shows the city name of where it was tweeted. Dropped this condition.
              //   https://twitter.com/Jlayden15John/status/879354795781500931
              // Check if the location is in the US. This will help reduce the database size big time.
              GeoLocation gl = status.getGeoLocation();
              if (gl == null)
                break;
              if (! UsaMap.Contains(gl.getLongitude(), gl.getLatitude()))
                break;
            }

            User u = status.getUser();
            long uid = u.getId();
            //StdoutWriter.W(String.format("Seed user: %d %s %d %s", uid, u.getScreenName(), status.getId(), youtube_video_id));
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
        // Can be safely ignored
        //StdoutWriter.W("Got track limitation notice:" + numberOfLimitedStatuses);
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
    _ts.ts.addListener(listener);
    FilterQuery fq = new FilterQuery();
    String[] keywordsArray = {"youtube"};
    // Bounding box of the contiguous USA
    double[][] locations = { { -124.848974d, 24.396308d }, { -66.885444d, 49.384358d } };

    // Twitter doesn't filter by AND. They do OR.
    //fq.track(keywordsArray);
    fq.locations(locations);

    _ts.ts.filter(fq);
  }

  static public void Stop() {
    if (_ts != null) {
      _ts.ts.cleanUp();
      _ts.ts.shutdown();
      StdoutWriter.W("StreamSeedUsers stopped.");
    }
  }
}
