/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
package idgs;

import idgs.client.TcpClientPool;
import idgs.exception.IdgsException;
import idgs.execution.ResultSet;
import idgs.metadata.StoreLoader;
import idgs.metadata.StoreMetadata;
import idgs.util.LOGUtils;
import idgs.util.ServerConst;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.PrintStream;
import java.io.UnsupportedEncodingException;
import java.lang.reflect.Method;
import java.net.URL;
import java.net.URLClassLoader;
import java.sql.Date;
import java.sql.SQLException;
import java.sql.Timestamp;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

import jline.ConsoleReader;
import jline.History;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.conf.Configuration;
import org.apache.hadoop.hive.cli.CliDriver;
import org.apache.hadoop.hive.cli.CliSessionState;
import org.apache.hadoop.hive.cli.OptionsProcessor;
import org.apache.hadoop.hive.conf.HiveConf;
import org.apache.hadoop.hive.metastore.api.FieldSchema;
import org.apache.hadoop.hive.ql.CommandNeedRetryException;
import org.apache.hadoop.hive.ql.Driver;
import org.apache.hadoop.hive.ql.parse.VariableSubstitution;
import org.apache.hadoop.hive.ql.processors.CommandProcessor;
import org.apache.hadoop.hive.ql.processors.CommandProcessorFactory;
import org.apache.hadoop.hive.ql.session.SessionState;
import org.apache.hadoop.hive.ql.session.SessionState.LogHelper;

public class IdgsCliDriver extends CliDriver {

  private static Log LOG = LogFactory.getLog(IdgsCliDriver.class);

  private static LogHelper console = new SessionState.LogHelper(LOG);

  private Configuration conf;
  
  static {
    // set hive log dir
    String dir = System.getenv("IDGS_LOG_DIR");
    if (dir != null) {
      java.util.Properties sysp = new java.util.Properties(System.getProperties());
      sysp.setProperty("hive.log.dir", dir);
      sysp.setProperty("hive.querylog.location", dir);
      sysp.setProperty("hive.exec.scratchdir", dir);
      System.setProperties(sysp);
    }
    
    String timeout = System.getenv("CLIENT_TIMEOUT");
    if (timeout != null) {
      try {
        long to = Long.valueOf(timeout);
        ServerConst.timeout = to;
      } catch (Exception ex) {
      }
    }
  }
  
  public IdgsCliDriver() throws Exception {
    SessionState ss = SessionState.get();
    conf = (ss != null) ? ss.getConf() : new Configuration();
  }
  
  public static void main(String[] args) throws Exception {
    int ret = new IdgsCliDriver().run(args);
    System.exit(ret);
  }
  
