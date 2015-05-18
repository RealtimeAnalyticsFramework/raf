package idgs.jdbc;

import junit.framework.TestCase;

public class BaseCase extends TestCase {

  private static final String driverName = "idgs.jdbc.IdgsJdbcDriver";
  
  static {
    try {
      Class.forName(driverName);
    } catch (ClassNotFoundException e) {
      e.printStackTrace();
    }
  }
  
}
