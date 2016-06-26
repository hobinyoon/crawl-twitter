package crawltwitter;


public final class TestCred {
	static void Cleanup() {
		System.out.println("Cleaning up ...");
		StreamSeedUsers.Stop();
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

			// It seems that this needs to go through the end of the main function.
			// strange thread model.
			StreamTweets.Start();
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("Exception caught: " + e);
			System.exit(1);
		}
	}
}
