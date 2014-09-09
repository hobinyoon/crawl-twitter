package crawltwitter;


public final class Crawl {
	static void Cleanup() {
		System.out.println("\nCleaning up ...");
		StreamSeedUsers.Stop();
		CrawlTweets.Stop();
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

			// It seems that this needs to go through the end of the main function.
			// strange thread model.
			StreamSeedUsers.Run();

			CrawlTweets.Run();
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("Exception caught: " + e);
			System.exit(-1);
		}
	}
}
