package crawltwitter;

import java.net.InetAddress;
import java.net.NetworkInterface;
import java.net.SocketException;
import java.util.Enumeration;
import java.util.Iterator;
import java.util.List;

import joptsimple.OptionParser;
import joptsimple.OptionSet;

public final class Conf {
	public static String ip = null;
	public static String token = null;
	public static String token_sec = null;
	public static String consumer = null;
	public static String consumer_sec = null;

	static {
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
		if (nonop_args.size() != 4) {
			_PrintHelp();
			System.exit(1);
		}

		Iterator<?> iter = nonop_args.iterator();
		token = (String) iter.next();
		token_sec = (String) iter.next();
		consumer = (String) iter.next();
		consumer_sec = (String) iter.next();

		System.out.printf("Conf:\n");
		System.out.printf("  my ip addr  : %s\n", ip);
		System.out.printf("  token       : %s\n", token);
		System.out.printf("  token_sec   : %s\n", token_sec);
		System.out.printf("  consumer    : %s\n", consumer);
		System.out.printf("  consumer_sec: %s\n", consumer_sec);
	}

	private static final OptionParser _opt_parser = new OptionParser() {{
		accepts("help", "Show this help message");
	}};

	private static void _PrintHelp() throws java.io.IOException {
		System.out.println("Usage: Crawl [<option>] token token_sec consumer consumer_sec");
		_opt_parser.printHelpOn(System.out);
	}
}
