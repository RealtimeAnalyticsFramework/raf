package idgs.jdbc.service;

import idgs.jdbc.ReflectUtils;

import java.io.File;
import java.util.Collection;
import java.util.Map;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.hive.conf.HiveConf;
import org.apache.hadoop.hive.conf.HiveConf.ConfVars;
import org.apache.hive.service.AbstractService;
import org.apache.hive.service.Service;
import org.apache.hive.service.cli.HiveSQLException;
import org.apache.hive.service.cli.SessionHandle;
import org.apache.hive.service.cli.operation.OperationManager;
import org.apache.hive.service.cli.session.HiveSession;
import org.apache.hive.service.cli.session.HiveSessionImpl;
import org.apache.hive.service.cli.session.SessionManager;
import org.apache.hive.service.cli.thrift.TProtocolVersion;
import org.apache.hive.service.server.HiveServer2;

public class IdgsSessionManager extends SessionManager {
  
  private final Log LOG = LogFactory.getLog(IdgsSessionManager.class.getName());
  
  private OperationManager opManager = new IdgsOperationManager();
  
  public IdgsSessionManager(HiveServer2 hiveServer2) {
    super(hiveServer2);
  }
  
  @Override
  public synchronized void init(HiveConf hiveConf) {
    try {
      ReflectUtils.setFieldValue(SessionManager.class, "hiveConf", this, hiveConf);
      if (hiveConf.getBoolVar(ConfVars.HIVE_SERVER2_LOGGING_OPERATION_ENABLED)) {
        ReflectUtils.methodInvoke(SessionManager.class, "initOperationLogRootDir", null, this, null);
      }
      ReflectUtils.methodInvoke(SessionManager.class, "createBackgroundOperationPool", null, this, null);
    } catch (Exception ex) {
      ex.printStackTrace();
    }
    
    addService(opManager);

    try {
      try {
        Collection<Service> services = getServices();
        for (Service service : services) {
          service.init(hiveConf);
        }
        
        ReflectUtils.methodInvoke(AbstractService.class, "ensureCurrentState", new Class<?>[] {STATE.class}, this, new Object[] {STATE.NOTINITED});
        ReflectUtils.setFieldValue(AbstractService.class, "hiveConf", this, hiveConf);
        ReflectUtils.methodInvoke(AbstractService.class, "changeState", new Class<?>[] {STATE.class}, this, new Object[] {STATE.INITED});
        LOG.info("Service:" + getName() + " is inited.");
      } catch (Exception e) {
        e.printStackTrace();
      }
    } catch (Exception e) {
      e.printStackTrace();
    }
  }
  
  @Override
  public SessionHandle openSession(TProtocolVersion protocol, String username, String password, String ipAddress,
      Map<String, String> sessionConf, boolean withImpersonation, String delegationToken)
          throws HiveSQLException {
    HiveSession session = null;
    try {
      HiveConf hiveConf = (HiveConf) ReflectUtils.getFieldValue(SessionManager.class, "hiveConf", this);
      session = new HiveSessionImpl(protocol, username, password, hiveConf, ipAddress);
  
      session.setSessionManager(this);
      session.setOperationManager(opManager);
    } catch (Exception e) {
      throw new HiveSQLException("Failed to open new session", e);
    }
    
    try {
      session.initialize(sessionConf);
      
      boolean isOperationLogEnabled = (Boolean) ReflectUtils.getFieldValue(SessionManager.class, "isOperationLogEnabled", this);
      if (isOperationLogEnabled) {
        File operationLogRootDir = (File) ReflectUtils.getFieldValue(SessionManager.class, "operationLogRootDir", this);
        session.setOperationLogSessionDir(operationLogRootDir);
      }
      session.open();
    } catch (Exception e) {
      throw new HiveSQLException("Failed to open new session", e);
    }
    
    try {
      ReflectUtils.methodInvoke(SessionManager.class, "executeSessionHooks", new Class<?>[] {HiveSession.class}, this, new Object[] {session});
    } catch (Exception e) {
      throw new HiveSQLException("Failed to execute session hooks", e);
    }

    try {
      @SuppressWarnings("unchecked")
      Map<SessionHandle, HiveSession> handleToSession = (Map<SessionHandle, HiveSession>) ReflectUtils.getFieldValue(SessionManager.class, "handleToSession", this);
      handleToSession.put(session.getSessionHandle(), session);
    } catch (Exception e) {
      throw new HiveSQLException("Failed to store session handle", e);
    }

    return session.getSessionHandle();
  }
  
  @Override
  public OperationManager getOperationManager() {
    return opManager;
  }

}
