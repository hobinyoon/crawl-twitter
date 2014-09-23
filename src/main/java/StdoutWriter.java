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

	static {
		Start();
	}

	static void Start() {
		if (_started)
			return;
		_t = new Thread() {
			public void run() {
				try {
					while (! _stop_requested) {
						_UpdateStatus();
						Thread.sleep(1000 / update_freq);
					}
				} catch (InterruptedException e) {
					;
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
			Util.ClearLine();

			long cur_ts = (new Date()).getTime();
			if (Mon.ts_begin_sleep != -1) {
				_ts_last_sleep_amount = cur_ts - Mon.ts_begin_sleep;
			} else {
				_ts_sleep_amount += _ts_last_sleep_amount;
				_ts_last_sleep_amount = 0;
			}
			long ts_sleep_amount = _ts_sleep_amount + _ts_last_sleep_amount;
			long ts_runtime = cur_ts - Conf.ts_start;

			if (Conf.stream_seed_users) {
				System.out.print(String.format("%s "
							+ "r=%d s=%d %.1f%% "
							+ "to_crawl: s=%d sn=%d sd=%d pn=%d pd=%d cn=%d cd=%d "
							+ "crawled: tc=%d tn=%d tni=%d uc=%d "
							+ "cur_uid=%d cred_used=%d",
							new SimpleDateFormat("HH:mm:ss").format(Calendar.getInstance().getTime()),
							ts_runtime / 1000, ts_sleep_amount / 1000, 100.0 * (ts_runtime - ts_sleep_amount) / ts_runtime,
							Mon.num_users_to_crawl_streamed,
							Mon.num_users_to_crawl_streamed_new,
							Mon.num_users_to_crawl_streamed_dup,
							Mon.num_users_to_crawl_parent_new,
							Mon.num_users_to_crawl_parent_dup,
							Mon.num_users_to_crawl_child_new,
							Mon.num_users_to_crawl_child_dup,
							Mon.num_crawled_tweets,
							Mon.num_crawled_tweets_new,
							Mon.num_crawled_tweets_new_imported,
							Mon.num_crawled_users,
							Mon.current_uid,
							Mon.num_credentials_used));
			} else {

				System.out.print(String.format("%s "
							+ "r=%d s=%d %.1f%% "
							+ "to_crawl: pn=%d pd=%d cn=%d cd=%d "
							+ "crawled: tc=%d tn=%d tni=%d uc=%d "
							+ "cur_uid=%d cred_used=%d",
							new SimpleDateFormat("HH:mm:ss").format(Calendar.getInstance().getTime()),
							ts_runtime / 1000, ts_sleep_amount / 1000, 100.0 * (ts_runtime - ts_sleep_amount) / ts_runtime,
							Mon.num_users_to_crawl_parent_new,
							Mon.num_users_to_crawl_parent_dup,
							Mon.num_users_to_crawl_child_new,
							Mon.num_users_to_crawl_child_dup,
							Mon.num_crawled_tweets,
							Mon.num_crawled_tweets_new,
							Mon.num_crawled_tweets_new_imported,
							Mon.num_crawled_users,
							Mon.current_uid,
							Mon.num_credentials_used));
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
			_t.interrupt();
			_t.join();
			Util.ClearLine();
			System.out.println("StdoutWriter stopped.");
		} catch (InterruptedException e) {
			e.printStackTrace();
		}
	}
}
