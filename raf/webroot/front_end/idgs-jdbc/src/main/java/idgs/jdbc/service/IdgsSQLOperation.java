package idgs.jdbc.service;

import idgs.IdgsDriver;
import idgs.jdbc.ReflectUtils;

import java.io.Serializable;
import java.util.Map;

import org.apache.hadoop.hive.conf.HiveConf;
import org.apache.hadoop.hive.metastore.api.Schema;
import org.apache.hadoop.hive.ql.Driver;
import org.apache.hadoop.hive.ql.exec.ExplainTask;
import org.apache.hadoop.hive.ql.exec.Task;
import org.apache.hadoop.hive.ql.parse.VariableSubstitution;
import org.apache.hadoop.hive.ql.processors.CommandProcessorResponse;
import org.apache.hive.service.cli.HiveSQLException;
import org.apache.hive.service.cli.OperationState;
import org.apache.hive.service.cli.TableSchema;
import org.apache.hive.service.cli.operation.SQLOperation;
import org.apache.hive.service.cli.session.HiveSession;

public class IdgsSQLOperation extends SQLOperation {

  public IdgsSQLOperation(HiveSession parentSession, String statement,
      Map<String, String> confOverlay, boolean runInBackground) {
    super(parentSession, statement, confOverlay, runInBackground);
  }

  @Override
  public void prepare(HiveConf hiveConf) throws HiveSQLException {
    setState(OperationState.RUNNING);

    try {
      Driver driver = new IdgsDriver(hiveConf);
      driver.setTryCount(Integer.MAX_VALUE);
      
      String subStatement = new VariableSubstitution().substitute(hiveConf, statement);
      CommandProcessorResponse response = driver.compileAndRespond(subStatement);
      if (0 != response.getResponseCode()) {
        throw toSQLException("Error while compiling statement", response);
      }

      Schema mResultSchema = driver.getSchema();

      TableSchema resultSchema = null;
      
      if(driver.isFetchingTable()) {
        //Schema has to be set
        if (mResultSchema == null || !mResultSchema.isSetFieldSchemas()) {
          throw new HiveSQLException("Error compiling query: Schema and FieldSchema " +
              "should be set when query plan has a FetchTask");
        }
        
        resultSchema = new TableSchema(mResultSchema);
        setHasResultSet(true);
      } else {
        setHasResultSet(false);
      }
      
      for (Task<? extends Serializable> task: driver.getPlan().getRootTasks()) {
        if (task.getClass() == ExplainTask.class) {
          resultSchema = new TableSchema(mResultSchema);
          setHasResultSet(true);
          break;
        }
      }
      
      ReflectUtils.setFieldValue(SQLOperation.class, "driver", this, driver);
      ReflectUtils.setFieldValue(SQLOperation.class, "response", this, response);
      ReflectUtils.setFieldValue(SQLOperation.class, "mResultSchema", this, mResultSchema);
      ReflectUtils.setFieldValue(SQLOperation.class, "resultSchema", this, resultSchema);
    } catch (HiveSQLException e) {
      setState(OperationState.ERROR);
      throw e;
    } catch (Exception e) {
      setState(OperationState.ERROR);
      throw new HiveSQLException("Error running query: " + e.toString(), e);
    }
  }
  
}