  public int run(String[] args) throws Exception {
    OptionsProcessor optionsProcessor = new OptionsProcessor();
    if (!optionsProcessor.process_stage1(args)) {
      return 1;
    }

    boolean initLogError = false;
    String logInitDetailMessage;
    try {
      logInitDetailMessage = LOGUtils.initLog4j();
    } catch (IdgsException e) {
      initLogError = true;
      logInitDetailMessage = e.getMessage();
    }

    CliSessionState sessionState = new CliSessionState(new HiveConf(SessionState.class));
    sessionState.in = System.in;
    try {
      sessionState.out = new PrintStream(System.out, true, "UTF-8");
      sessionState.info = new PrintStream(System.err, true, "UTF-8");
      sessionState.err = new PrintStream(System.err, true, "UTF-8");
    } catch (UnsupportedEncodingException e) {
      return 3;
    }

    if (!optionsProcessor.process_stage2(sessionState)) {
      return 2;
    }

    if (!sessionState.getIsSilent()) {
      if (initLogError) {
        System.err.println(logInitDetailMessage);
      } else {
        SessionState.getConsole().printInfo(logInitDetailMessage);
      }
    }

    // set all properties specified via command line
    HiveConf hiveConf = sessionState.getConf();
    for (Map.Entry<Object, Object> item : sessionState.cmdProperties.entrySet()) {
      hiveConf.set((String) item.getKey(), (String) item.getValue());
    }
    
    prompt = IdgsConfVars.getVar(hiveConf, IdgsConfVars.CLIPROMPT);
    prompt = new VariableSubstitution().substitute(hiveConf, prompt);
    prompt2 = String.format("%1$-" + prompt.length() +"s", "");

    SessionState.start(sessionState);

    // ###### diff from hive use IdgsCliDriver.
    IdgsCliDriver cliDriver = new IdgsCliDriver();
    
    // ###### init client connection
    cliDriver.initConnection(hiveConf);

    // ###### load data store config and metadata
    cliDriver.initStoreMetadata(hiveConf);
    
    cliDriver.setHiveVariables(optionsProcessor.getHiveVariables());

    // Execute -i init files (always in silent mode)
    cliDriver.processInitFiles(sessionState);

    if (sessionState.execString != null) {
      LOG.info ("Command: " + sessionState.execString);
      return cliDriver.processLine(sessionState.execString);
    }

    try {
      if (sessionState.fileName != null) {
        return cliDriver.processFile(sessionState.fileName);
      }
    } catch (FileNotFoundException e) {
      System.err.println("Could not open input file for reading. (" + e.getMessage() + ")");
      return 3;
    }

    ConsoleReader consoleReader = new ConsoleReader();
    consoleReader.setBellEnabled(false);
    consoleReader.addCompletor(CliDriver.getCommandCompletor()[0]);

    String commandLine;
    File historyFile = new File(System.getProperty("user.home") + File.separator + ".hivehistory");
    consoleReader.setHistory(new History(historyFile));

    Method getFormattedDbMethod = CliDriver.class.getDeclaredMethod("getFormattedDb", HiveConf.class, CliSessionState.class);
    getFormattedDbMethod.setAccessible(true);

    Method spacesForStringMethod = CliDriver.class.getDeclaredMethod("spacesForString", String.class);
    spacesForStringMethod.setAccessible(true);

    String commandPrefix = "";
    String currentDB = (String) getFormattedDbMethod.invoke(null, hiveConf, sessionState);
    String currentPrompt = prompt + currentDB;
    String spacesForDB = (String) spacesForStringMethod.invoke(null, currentDB);

    int ret = 0;
    while ((commandLine = consoleReader.readLine(currentPrompt + "> ")) != null) {
      if (!commandPrefix.equals("")) {
        commandPrefix += '\n';
      }
      if (commandLine.trim().endsWith(";") && !commandLine.trim().endsWith("\\;")) {
        commandLine = commandPrefix + commandLine;
        LOG.info ("Command: " + commandLine);
        ret = cliDriver.processLine(commandLine, true);
        commandPrefix = "";
        currentDB = (String) getFormattedDbMethod.invoke(null, hiveConf, sessionState);
        currentPrompt = prompt + currentDB;
        spacesForDB = spacesForDB.length() == currentDB.length() ? spacesForDB : (String) spacesForStringMethod.invoke(null, currentDB);
      } else {
        commandPrefix = commandPrefix + commandLine;
        currentPrompt = prompt2 + spacesForDB;
        continue;
      }
    }

    sessionState.close();

    return ret;
  }
  
  public static ResultSet run(String cmd) throws Exception {
    boolean initLogError = false;
    String logInitDetailMessage = null;
    try {
      logInitDetailMessage = LOGUtils.initLog4j();
    } catch (IdgsException e) {
      initLogError = true;
      logInitDetailMessage = e.getMessage();
    }
    
    CliSessionState sessionState = new CliSessionState(new HiveConf(SessionState.class));
    
    if (!sessionState.getIsSilent()) {
      if (initLogError) {
        System.err.println(logInitDetailMessage);
      } else {
        SessionState.getConsole().printInfo(logInitDetailMessage);
      }
    }
    
    SessionState.start(sessionState);
    
    IdgsCliDriver cliDriver = new IdgsCliDriver();
    
    HiveConf hiveConf = (HiveConf) sessionState.getConf();
    
    // ###### init client connection
    cliDriver.initConnection(hiveConf);

    // ###### load data store config and metadata
    cliDriver.initStoreMetadata(hiveConf);
    
    cliDriver.processInitFiles(sessionState);
    
    return cliDriver.processSqlCmd(cmd);
  }

