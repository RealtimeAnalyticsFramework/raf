;
(function ($) {
    $.idgs.cluster = {
    };
    
    /// @fn getClusterConfig(request, callback)
    /// @param function callback InsertResponse
    $.idgs.cluster.getClusterConfig = function (callback) {
      var rpcMsg = {operation_name: "get_cluster_cfg"};
      rpcMsg = $.extend(true, {}, $.idgs.cluster.rpc_msg_defaults, rpcMsg);
      
      $.idgs.sendrecv(rpcMsg, function (response) {
        var payload = response.payload;
        callback(payload);
      });      
    };

    /// @fn listMembers(callback)
    /// @param function callback InsertResponse
    $.idgs.cluster.listMembers = function (callback) {
      var rpcMsg = {operation_name: "list_members"};
      rpcMsg = $.extend(true, {}, $.idgs.cluster.rpc_msg_defaults, rpcMsg);
      
      $.idgs.sendrecv(rpcMsg, function (response) {
        var payload = response.payload;
        callback(payload);
      });      
    };

    /// @fn listPartitions(callback)
    /// @param function callback InsertResponse
    $.idgs.cluster.listPartitions = function (callback) {
      var rpcMsg = {       
        dest_actor: {
          member_id: "ANY_MEMBER",   // -2
          actor_id: "cluster.partition"
        },
        operation_name: "list_partitions"
      };
      rpcMsg = $.extend(true, {}, $.idgs.cluster.rpc_msg_defaults, rpcMsg);
      
      $.idgs.sendrecv(rpcMsg, function (response) {
        var payload = response.payload;
        callback(payload);
      });      
    };

    ///
    /// default value in RpcMessage
    ///
    $.idgs.cluster.rpc_msg_defaults = {
       dest_actor: {
          member_id: "ANY_MEMBER",   // -2
          actor_id: "cluster.member"
       },
       source_actor: {
          member_id: "CLIENT_MEMBER",   // -4
          actor_id: "ajax"
       },

       serdes_type: "PB_JSON" // 1      
    };

})(jQuery);
