package crawltwitter;


public class Util {
	static final String ESC = "\033[";

	static void ClearLine() {
		ClearLine(1);
	}

	static void ClearLine(int num_lines) {
		// http://en.wikipedia.org/wiki/ANSI_escape_code
		System.out.print(ESC + "1K");	// clear from cursor to beginning of the line
		for (int i = 2; i <= num_lines; i ++) {
			System.out.print(ESC + "1A");	// move up
			System.out.print(ESC + "1K");	// clear from cursor to beginning of the line
		}
		System.out.print(ESC + "1G"); // move the cursor to column 1
	}
}
