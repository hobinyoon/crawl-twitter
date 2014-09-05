package crawltwitter;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import com.mysql.jdbc.MysqlErrorNumbers;


public class DB {
	static private String _cs = "jdbc:mysql://localhost:3306/twitter";
	static private String _user = "twitter";
	static private String _password = "twitterpass";
	static private Connection _conn = null;

	static {
		try {
			_conn = DriverManager.getConnection(_cs, _user, _password);
			_conn.setAutoCommit(false);
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("Exception caught: " + e);
			System.exit(-1);
		}
	}

	static void Close() {
		try {
			if (_conn != null) {
				_conn.close();
			}
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("Exception caught: " + e);
			System.exit(-1);
		}
	}

	static void Insert() throws SQLException {
		PreparedStatement ps = null;
		String qs = "INSERT INTO child_ids_to_crawl (id) VALUES (?)";
		try {
			ps = _conn.prepareStatement(qs);
			ps.setLong(1, 123);
			ps.executeUpdate();
			_conn.commit();
		} catch (SQLException e) {
			if (e.getErrorCode() == MysqlErrorNumbers.ER_DUP_ENTRY) {
				//System.out.println("Duplicate entry: " + e.getMessage());
				System.out.println(e);
			} else
				throw e;
		} finally {
			if (ps != null) {
				ps.close();
			}
		}
	}

	static void Select() throws SQLException {
		Statement stmt = _conn.createStatement();
		String q = "SELECT * FROM followers";
		ResultSet rs = stmt.executeQuery(q);
		while (rs.next()) {
			long id = rs.getInt("id");
			String followers = rs.getString("followers");
			System.out.printf("%d %s\n", id, followers);
		}
		if (stmt != null) {
			stmt.close();
		}
	}
}
