package crawltwitter;

import java.lang.InterruptedException;
import java.text.SimpleDateFormat;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;
import java.util.Calendar;

public class StdoutWriter {
	private static int update_freq = 5;	// updates per sec
	private static final Lock _lock = new ReentrantLock();
	private static volatile boolean _stop_requested = false;
	private static Thread _t = null;
	private static volatile boolean _status_written = false;

	public static void Run() {
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
	}

	static void _UpdateStatus() {
		_lock.lock();
		try {
			Util.ClearLine();
			System.out.print(String.format("%s seed_users: new=%d dup=%d crawled=%d children: inserted_tweets=%d crawled_tweets=%d crawled_users=%d cur_uid=%d",
						new SimpleDateFormat("HH:mm:ss").format(Calendar.getInstance().getTime()),
						Mon.num_seed_users_new,
						Mon.num_seed_users_dup,
						Mon.num_seed_users_streamed,
						Mon.num_crawled_tweets_inserted_to_db,
						Mon.num_crawled_tweets,
						Mon.num_crawled_child_users,
						Mon.current_c_uid));
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
