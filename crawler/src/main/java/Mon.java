package crawltwitter;

import java.util.Date;

public class Mon {
	static public long num_users_to_crawl_streamed = 0;
	static public long num_users_to_crawl_streamed_new = 0;
	static public long num_users_to_crawl_streamed_dup = 0;
	static public long num_users_to_crawl_parent_new = 0;
	static public long num_users_to_crawl_parent_dup = 0;
	static public long num_users_to_crawl_child_new = 0;
	static public long num_users_to_crawl_child_dup = 0;

	static public long num_crawled_tweets = 0;
	static public long num_crawled_tweets_new = 0;
	static public long num_crawled_tweets_new_imported = 0;
	static public long num_crawled_users = 0;

	static public DB.UserToCrawl user_being_crawled = null;

	static public long num_credentials_used = 0;

	static private char status = 'R';	// R:running, S:sleeping
	static private long ts_start = -1;
	static private long cur_sleep_amount = 0;
	static private long sleep_amount = 0;
	static private long ts_begin_sleep = -1;

	static {
		status = 'R';
		ts_start = (new Date()).getTime();
	}

	static public void Sleep(long s) throws InterruptedException {
		status = 'S';
		cur_sleep_amount = s;
		ts_begin_sleep = (new Date()).getTime();
		Thread.sleep(s);
		ts_begin_sleep = -1;
		cur_sleep_amount = 0;
		sleep_amount += s;
		status = 'R';
	}

	static public String Status() {
		StringBuilder sb = new StringBuilder();
		sb.append(status);
		if (status == 'S') {
			sb.append("-");
			sb.append(cur_sleep_amount);
		}
		return sb.toString();
	}

	static public String RuntimeSleeptimeStr() {
		long cur_ts = (new Date()).getTime();
		long ts_runtime = cur_ts - ts_start;

		// sleep amount
		long sa = -1;
		if (Mon.status == 'R') {
			sa = sleep_amount;
		} else if (Mon.status == 'S') {
			sa = sleep_amount + (cur_ts - ts_begin_sleep);
		}

		return String.format("r=%d s=%d %.1f%%",
				ts_runtime / 1000, sa / 1000, 100.0 * (ts_runtime - sa) / ts_runtime);
	}
}
