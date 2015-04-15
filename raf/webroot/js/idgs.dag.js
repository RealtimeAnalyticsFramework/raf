;
(function ($) {
    $.idgs.dag = {
    };
    
    /// @fn listRdd(callback)
    /// @param function callback
    $.idgs.dag.listRdd = function (callback) {
      var rpcMsg = {operation_name: "list_rdd"};
      rpcMsg = $.extend(true, {}, $.idgs.dag.rpc_msg_defaults, rpcMsg);
      
      $.idgs.sendrecv(rpcMsg, function (response) {
        var payload = response.payload;
        callback(payload);
      });      
    };
    
    /// @fn showDag(model)
    /// @param object model
    $.idgs.dag.showDag = function (model) {
      // Create a new directed graph
      var g = new dagreD3.graphlib.Graph().setGraph({});
      
      // add store delegate nodes
      if (model.store_delegate) { 
        model.store_delegate.forEach(function(delegate) {
          g.setNode(delegate.store_name, {label: delegate.store_name });
          g.setNode(delegate.rdd_name, {label: delegate.rdd_name });
          
          g.setEdge(delegate.store_name,     delegate.rdd_name,     { label: "Delegate" });
          g.node(delegate.store_name).style = "fill: #77f";
          g.node(delegate.rdd_name).style = "fill: #7f7";
        });
      }
      
      // add normal rdd nodes
      if ( model.rdd) {
        model.rdd.forEach(function(r) {
          g.setNode(r.out_rdd.rdd_name, {label: r.out_rdd.rdd_name });
          r.in_rdd.forEach(function(in_rdd) {
            g.setEdge(in_rdd.rdd_name,     r.out_rdd.rdd_name,     { label: r.transformer_op_name });
          });
        });
      }
      
      // Set some general styles
      g.nodes().forEach(function(v) {
        var node = g.node(v);
        node.rx = node.ry = 5;
      });
      
      // init d3 svg
      var svg = d3.select("svg");
      var inner = svg.select("g");

      // Set up zoom support
      var zoom = d3.behavior.zoom().on("zoom", function() {
        inner.attr("transform", "translate(" + d3.event.translate + ")" +
                                  "scale(" + d3.event.scale + ")");
      });
      svg.call(zoom);

      // Create the renderer
      var render = new dagreD3.render();

      // Run the renderer. This is what draws the final graph.
      render(inner, g);

      // Center the graph
      /*
      var initialScale = 0.75;
      zoom
        .translate([(svg.attr("width") - g.graph().width * initialScale) / 2, 20])
        .scale(initialScale)
        .event(svg);
      svg.attr('height', g.graph().height * initialScale + 40);
      */
    };

    /// @fn createStoreDelegateRdd(request, callback)
    /// @param object request e.g. {schema_name : "tpch", store_name : "Customer", rdd_name : "sd_customer"}
    /// @param function callback
    $.idgs.dag.createStoreDelegate = function (request, callback) {
      var rpcMsg = {
        operation_name: "CREATE_STORE_DELEGATE_RDD",
        payload: request        
        };
      rpcMsg = $.extend(true, {}, $.idgs.dag.rpc_msg_defaults, rpcMsg);
      
      $.idgs.sendrecv(rpcMsg, function (response) {
        var payload = response.payload;
        callback(payload);
      });      
    };
    
    /// @fn filter(request, callback)
    /// @param object request
    /// @param function callback
    $.idgs.dag.filter = function (request, callback) {
      var rpcMsg = {
        operation_name: "CREATE_RDD",
        payload: request        
        };
      rpcMsg = $.extend(true, {}, $.idgs.dag.rpc_msg_defaults, rpcMsg);
      
      $.idgs.sendrecv(rpcMsg, function (response) {
        var payload = response.payload;
        callback(payload);
      });      
    };
    
    /// @fn count(request, callback)
    /// @param object request
    /// @param function callback
    $.idgs.dag.count = function (request, callback) {
      var rpcMsg = {
        operation_name: "RDD_ACTION_REQUEST",
        payload: request        
        };
      rpcMsg = $.extend(true, {}, $.idgs.dag.rpc_msg_defaults, rpcMsg);
      
      $.idgs.sendrecv(rpcMsg, function (response) {
        var payload = response.payload;
        payload.attachments = response.attachments;
        delete payload.attachments.KEY_METADATA;
        delete payload.attachments.VALUE_METADATA;
        callback(payload);
      });      
    };
    
    
    
    ///
    /// default value in RpcMessage
    ///
    $.idgs.dag.rpc_msg_defaults = {
       dest_actor: {
          member_id: "ANY_MEMBER",   // -2
          actor_id: "rdd.service"
       },
       source_actor: {
          member_id: "CLIENT_MEMBER",   // -4
          actor_id: "ajax"
       },

       serdes_type: "PB_JSON" // 1      
    };

})(jQuery);
