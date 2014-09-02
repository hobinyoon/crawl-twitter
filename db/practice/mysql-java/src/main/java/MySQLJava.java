package mysqlclient;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.logging.Level;
import java.util.logging.Logger;

public final class MySQLJava {

	static void Insert(Connection conn) throws SQLException {
		PreparedStatement ps = null;
		String qs = "INSERT INTO followers (id, followers) VALUES (?, ?)";
		ps = conn.prepareStatement(qs);
		ps.setLong(1, 123);
		{
			StringBuilder sb = new StringBuilder();
			for (int i = 0; i < 1000; i ++) {
				sb.append(Integer.toString(i + 5));
				sb.append(" ");
			}
			ps.setString(2, sb.toString());
		}
		ps.executeUpdate();
		conn.commit();

		if (ps != null) {
			ps.close();
		}
	}

	static void Select(Connection conn) throws SQLException {
		Statement stmt = conn.createStatement();
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

	public static void main(String[] args) {
		String cs = "jdbc:mysql://localhost:3306/twitter";
		String user = "twitter";
		String password = "twitterpass";

		Connection conn = null;

		try {
			conn = DriverManager.getConnection(cs, user, password);
			conn.setAutoCommit(false);
			//Insert(conn);
			Select(conn);
		} catch (SQLException ex) {
			Logger lgr = Logger.getLogger(MySQLJava.class.getName());
			lgr.log(Level.SEVERE, ex.getMessage(), ex);
		} finally {
			try {
				if (conn != null) {
					conn.close();
				}
			} catch (SQLException ex) {
				Logger lgr = Logger.getLogger(MySQLJava.class.getName());
				lgr.log(Level.SEVERE, ex.getMessage(), ex);
			}
		}
	}
}
