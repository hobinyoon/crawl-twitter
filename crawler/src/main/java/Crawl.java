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

			Conf.ParseArgs(args);
			StdoutWriter.Start();
			DB.Init();

			if (Conf.stream_seed_users)
				StreamSeedUsers.Start();

			CrawlTweets.Run();
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("Exception caught: " + e);
			System.exit(1);
		}
	}
}
