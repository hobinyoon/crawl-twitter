package crawltwitter;

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

	static public long current_uid = 0;

	static public long num_credentials_used = 0;

	static public long ts_begin_sleep = -1;

	static {
	}
}
