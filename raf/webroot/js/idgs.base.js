;
(function ($) {
    $.ajaxSetup ({
      contentType: "application/json",
      mimeType: "application/json"
    });

    $.idgs = {
    };
    window.idgs = $.idgs;
    
    /// @fn sendrecv(request, callback)
    /// @param object request RpcMessage
    /// @param function callback RpcMessage
    /// sample of RpcMessage
    /// {
    ///   dest_actor: {
    ///     member_id: "store.service",   // -4
    ///     member_id: "ANY_MEMBER"   // -2
    ///   },
    ///   source_actor: {
    ///     member_id: "CLIENT_MEMBER",   // -4
    ///     actor_id: "ajax"
    ///   },
    ///   operation_name: "insert",
    ///   serdes_type: "PB_JSON" // 1      
    ///   payload: {      
    ///     store_name: "order",
    ///     shcema: "ssb"
    ///   },
    ///   attachments: {      
    ///     key: {
    ///       o_id: 1
    ///     },
    ///     key: {
    ///       name: "tom"
    ///     }
    ///   }
    /// };
    ///
    ///
    $.idgs.sendrecv = function (request, callback) {
      request = $.extend(true, {}, $.idgs.rpc_msg_defaults, request);
      
      if (!request.dest_actor.actor_id) {
        console.log("destination actor is null");
        return;
      }
      if (!request.operation_name) {
        console.log("operation name is null");
        return;
      }
      
      // serialize payload
      if (request.payload && typeof request.payload == "object") {
        request.payload = JSON.stringify(request.payload)
      }
      
      if (request.attachments  && typeof request.attachments == "object") {
        var orig = request.attachments;
        var ser = [];
        for (var name in orig) {
          if (orig.hasOwnProperty(name)) {
            ser.push({name:name, value:JSON.stringify(orig[name])});
          }
        } 
        request.attachments = ser;
      } 
      
      var url = "/actor";
      var post_data = JSON.stringify(request);
      $.ajax(url, {
        type: "POST",
        data: post_data,
        dataType: "json",
        contentType: "application/json; charset=UTF-8", 
        success:function (data) {
          var response = data;
        
          // parse payload
          if (response.payload) {
            response.payload = JSON.parse(response.payload)
          }
          
          // parse attachments
          if (response.attachments) {
            var orig = response.attachments;
            var ser = {};
            for (var i = 0; i < orig.length; ++i) {
              ser[orig[i].name] = JSON.parse(orig[i].value);
            } 
            response.attachments = ser;
          } 
          
          // call user logic
          callback(response);   
        }
      });
           
    };
    
    ///
    /// default value in RpcMessage
    ///
    $.idgs.rpc_msg_defaults = {
       dest_actor: {
          member_id: "ANY_MEMBER"   // -2
       },
       source_actor: {
          member_id: "CLIENT_MEMBER",   // -4
          actor_id: "ajax"
       },

       serdes_type: "PB_JSON" // 1      
    };
})(jQuery);
