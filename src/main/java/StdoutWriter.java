package crawltwitter;

import java.lang.InterruptedException;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

public class StdoutWriter {
	private static int update_freq = 5;	// updates per sec
	private static final Lock _lock = new ReentrantLock();
	private static volatile boolean _stop_requested = false;
	private static Thread _t = null;
	private static volatile String _status_update = null;

	public static void Run() {
		_t = new Thread() {
			public void run() {
				try {
					while (! _stop_requested) {
						_UpdateStatus();
						Thread.sleep(1000 / update_freq);
					}
				} catch (InterruptedException e) {
					System.out.println("Thread interrupted. Well... it's okay not to do anything on it.");
				}
			}
		};
		_t.start();
	}

	static void _UpdateStatus() {
		if (_status_update == null)
			return;

		_lock.lock();
		try {
			Util.ClearLine();
			System.out.print(_status_update);
			System.out.flush();
			_status_update = null;
		} finally {
			_lock.unlock();
		}
	}

	public static void Update(String s) {
		_status_update = s;
	}

	public static void W(String s) {
		_lock.lock();
		try {
			Util.ClearLine();
			System.out.println("");
			System.out.println(s);
		} finally {
			_lock.unlock();
		}
	}

	public static void Stop() {
		try {
			_stop_requested = true;
			_t.join();
			System.out.println("");
		} catch (InterruptedException e) {
			System.out.println("Thread interrupted. Well... it's okay not to do anything on it.");
		}
	}
}
