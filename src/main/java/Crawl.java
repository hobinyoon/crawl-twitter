package crawltwitter;


public final class Crawl {
	static void Cleanup() {
		System.out.println("Cleaning up ...");
		DB.Close();
	}

	public static void main(String[] args) {
		try {
			Runtime.getRuntime().addShutdownHook(new Thread() {
				@Override
				public void run() {
					Cleanup();
				}
			});

			DB.Insert();

			//GetSeedUsers.Run();

			//Practice.GetFriendsIDs();
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("Exception caught: " + e);
			System.exit(-1);
		}
	}
}
