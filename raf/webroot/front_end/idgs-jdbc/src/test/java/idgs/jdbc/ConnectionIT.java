/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs.jdbc;

import java.sql.Connection;
import java.sql.DriverManager;
import java.sql.SQLException;
import java.util.Properties;

public class ConnectionIT extends BaseCase {

  public void testConnectionWithDefault() {
    Connection conn = null;
    
    try {
      conn = DriverManager.getConnection("");
      assertTrue(conn != null);
    } catch (SQLException ex) {
      ex.printStackTrace();
      fail();
    } finally {
      if (conn != null) {
        try {
          conn.close();
        } catch (SQLException e) {
          e.printStackTrace();
        }
      }
    }
  }

  public void testConnectionWithUrl() {
    String url = "jdbc:idgs://127.0.0.1:7700";
    
    Connection conn = null;
    try {
      conn = DriverManager.getConnection(url);
      assertTrue(conn != null);
    } catch (SQLException ex) {
      ex.printStackTrace();
      fail();
    } finally {
      if (conn != null) {
        try {
          conn.close();
        } catch (SQLException e) {
          e.printStackTrace();
        }
      }
    }
  }
  
  public void testConnectionWithSpecifiedClinetConfig() {
    String url = "jdbc:idgs://";
    
    Connection conn = null;
    
    String clientCfgFile = "conf/client_8800.conf";
    Properties props = new Properties();
    props.setProperty(IdgsJdbcDriver.CLIENT_CONFIG_FILE_PATH, clientCfgFile);
    
    try {
      conn = DriverManager.getConnection(url, props);
    } catch (SQLException ex) {
      ex.printStackTrace();
      fail();
    } finally {
      if (conn != null) {
        try {
          conn.close();
        } catch (SQLException e) {
          e.printStackTrace();
        }
      }
    }
  }
  
  public void testConnectionWithSpecifiedStoreConfig() {
    String url = "jdbc:idgs://127.0.0.1:7700";
    Connection conn = null;
    
    String storeCfgFile = "conf/sync_data_store.conf";
    Properties props = new Properties();
    props.setProperty(IdgsJdbcDriver.STORE_CONFIG_FILE_PATH, storeCfgFile);
    
    try {
      conn = DriverManager.getConnection(url, props);
    } catch (SQLException ex) {
      ex.printStackTrace();
      fail();
    } finally {
      if (conn != null) {
        try {
          conn.close();
        } catch (SQLException e) {
          e.printStackTrace();
        }
      }
    }
  }
  
}
