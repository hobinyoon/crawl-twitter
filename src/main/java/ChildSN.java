package crawltwitter;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.ArrayList;
import java.util.List;

public final class ChildSN {
	static List<String> sns = new ArrayList();

	public static void LoadParentsToCrawlToDB() throws Exception {
		try (Timing _ = new Timing()) {
			final File folder = new File(Conf.dn_children);
			for (final File fileEntry : folder.listFiles()) {
				if (! fileEntry.isFile())
					continue;
				_LoadSNsFromFile(fileEntry.getPath());
			}

			_WriteToDB();
		}
	}

	static void _LoadSNsFromFile(String fn) throws Exception {
		//System.out.println(fn);
		BufferedReader br = new BufferedReader(new FileReader(fn));
		String line;
		int lines_to_skip = 0;
		while ((line = br.readLine()) != null) {
			if (lines_to_skip > 0) {
				lines_to_skip --;
				continue;
			}

			String[] t = line.split(" ");
			sns.add(t[0]);
			lines_to_skip = Integer.parseInt(t[1]);
		}
		br.close();
		//System.out.printf("%d sns read\n", sns.size());
	}

	static void _WriteToDB() throws Exception {
		int cnt = 0;
		System.out.printf("cnt=%d\n", cnt);

		String cs = "jdbc:mysql://localhost:3306/twitter";
		Connection conn = null;

		conn = DriverManager.getConnection(cs, Conf.db_user, Conf.db_pass);
		conn.setAutoCommit(false);
		String qs = "INSERT INTO child_sns_to_crawl (sn) VALUES (?)";
		PreparedStatement ps = conn.prepareStatement(qs);

		for (String sn: sns) {
			ps.setString(1, sn);
			ps.executeUpdate();
			cnt ++;
			if (cnt % 100 == 0) {
				System.out.printf("cnt=%d %.2f%%\n", cnt, 100.0 * cnt / sns.size());
				conn.commit();
			}
		}

		conn.commit();
		if (conn != null)
			conn.close();
	}
}
