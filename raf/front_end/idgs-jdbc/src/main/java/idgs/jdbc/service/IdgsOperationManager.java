package idgs.jdbc.service;

import idgs.jdbc.ReflectUtils;

import java.sql.SQLException;
import java.util.Map;

import org.apache.hadoop.hive.ql.processors.CommandProcessor;
import org.apache.hadoop.hive.ql.processors.CommandProcessorFactory;
import org.apache.hive.service.cli.HiveSQLException;
import org.apache.hive.service.cli.operation.ExecuteStatementOperation;
import org.apache.hive.service.cli.operation.HiveCommandOperation;
import org.apache.hive.service.cli.operation.Operation;
import org.apache.hive.service.cli.operation.OperationManager;
import org.apache.hive.service.cli.session.HiveSession;

public class IdgsOperationManager extends OperationManager {

  @Override
  public ExecuteStatementOperation newExecuteStatementOperation(HiveSession parentSession,
      String statement, Map<String, String> confOverlay, boolean runAsync)
          throws HiveSQLException {
    ExecuteStatementOperation statementOp = null;

    String[] tokens = statement.trim().split("\\s+");
    CommandProcessor processor = null;
    try {
      processor = CommandProcessorFactory.getForHiveCommand(tokens, parentSession.getHiveConf());
    } catch (SQLException e) {
      throw new HiveSQLException(e.getMessage(), e.getSQLState(), e);
    }
    
    try {
      if (processor == null) {
        statementOp = new IdgsSQLOperation(parentSession, statement, confOverlay, false);
      } else {
        statementOp = (ExecuteStatementOperation) ReflectUtils.newInstance(HiveCommandOperation.class, 
            new Class<?>[] {HiveSession.class, String.class, Map.class, Boolean.class}, 
            new Object[] {parentSession, statement, processor, confOverlay});
      }
    
      ReflectUtils.methodInvoke(OperationManager.class, "addOperation", new Class<?>[] {Operation.class}, this, new Object[] {statementOp});
    } catch (Exception e) {
      e.printStackTrace();
    }
    
    return statementOp;
  }
}
