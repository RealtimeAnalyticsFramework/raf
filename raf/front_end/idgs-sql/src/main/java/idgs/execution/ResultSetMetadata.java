package idgs.execution;

import java.util.HashMap;
import java.util.Map;

import com.google.protobuf.Descriptors.Descriptor;
import com.google.protobuf.Descriptors.FieldDescriptor;
import com.google.protobuf.DynamicMessage;

public class ResultSetMetadata {

  private Descriptor keyDescriptor;
  
  private Descriptor valueDescriptor;
  
  private Map<String, FieldDescriptor> keyDescMap;
  
  private Map<String, FieldDescriptor> valueDescMap;
  
  public ResultSetMetadata(Descriptor keyDescriptor, Descriptor valueDescriptor) {
    this.keyDescriptor = keyDescriptor;
    this.valueDescriptor = valueDescriptor;
    
    keyDescMap = new HashMap<String, FieldDescriptor>();
    for (FieldDescriptor field : keyDescriptor.getFields()) {
      keyDescMap.put(field.getName(), field);
    }
    
    valueDescMap = new HashMap<String, FieldDescriptor>();
    for (FieldDescriptor field : valueDescriptor.getFields()) {
      valueDescMap.put(field.getName(), field);
    }
  }
  
  public FieldDescriptor findField(String fieldName) {
    FieldDescriptor field = null;
    if (isKeyField(fieldName)) {
      field = keyDescMap.get(fieldName);
    } else {
      field = valueDescMap.get(fieldName);
    }
    
    return field;
  }
  
  public boolean isKeyField(String fieldName) {
    return keyDescMap.containsKey(fieldName);
  }
  
  public Descriptor getKeyMetadata() {
    return keyDescriptor;
  }
  
  public Descriptor getValueMetadata() {
    return valueDescriptor;
  }
  
  public DynamicMessage.Builder newKeyBuilder() {
    if (keyDescriptor == null) {
      return null;
    } else {
      return DynamicMessage.newBuilder(keyDescriptor);
    }
  }
  
  public DynamicMessage.Builder newValueBuilder() {
    if (valueDescriptor == null) {
      return null;
    } else {
      return DynamicMessage.newBuilder(valueDescriptor);
    }
  }
  
}
