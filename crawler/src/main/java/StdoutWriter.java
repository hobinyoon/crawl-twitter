package crawltwitter;

import java.lang.InterruptedException;
import java.text.SimpleDateFormat;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.Calendar;
import java.util.Date;

public class StdoutWriter {
	private static volatile boolean _started = false;
	private static int update_freq = 5;	// updates per sec
	private static final Lock _lock = new ReentrantLock();
	private static volatile boolean _stop_requested = false;
	private static Thread _t = null;
	private static volatile boolean _status_written = false;
	private static Integer _wait_obj = new Integer(0);

	static {
		Start();
	}

	static void Start() {
		if (_started)
			return;

		_t = new Thread() {
			public void run() {
				while (! _stop_requested) {
					_UpdateStatus();

					synchronized (_wait_obj) {
						try {
							_wait_obj.wait(1000 / update_freq);
						} catch (java.lang.InterruptedException e) {
							// Ignore
						}
					}
				}
			}
		};
		_t.start();
		_started = true;
	}

	static long _ts_sleep_amount = 0;
	static long _ts_last_sleep_amount = 0;

	static long _ts_cur_sleep_amount = 0;

	static void _UpdateStatus() {
		_lock.lock();
		try {
			if (_status_written)
				Util.ClearLine(4);
			System.out.printf("%s %s %s\n",
						new SimpleDateFormat("HH:mm:ss").format(Calendar.getInstance().getTime()),
						Mon.Status(), Mon.RuntimeSleeptimeStr());
			System.out.printf("         crawled: tc=%d tn=%d uc=%d\n",
						Mon.num_crawled_tweets,
						Mon.num_crawled_tweets_new,
						Mon.num_crawled_users);
			System.out.printf("         crawling: usr=%s cred=%d,%s\n",
						Mon.user_being_crawled,
						Mon.num_credentials_used,
						CrawlTweets.GetCredTokenLast4());
			if (Conf.stream_seed_users) {
				System.out.printf("         to_crawl: s=%d sn=%d sd=%d pn=%d pd=%d cn=%d cd=%d ",
							Mon.num_users_to_crawl_streamed,
							Mon.num_users_to_crawl_streamed_new,
							Mon.num_users_to_crawl_streamed_dup,
							Mon.num_users_to_crawl_parent_new,
							Mon.num_users_to_crawl_parent_dup,
							Mon.num_users_to_crawl_child_new,
							Mon.num_users_to_crawl_child_dup);
			} else {
				System.out.printf("         to_crawl: pn=%d pd=%d cn=%d cd=%d ",
							Mon.num_users_to_crawl_parent_new,
							Mon.num_users_to_crawl_parent_dup,
							Mon.num_users_to_crawl_child_new,
							Mon.num_users_to_crawl_child_dup);
			}
			System.out.flush();
			_status_written = true;
		} finally {
			_lock.unlock();
		}
	}

	public static void W(String s) {
		_lock.lock();
		try {
			if (_status_written) {
				System.out.println("");
				_status_written = false;
			}
			System.out.println(s);
		} finally {
			_lock.unlock();
		}
	}

	public static void Stop() {
		try {
			_stop_requested = true;
			synchronized (_wait_obj) {
				_wait_obj.notifyAll();
			}
			_t.join();
			Util.ClearLine();
			System.out.println("StdoutWriter stopped.");
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
}
