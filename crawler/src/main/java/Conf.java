package crawltwitter;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.net.UnknownHostException;
import java.util.Calendar;
import java.util.Date;
import java.util.Enumeration;
import java.util.List;

import joptsimple.OptionParser;
import joptsimple.OptionSet;

public final class Conf {
	public static boolean stream_seed_users = false;
	public static String db_ipaddr = null;

	public static String db_url = null;
	public static String db_user = "twitter";
	public static String db_pass = null;

	public static Date tweet_oldest_date = null;
	public static Date tweet_youngest_date = null;

	public static long cred_rate_limit_wait_cushion_in_milli = 5000;

	public static String ip = null;

	public static long cred_auth_fail_retry_wait_sec = 3600;	// 1 hour

	public static int NEXT_CHECK_OUT_AFTER_SEC = 3600;

	private static void _Init() {
		try {
			// Read the password
			{
				String fn = String.format("%s/.my.conf", System.getProperty("user.home"));
				//String fn = String.format("%s/.my.cnf", System.getProperty("user.home"));
				try (BufferedReader br = new BufferedReader(new FileReader(fn))) {
					String line;
					while ((line = br.readLine()) != null) {
						StdoutWriter.W(line);
					}
				}

				// If this were a static initialization block, you can't call System.exit() here.
				// It triggers ShutdownHook of the main thread, which calls StdoutWriter.Stop(),
				// which looks at Conf.stream_seed_users, which is, I think, not ready yet,
				// because this static initialization block is not finished.
				//
				// When it's a regular function, it's okay. There's no such deadlock.
				System.exit(1);
			}

			Calendar cal = Calendar.getInstance();
			// TODO: Use a YAML configuration file
			cal.set(2016, 5, 1, 0, 0, 0);	// 5 is June. Not intuitive.
			tweet_oldest_date = cal.getTime();

			cal.set(2017, 5, 21, 0, 0, 0);
			tweet_youngest_date = cal.getTime();

			ip = _GetIPv4Addr();
		} catch (Exception e) {
			StringWriter sw = new StringWriter();
			e.printStackTrace(new PrintWriter(sw));
			StdoutWriter.W(String.format("Exception: %s\nStack trace: %s", e, sw));
			System.exit(1);
		}
	}

	private static String _GetIPv4Addr() throws SocketException {
		Enumeration<NetworkInterface> interfaces = NetworkInterface.getNetworkInterfaces();
		while (interfaces.hasMoreElements()) {
			NetworkInterface iface = interfaces.nextElement();
			// filters out 127.0.0.1 and inactive interfaces
			if (iface.isLoopback() || !iface.isUp())
				continue;

			Enumeration<InetAddress> addresses = iface.getInetAddresses();
			while(addresses.hasMoreElements()) {
				InetAddress addr = addresses.nextElement();
				String ip = addr.getHostAddress();
				// get IPv4 address only
				if (ip.matches("\\d+\\.\\d+\\.\\d+\\.\\d+"))
					return ip;
			}
		}
		return null;
	}

	public static void ParseArgs(String[] args)
		throws java.io.IOException, java.text.ParseException, java.lang.InterruptedException {
		_Init();

		OptionSet options = _opt_parser.parse(args);
		if (options.has("help")) {
			_PrintHelp();
			System.exit(0);
		}
		List<?> nonop_args = options.nonOptionArguments();
		if (nonop_args.size() != 0) {
			_PrintHelp();
			System.exit(1);
		}

		stream_seed_users = options.has("stream_seed_users");
		db_ipaddr = (String) options.valueOf("db_ipaddr");
		db_url = String.format("jdbc:mysql://%s:3306/twitter3?useUnicode=true&characterEncoding=utf-8", Conf.db_ipaddr);

		System.out.printf("Conf:\n");
		System.out.printf("  my ip addr: %s\n", ip);
		System.out.printf("  stream_seed_users: %b\n", stream_seed_users);
		System.out.printf("  db_ipaddr: %s\n", db_ipaddr);
	}

	private static final OptionParser _opt_parser = new OptionParser() {{
		accepts("help", "Show this help message");
		accepts("stream_seed_users", "Stream seed users");
		accepts("db_ipaddr", "IP address of DB")
			.withRequiredArg().ofType(String.class).defaultsTo("localhost");
	}};

	private static void _PrintHelp() throws java.io.IOException {
		System.out.println("Usage: Crawl [<option>]");
		_opt_parser.printHelpOn(System.out);
	}
}
