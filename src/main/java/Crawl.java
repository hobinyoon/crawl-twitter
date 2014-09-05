package crawltwitter;


public final class Crawl {
	static void Cleanup() {
		System.out.println("\nCleaning up ...");
		DB.Close();
		StdoutWriter.Stop();
	}

	public static void main(String[] args) {
		try {
			Runtime.getRuntime().addShutdownHook(new Thread() {
				@Override
				public void run() {
					Cleanup();
				}
			});

			StdoutWriter.Run();
			CrawlSeedUsers.Run();
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("Exception caught: " + e);
			System.exit(-1);
		}
	}
}
