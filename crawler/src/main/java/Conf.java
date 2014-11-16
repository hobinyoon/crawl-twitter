package crawltwitter;

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
	public static boolean stream_seed_users;
	public static String db_ipaddr = null;

	public static String db_url = null;
	public static String db_user = "twitter";
	public static String db_pass = "twitterpass";

	public static String fn_twitter_credential_stream = System.getProperty("user.home") + "/private/.twitter_auth/crawl-seed-users";
	public static String fn_twitter_credentials = System.getProperty("user.home") + "/private/.twitter_auth/credentials";

	public static Date user_ca_oldest_date = null;
	public static Date tweet_oldest_date = null;
	public static Date tweet_youngest_date = null;

	public static long cred_rate_limit_wait_cushion_in_milli = 5000;

	public static String ip = null;

	public static long cred_auth_fail_retry_wait_sec = 3600;	// 1 hour

	public static int NEXT_CHECK_OUT_AFTER_SEC = 3600;

	static {
		Calendar cal = Calendar.getInstance();
		cal.set(2013, 8, 1, 0, 0, 0);	// 8 is September
		user_ca_oldest_date = cal.getTime();
		//System.out.println(user_ca_oldest_date);

		tweet_oldest_date = user_ca_oldest_date;
		cal.set(2014, 8, 1, 0, 0, 0);
		tweet_youngest_date = cal.getTime();

		try {
			ip = _GetIPv4Addr();
		} catch (SocketException e) {
			e.printStackTrace();
			System.out.println("Exception caught: " + e);
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
