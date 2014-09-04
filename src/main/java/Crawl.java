package crawltwitter;


public final class Crawl {
	public static void main(String[] args) {
		try {
			GetSeedUsers.Run();
			//Practice.GetFriendsIDs();
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("Exception caught: " + e);
			System.exit(-1);
		}
	}
}