  /**
   * override super method process to handle idgs command
   */
  @Override
  public int processCmd(String cmd) {
    CliSessionState ss = (CliSessionState) SessionState.get();
    String cmd_trimmed = cmd.trim();
    String[] tokens = cmd_trimmed.split("\\s+");
    String cmd_1 = cmd_trimmed.substring(tokens[0].length()).trim();
    int ret = 0;

    if (cmd_trimmed.toLowerCase().equals("quit")
        || cmd_trimmed.toLowerCase().equals("exit")
        || tokens[0].equalsIgnoreCase("source") || cmd_trimmed.startsWith("!")
        || tokens[0].toLowerCase().equals("list")) {
      super.processCmd(cmd);
    } else {
      HiveConf hconf = (HiveConf) conf;

      try {
        CommandProcessor proc = CommandProcessorFactory.get(tokens, hconf);
        
        if (proc != null) {

          // Spark expects the ClassLoader to be an URLClassLoader.
          // In case we're using something else here, wrap it into an
          // URLCLassLaoder.
          if (System.getenv("TEST_WITH_ANT") == "1") {
            ClassLoader cl = Thread.currentThread().getContextClassLoader();
            Thread.currentThread().setContextClassLoader(new URLClassLoader(new URL[1], cl));
          }

          if (proc instanceof Driver) {
            // There is a small overhead here to create a new instance of
            // SharkDriver for every command. But it saves us the hassle of
            // hacking CommandProcessorFactory.
            Driver qp = null;
            try {
              // ##### using hive_idgs driver
              qp = (IdgsConfVars.getVar(conf, IdgsConfVars.EXEC_MODE) == "idgs") ? new IdgsDriver(hconf) : Driver.class.newInstance();
            } catch (Exception e) {
              e.printStackTrace();
            }

            LOG.info("Execution Mode: " + IdgsConfVars.getVar(conf, IdgsConfVars.EXEC_MODE));

            qp.init();
            PrintStream out = ss.out;
            long start = System.currentTimeMillis();
            if (ss.getIsVerbose()) {
              out.println(cmd);
            }

            ret = qp.run(cmd).getResponseCode();
            if (ret != 0) {
              qp.close();
              return ret;
            }
            
            boolean isPrint = IdgsConfVars.getBoolVar(conf, IdgsConfVars.PRINT_RESULT);
            List<Object[]> res = new ArrayList<Object[]>();
            
            if (isPrint) {
              if (HiveConf.getBoolVar(conf, HiveConf.ConfVars.HIVE_CLI_PRINT_HEADER)) {
                // Print the column names.
                List<FieldSchema> fieldSchemas = qp.getSchema().getFieldSchemas();
                if (fieldSchemas != null) {
                  for (FieldSchema fieldSchema : fieldSchemas) {
                    out.print("header" + fieldSchema.getName() + "\t");
                  }
                  out.println();
                }
              }
            }
              
            long printTime = 0;
            int counter = 0;
            
            SimpleDateFormat timestampFormat = new SimpleDateFormat("yyyy-MM-dd hh:mm:ss");
            SimpleDateFormat dateFormat = new SimpleDateFormat("yyyy-MM-dd");

            try {
              long s = System.currentTimeMillis();
              while (qp.getResults(res)) {
                for (Object[] row : res) {
                  if (isPrint) {
                    for (Object v : row) {
                      if (v != null) {
                        if (v instanceof byte[]) {
                          out.print(new String((byte[]) v));
                        } else if (v instanceof Timestamp) {
                          out.print(timestampFormat.format((Timestamp) v));
                        } else if (v instanceof Date) {
                          out.print(dateFormat.format((Date) v));
                        } else {
                          out.print(v);
                        }
                      } else {
                        out.print(v);
                      }
                      out.print("\t");
                    }
                    out.println();
                  }
                }
                
                counter += res.size();
                res.clear();
                if (out.checkError()) {
                  break;
                }
              }
              printTime = System.currentTimeMillis() - s;
            } catch (IOException e) {
              console.printError("Failed with exception " + e.getClass().getName() + ":" + e.getMessage(), "\n"
                  + org.apache.hadoop.util.StringUtils.stringifyException(e));
              ret = 1;
            }

            int cret = qp.close();
            if (ret == 0) {
              ret = cret;
            }

            long end = System.currentTimeMillis();
            double timeTaken = (end - start) / 1000.0;
            console.printInfo("Time taken: " + timeTaken + " seconds, Fetched: " + counter + " row(s)");
            
            // Destroy the driver to release all the locks.
            if (qp instanceof IdgsDriver) {
              LOG.info("Time taken: " + timeTaken + " seconds, Fetched: " + counter + " row(s)");
              LOG.info("Compile time taken: " + (((IdgsDriver) qp).getCompileTime() / 1000.0) + " seconds");
              LOG.info("Task run time taken: " + (((IdgsDriver) qp).getTaskRunTime() / 1000.0) + " seconds");
              LOG.info("Print time taken: " + (printTime / 1000.0) + " seconds");
              
              qp.destroy();
            }
          } else {
            if (ss.getIsVerbose()) {
              ss.out.println(tokens[0] + " " + cmd_1);
            }

            ret = proc.run(cmd_1).getResponseCode();
          }
        }
      } catch (CommandNeedRetryException ex) {
        LOG.error("Execute command " + cmd + " error.", ex);
        console.printInfo("Retry query with a different approach...");
      } catch (Exception ex) {
        LOG.error("Execute command " + cmd + "  error.", ex);
        console.printInfo("Execute command error, caused " + ex.getMessage() + ".");
        ret = 1;
      }
    }

    return ret;
  }
  
