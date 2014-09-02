package mysqlclient;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.PreparedStatement;
import java.sql.SQLException;
import java.sql.Statement;
import java.util.logging.Level;
import java.util.logging.Logger;

public final class MySQLJava {
	public static void main(String[] args) {
		String cs = "jdbc:mysql://localhost:3306/twitter";
		String user = "twitter";
		String password = "twitterpass";

		Connection conn = null;
		PreparedStatement ps = null;

		try {
			conn = DriverManager.getConnection(cs, user, password);
			conn.setAutoCommit(false);

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
		} catch (SQLException ex) {
			Logger lgr = Logger.getLogger(MySQLJava.class.getName());
			lgr.log(Level.SEVERE, ex.getMessage(), ex);
		} finally {
			try {
				if (ps != null) {
					ps.close();
				}
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
