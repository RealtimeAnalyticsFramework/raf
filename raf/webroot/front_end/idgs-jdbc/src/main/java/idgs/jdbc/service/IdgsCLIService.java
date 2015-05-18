package idgs.jdbc.service;

import idgs.jdbc.ReflectUtils;

import java.io.IOException;
import java.util.Collection;

import javax.security.auth.login.LoginException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.hadoop.hive.conf.HiveConf;
import org.apache.hadoop.hive.conf.HiveConf.ConfVars;
import org.apache.hadoop.hive.ql.exec.FunctionRegistry;
import org.apache.hadoop.hive.ql.metadata.HiveException;
import org.apache.hadoop.hive.ql.session.SessionState;
import org.apache.hadoop.hive.shims.ShimLoader;
import org.apache.hadoop.security.UserGroupInformation;
import org.apache.hive.service.AbstractService;
import org.apache.hive.service.Service;
import org.apache.hive.service.ServiceException;
import org.apache.hive.service.auth.HiveAuthFactory;
import org.apache.hive.service.cli.CLIService;
import org.apache.hive.service.cli.session.SessionManager;
import org.apache.hive.service.server.HiveServer2;

public class IdgsCLIService extends CLIService {
  
  private final Log LOG = LogFactory.getLog(IdgsCLIService.class.getName());
  
  private HiveServer2 server2;

  public IdgsCLIService(HiveServer2 hiveServer2) {
    super(hiveServer2);
    this.server2 = hiveServer2;
  }
  
  @Override
  public synchronized void init(HiveConf hiveConf) {
    try {
      SessionState ss = new SessionState(hiveConf);
      ss.setIsHiveServerQuery(true);
      SessionState.start(ss);
      ss.applyAuthorizationPolicy();
    } catch (HiveException e) {
      throw new RuntimeException("Error applying authorization policy on hive configuration", e);
    }
    
    try {
      ReflectUtils.setFieldValue(CLIService.class, "hiveConf", this, hiveConf);
    } catch (Exception e) {
      e.printStackTrace();
    }
    
    SessionManager sessionManager = new IdgsSessionManager(server2);
    addService(sessionManager);
    
    try {
      ReflectUtils.setFieldValue(CLIService.class, "sessionManager", this, sessionManager);
    } catch (Exception e) {
      e.printStackTrace();
    }
    
    if (ShimLoader.getHadoopShims().isSecurityEnabled()) {
      try {
        HiveAuthFactory.loginFromKeytab(hiveConf);
        UserGroupInformation serviceUGI = ShimLoader.getHadoopShims().getUGIForConf(hiveConf);
        ReflectUtils.setFieldValue(CLIService.class, "serviceUGI", this, serviceUGI); 
      } catch (IOException e) {
        throw new ServiceException("Unable to login to kerberos with given principal/keytab", e);
      } catch (LoginException e) {
        throw new ServiceException("Unable to login to kerberos with given principal/keytab", e);
      } catch (Exception e) {
        throw new ServiceException("Unable to login to kerberos with given principal/keytab", e);
      }

      String principal = hiveConf.getVar(ConfVars.HIVE_SERVER2_SPNEGO_PRINCIPAL);
      String keyTabFile = hiveConf.getVar(ConfVars.HIVE_SERVER2_SPNEGO_KEYTAB);
      if (principal.isEmpty() || keyTabFile.isEmpty()) {
        LOG.info("SPNego httpUGI not created, spNegoPrincipal: " + principal +
            ", ketabFile: " + keyTabFile);
      } else {
        try {
          UserGroupInformation httpUGI = HiveAuthFactory.loginFromSpnegoKeytabAndReturnUGI(hiveConf);
          ReflectUtils.setFieldValue(CLIService.class, "httpUGI", this, httpUGI);
          LOG.info("SPNego httpUGI successfully created.");
        } catch (IOException e) {
          LOG.warn("SPNego httpUGI creation failed: ", e);
        } catch (Exception e) {
          LOG.error("SPNego httpUGI creation failed: ", e);
        }
      }
    }
    
    FunctionRegistry.setupPermissionsForBuiltinUDFs(
        hiveConf.getVar(ConfVars.HIVE_SERVER2_BUILTIN_UDF_WHITELIST),
        hiveConf.getVar(ConfVars.HIVE_SERVER2_BUILTIN_UDF_BLACKLIST));
    
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
  }
  
}
