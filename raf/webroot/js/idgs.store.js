;
(function ($) {
    $.idgs.store = {
    };
    
    /// @fn insert(request, callback)
    /// @param object request InsertRequest
    /// @param function callback InsertResponse
    $.idgs.store.insert = function (request, callback) {
      var rpcMsg = {operation_name: "insert"};
      rpcMsg = $.extend(true, {}, $.idgs.store.rpc_msg_defaults, rpcMsg);
      rpcMsg.attachments = {};
      rpcMsg.attachments.key = request.key;
      rpcMsg.attachments.value = request.value;
      delete request.key;
      delete request.value;
      rpcMsg.payload = request;
      
      $.idgs.sendrecv(rpcMsg, function (response) {
        var payload = response.payload;
        if (response.attachments && response.attachments.last_value) {
          payload.last_value = response.attachments.last_value;
        } 
        callback(payload);
      });      
    };

    /// @fn update(request, callback)
    /// @param object request UpdateRequest
    /// @param function callback UpdateResponse    
    $.idgs.store.update = function (request, callback) {
      var rpcMsg = {operation_name: "update"};
      rpcMsg = $.extend(true, {}, $.idgs.store.rpc_msg_defaults, rpcMsg);
      rpcMsg.attachments = {};
      rpcMsg.attachments.key = request.key;
      rpcMsg.attachments.value = request.value;
      delete request.key;
      delete request.value;
      rpcMsg.payload = request;
      
      $.idgs.sendrecv(rpcMsg, function (response) {
        var payload = response.payload;
        if (response.attachments && response.attachments.last_value) {
          payload.last_value = response.attachments.last_value;
        } 
        callback(payload);
      });      
    };
    
    /// @fn get(request, callback)
    /// @param object request GerRequest
    /// @param function callback GetResponse
    $.idgs.store.get = function (request, callback) {
      var rpcMsg = {operation_name: "get"};
      rpcMsg = $.extend(true, {}, $.idgs.store.rpc_msg_defaults, rpcMsg);
      rpcMsg.attachments = {};
      rpcMsg.attachments.key = request.key;
      delete request.key;
      rpcMsg.payload = request;
      
      $.idgs.sendrecv(rpcMsg, function (response) {
        var payload = response.payload;
        if (response.attachments) {
          if (response.attachments.value) {
            payload.value = response.attachments.value;
          }
          
          if (response.attachments.last_value) {
            payload.last_value = response.attachments.last_value;
          }
        } 
        callback(payload);
      });
    };

    /// @fn delete(request, callback)
    /// @param object request DeleteRequest
    /// @param function callback DeleteResponse
    $.idgs.store.delete = function (request, callback) {
      var rpcMsg = {operation_name: "delete"};
      rpcMsg = $.extend(true, {}, $.idgs.store.rpc_msg_defaults, rpcMsg);
      rpcMsg.attachments = {};
      rpcMsg.attachments.key = request.key;
      delete request.key;
      rpcMsg.payload = request;
      
      $.idgs.sendrecv(rpcMsg, function (response) {
        var payload = response.payload;
        if (response.attachments && response.attachments.last_value) {
          payload.last_value = response.attachments.last_value;
        } 
        callback(payload);
      });
    };
    
        /// @fn truncate(request, callback)
    /// @param object request TruncateRequest
    /// @param function callback TruncateResponse
    $.idgs.store.truncate = function (request, callback) {
      var rpcMsg = {operation_name: "truncate"};
      rpcMsg = $.extend(true, {}, $.idgs.store.rpc_msg_defaults, rpcMsg);
      rpcMsg.attachments = {};
      rpcMsg.payload = request;
      
      $.idgs.sendrecv(rpcMsg, function (response) {
        var payload = response.payload;
        callback(payload);
      });
    };

    ///
    /// default value in RpcMessage
    ///
    $.idgs.store.rpc_msg_defaults = {
       dest_actor: {
          member_id: "ANY_MEMBER",   // -2
          actor_id: "store.service"
       },
       source_actor: {
          member_id: "CLIENT_MEMBER",   // -4
          actor_id: "ajax"
       },

       serdes_type: "PB_JSON" // 1      
    };

})(jQuery);
