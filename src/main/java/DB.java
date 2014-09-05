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
	static private PreparedStatement _ps_insert_user = null;

	static {
		try {
			_conn = DriverManager.getConnection(_cs, _user, _password);
			_conn.setAutoCommit(false);
			_ps_insert_user = _conn.prepareStatement("INSERT INTO child_ids_to_crawl (id) VALUES (?)");
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("Exception caught: " + e);
			System.exit(-1);
		}
	}

	static void Close() {
		try {
			if (_ps_insert_user != null)
				_ps_insert_user.close();
			if (_conn != null)
				_conn.close();
		} catch (Exception e) {
			e.printStackTrace();
			System.out.println("Exception caught: " + e);
			System.exit(-1);
		}
	}

	static boolean InsertUser(long uid) throws SQLException {
		try {
			_ps_insert_user.setLong(1, uid);
			_ps_insert_user.executeUpdate();
			_conn.commit();
			return true;
		} catch (SQLException e) {
			if (e.getErrorCode() == MysqlErrorNumbers.ER_DUP_ENTRY) {
				//System.out.println("Duplicate entry: " + e.getMessage());
				System.out.println(e);
			} else
				throw e;
		}
		return false;
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
