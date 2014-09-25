package crawltwitter;

public class Filter {
	public static String YouTubeLink(String url) {
		if (! url.toLowerCase().contains("youtube."))
			return null;
		int pos = url.indexOf("v=");
		if (pos == -1)
			return null;
		if (url.length() < pos + 2 + 11)
			return null;
		return url.substring(pos + 2, pos + 2 + 11);
	}
}