  public ResultSet processSqlCmd(String cmd) throws CommandNeedRetryException, SQLException {
    String cmd_trimmed = cmd.trim();
    String[] tokens = cmd_trimmed.split("\\s+");
    int ret = 0;

    ResultSet resultSet = null;
    if (cmd_trimmed.toLowerCase().equals("quit")
        || cmd_trimmed.toLowerCase().equals("exit")
        || tokens[0].equalsIgnoreCase("source") || cmd_trimmed.startsWith("!")
        || tokens[0].toLowerCase().equals("list")) {
      throw new CommandNeedRetryException(cmd + " is not a vaild sql");
    } else {
      long start = System.currentTimeMillis();
      HiveConf hconf = (HiveConf) conf;
      CommandProcessor proc = CommandProcessorFactory.get(tokens, hconf);
      if (proc == null || !(proc instanceof Driver)) {
        throw new CommandNeedRetryException(cmd + " is not a vaild sql");
      }

      IdgsDriver qp = new IdgsDriver(hconf);
      qp.init();
      ret = qp.run(cmd).getResponseCode();
      if (ret != 0) {
        qp.close();
        throw new CommandNeedRetryException(cmd + " execute error.");
      }
      
      int cret = qp.close();
      if (ret == 0) {
        ret = cret;
      }

      resultSet = qp.getResultSet();

      long end = System.currentTimeMillis();
      double timeTaken = (end - start) / 1000.0;
      int counter = (resultSet == null) ? 0 : resultSet.getRowCount();
      
      console.printInfo("Time taken: " + timeTaken + " seconds, Fetched: " + counter + " row(s)");
      LOG.info(cmd);
      LOG.info("Time taken: " + timeTaken + " seconds" + (counter == 0 ? "" : ", Fetched: " + counter + " row(s)"));
      LOG.info("Compile time taken: " + (qp.getCompileTime() / 1000.0) + " seconds");
      LOG.info("Task run time taken: " + (qp.getTaskRunTime() / 1000.0) + " seconds");
      
      qp.destroy();
    }

    return resultSet;
  }
  
  private void initConnection(HiveConf conf) throws IdgsException {
    LOG.info("initialize client connection.");

    String clientCfgFile = IdgsConfVars.getVar(conf, IdgsConfVars.CLIENT_CONFIG_FILE);
    
    try {
      TcpClientPool.getInstance().loadClientConfig(clientCfgFile);
      if (TcpClientPool.getInstance().size() == 0) {
        throw new IdgsException("no available server found");
      }
    } catch (IOException e) {
      throw new IdgsException("init connection error.", e);
    }
  }
  
  private void initStoreMetadata(HiveConf conf) throws IdgsException {
    LOG.info("start loader store metastore");
    
    String storeLoaderClass = IdgsConfVars.getVar(conf, IdgsConfVars.STORE_LOADER_CLASS);
    
    try {
      Class<?> _class = Class.forName(storeLoaderClass);
      Object inst = _class.newInstance();
      if (inst instanceof StoreLoader) {
        StoreLoader loader = (StoreLoader) inst;
        loader.init(conf);
        StoreMetadata.getInstance().initMetadata(conf, loader.loadDataStoreConf());
      } else {
        throw new SQLException("class " + storeLoaderClass + " is not StoreLoader");
      }
    } catch (Exception e) {
      throw new IdgsException(e);
    }
  }

  @Override
  public int processFile(String fileName) throws IOException {
    return super.processFile(fileName);
  }
  
}
