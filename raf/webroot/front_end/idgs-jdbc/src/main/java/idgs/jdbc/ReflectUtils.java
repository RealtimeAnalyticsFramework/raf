package idgs.jdbc;

import java.lang.reflect.Constructor;
import java.lang.reflect.Field;
import java.lang.reflect.Method;
import java.util.Map;

import org.apache.hive.service.cli.operation.HiveCommandOperation;
import org.apache.hive.service.cli.session.HiveSession;

public class ReflectUtils {

  public static void setFieldValue(Class<?> _class, String name, Object instance, Object value) throws Exception {
    Field field = _class.getDeclaredField(name);
    field.setAccessible(true);
    field.set(instance, value);
    field.setAccessible(false);
  }
  
  public static Object getFieldValue(Class<?> _class, String name, Object instance) throws Exception {
    Field field = _class.getDeclaredField(name);
    field.setAccessible(true);
    Object value = field.get(instance);
    field.setAccessible(false);
    
    return value;
  }
  
  public static Object methodInvoke(Class<?> _class, String name, Class<?>[] types, Object instance, Object[] params) throws Exception {
    Method method = _class.getDeclaredMethod(name, types);
    method.setAccessible(true);
    Object ret = method.invoke(instance, params);
    method.setAccessible(false);
    return ret;
  }
  
  public static Object newInstance(Class<?> _class, Class<?>[] types, Object[] params) throws Exception {
    Constructor<HiveCommandOperation> constructor = HiveCommandOperation.class.getDeclaredConstructor(HiveSession.class, String.class, Map.class, Boolean.class);
    constructor.setAccessible(true);
    Object instance = constructor.newInstance(params);
    constructor.setAccessible(false);
    return instance;
  }
  
}
