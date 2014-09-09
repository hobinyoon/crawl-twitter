package crawltwitter;

import java.lang.InterruptedException;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class StdoutWriter {
	private static int update_freq = 5;	// updates per sec
	private static final Lock _lock = new ReentrantLock();
	private static volatile boolean _stop_requested = false;
	private static Thread _t = null;

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
			System.out.print(String.format("seed_users: new=%d dup=%d crawled=%d children: inserted_tweets=%d crawled_tweets=%d crawled_users=%d",
						Mon.num_seed_users_new,
						Mon.num_seed_users_dup,
						Mon.num_seed_users_streamed,
						Mon.num_crawled_tweets_inserted_to_db,
						Mon.num_crawled_tweets,
						Mon.num_crawled_child_users));
			System.out.flush();
		} finally {
			_lock.unlock();
		}
	}

	public static void W(String s) {
		_lock.lock();
		try {
			Util.ClearLine();
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
