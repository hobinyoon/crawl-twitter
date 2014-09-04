package crawltwitter;


public class Util {
	static void ClearLine() {
		// http://en.wikipedia.org/wiki/ANSI_escape_code
		final String ESC = "\033[";
		System.out.print(ESC + "1K");	// clear from cursor to beginning of the line
		System.out.print(ESC + "1G"); // move the cursor to column 1
		System.out.flush();
	}
}
