(function ($) {
$(document).ready($.jswt.load_templates(function () {

var dashboard_content = "#dashboard-content";

//
// Store
//
var showStore = function (tab) {
  $("#results_store_tab").empty();
  
  var $container = $("#" + tab.id);
  $container.empty().append($("#storeCRUDTemplate").render({}));
  $('#store_commands').autosize();

  $container.find("#button_exec").click(function (event) {
    var script = $('#store_commands').val();
    eval(script);
  });

  // insert button
	$container.find("#button_insert").click(function (event) {
	  $('#store_commands').val($('#store_insert_commands').val()).trigger('autosize.resize');
  });
            
    // update button
  $container.find("#button_update").click(function (event) {
    $('#store_commands').val($('#store_update_commands').val()).trigger('autosize.resize');
  });
            
  $container.find("#button_get").click(function (event) {
    $('#store_commands').val($('#store_get_commands').val()).trigger('autosize.resize');
  });
            
  $container.find("#button_delete").click(function (event) {
    $('#store_commands').val($('#store_delete_commands').val()).trigger('autosize.resize');
  });
            
  $container.find("#button_truncate").click(function (event) {
    $('#store_commands').val($('#store_truncate_commands').val()).trigger('autosize.resize');
  });
};

//
// SQL
//
var showSQL = function (tab) {
  var $container = $("#" + tab.id);
  $container.empty().append($("#sqlTemplate").render({}));
  $('#sql_commands').autosize();
  $('#sql_result').autosize();
  $('#sql_exec').click(function() {
    var sql = $('#sql_commands').val();

    var script = "$IDGS_HOME/bin/idgs-sql-cli.sh -e \"" + sql + "\"";
    var url = "/shell";

    url = "/sql";
    script = sql;

    $('#sql_result').val("").trigger('autosize.resize');
    $.ajax(url, {
      type: "POST",
      data: script,
      dataType: "text",
      contentType: "text/plain; charset=UTF-8", 
      success:function (data) {
        // alert(data);
        $('#sql_result').val(data).trigger('autosize.resize');
      }
    });    
  });
};

///
/// Cluster configuration
///
var showCluster = function (tab) {
  var $container = $("#" + tab.id);
  idgs.cluster.getClusterConfig(function(data) {
    // $container.html(JSON.stringify(data));
    $container.empty();
    
    $container.jswt_panel({
        style: "default",
        icon_class: "fa fa-stethoscope",
        title: "General Information"
    }).jswt_properties({
        data: data,
        properties: [
            {label: "Worker Thread Count", attr: "thread_count"},
            {label: "IO Thread Count", attr: "io_thread_count"},
            {label: "Max Idle Thread Count", attr: "max_idle_thread"},
            // {label: "repartition_batch", attr: "repartition_batch"},
            // {label: "mtu", attr: "mtu"},
            // {label: "tcp_batch", attr: "tcp_batch"},
            // {label: "reserved_member_size", attr: "reserved_member_size"},
            {label: "Partition Count", attr: "partition_count"},
            {label: "Group Name", attr: "group_name"}
        ]
    });
    
    for ( var i = 0; i < data.modules.length; ++i) {
      var mod = data.modules[i];
      if ( mod.config_path ) {
        mod.config_url = "<a href='" + mod.config_path + "'>" + mod.config_path + "</a>";
      } 
      $container.jswt_panel({
          style: "success",
          icon_class: "fa fa-compass",
          title: "Module: " + mod.name
      }).jswt_properties({
          data: mod,
          properties: [
              // {label: "Name", attr: "name"},
              {label: "Module", attr: "module_path"},
              {label: "Config", attr: "config_url"}
          ]
      });
    }
  });
};

///
/// Membership table
///
var showMembers = function (tab) {
  var $container = $("#" + tab.id);

  idgs.cluster.listMembers(function(data) {
    $container.html(JSON.stringify(data));
    var formator = function(data) {
      var newdata = { member:[]};
      for (var i = 0; i < data.member.length; ++i) {
        var m = data.member[i]
        var new_m = {
          id: m.id,
          pid: m.pid,
          public_address: m.public_address.host + ":" + m.public_address.port, 
          inner_address: m.inner_address.host + ":" + m.inner_address.port, 
          local_store: m.service.local_store,
          leading: m.flags & 1,
          state: m.state,
          weight: m.weight          
        };
        newdata.member.push(new_m);
      }
      // alert(JSON.stringify(data));
      return newdata;
    };

    $($container).empty().jswt_table({
      data: data,
      formator: formator,
      root: "member",
      headers: [
        {label: "ID", attr: "id"},
        {label: "Pid", attr: "pid"},
        {label: "Public Address", attr: "public_address"},
        {label: "Inner Address", attr: "inner_address"},
        {label: "Store?", attr: "local_store"},
        {label: "Leading?", attr: "leading"},
        {label: "State", attr: "state"},
        {label: "Weight", attr: "weight"}
      ]
    });    


  });
};

///
/// Partition table
///
var showPartitions = function (tab) {
  var $container = $("#" + tab.id);
  idgs.cluster.listPartitions(function(data) {
    // $container.html(JSON.stringify(data));
    var formator = function(data) {
      var newdata = { partition:[]};
      for (var i = 0; i < data.partition.length; ++i) {
        var cells = data.partition[i].cells;
        var new_cell = {partition: i};
        
        for (var j = 0; j < cells.length; ++j) {
          new_cell["mid" + j] = cells[j].member_id;
          new_cell["state" + j] = cells[j].state;
        }
        
        newdata.partition.push(new_cell);
      }
      // alert(JSON.stringify(data));
      return newdata;
    };
    
    $($container).empty().jswt_table({
      data: data,
      formator: formator,
      root: "partition",
      headers: [
        {label: "Partition", attr: "partition"},
        {label: "Member0", attr: "mid0"},
        {label: "State0", attr: "state0"},
        {label: "Member1", attr: "mid1"},
        {label: "State1", attr: "state1"},
        {label: "Member2", attr: "mid2"},
        {label: "State2", attr: "state2"}
      ]
    });    
  });
};

        
///
/// query result
///
var query_result_table = function (tab, context) {
    var $container = $("#" + tab.id);
    if (!context) {
        $container.html("");
    } else {
        
    }
};

///
/// todo
///
var todo = function () {
    $(dashboard_content).html("This page will be built later.");
};

///
/// click dashboard
///
var click_dashboard = function () {
    $(dashboard_content).html($("#dashboardTemplate").render({}));
};

///
/// cluster
///
var click_cluster = function () {
   $(dashboard_content).empty().jswt_tab({
        tabs: [
            {id: "cluster_tab", label: "Configuration", onclick: showCluster, icon_class: "fa fa-file"},
            {id: "member_tab", label: "Member", onclick: showMembers, icon_class: "fa fa-file"},
            {id: "partition_tab", label: "Partition", onclick: showPartitions, icon_class: "fa fa-file"}
        ]
    });
}

///
/// store
///
var click_store = function () {
   $(dashboard_content).empty().jswt_tab({
        tabs: [
            {id: "crud_tab", label: "Store CRUD", onclick: showStore, icon_class: "fa fa-file"}
        ]
    }).jswt_tab({
        id: "result_tabs",
        tabs: [
            {id: "results_store_tab", label: "Results", onclick: query_result_table, icon_class: "fa fa-table"}
        ]
    });
}

///
/// show transform
///
var showTransform = function(tab) {
  var $container = $("#" + tab.id);
  $container.empty().append($("#dagTransformTemplate").render({}));
  $('#dag_commands').autosize();
  $('#dag_result').autosize();

  $container.find("#dag_exec").click(function (event) {
    var script = $('#dag_commands').val();
    eval(script);
  });

  // store delegate button
  $container.find("#button_store_delegate").click(function (event) {
    $('#dag_commands').val($('#dag_store_delegate_command').val()).trigger('autosize.resize');
  });

  // Filter button
  $container.find("#button_filter").click(function (event) {
    $('#dag_commands').val($('#dag_filter_command').val()).trigger('autosize.resize');
  });
            
};

///
/// show action
///
var showAction = function(tab) {
  var $container = $("#" + tab.id);
  $container.empty().append($("#dagActionTemplate").render({}));
  $('#action_commands').autosize();
  $('#action_result').autosize();

  $container.find("#action_exec").click(function (event) {
    var script = $('#action_commands').val();
    eval(script);
  });

  // store delegate button
  $container.find("#button_count").click(function (event) {
    // alert("count" + $('#dag_count_command').val());
    $('#action_commands').val($('#dag_count_command').val()).trigger('autosize.resize');
  });
};

///
/// show dag
///
var listRdd = function(tab) {
  var $container = $("#" + tab.id);
  $container.empty().append($("#dagGraphTemplate").render({}));

  idgs.dag.listRdd(function(data) {
    // $container.empty().html(JSON.stringify(data));
    idgs.dag.showDag(data);  
  });
};

///
/// RDD
///
var click_rdd = function () {
   $(dashboard_content).empty().jswt_tab({
        tabs: [
            {id: "transform_tab", label: "Transform", onclick: showTransform, icon_class: "fa fa-file"},
            {id: "action_tab", label: "Action", onclick: showAction, icon_class: "fa fa-file"},
            {id: "list_rdd_tab", label: "DAG", onclick: listRdd, icon_class: "fa fa-file"}
        ]
    });
}

///
/// SQL
///
var click_sql = function () {
   $(dashboard_content).empty().jswt_tab({
        tabs: [
            {id: "sql_tab", label: "SQL", onclick: showSQL, icon_class: "fa fa-file"}
        ]
    });
}

///
/// init dashboard
///
$.jswt_dashboard.init({
    title: "IDGS Management Console",
    menus: [
        {icon_class: "fa fa-dashboard", label: "Dashboard", desc: "Overview & Stats", onclick: click_dashboard},
        {icon_class: "fa fa-laptop", label: "Cluster", desc: "MembershipTable & PartitionTable", onclick: click_cluster},
        {icon_class: "fa fa-bar-chart-o", label: "Store", desc: "CRUD of store", onclick: click_store},
        {icon_class: "fa fa-pencil", label: "Computing", desc: "Computing DAG", onclick: click_rdd },
        {icon_class: "fa fa-user", label: "SQL", desc: "DDL & DML", onclick: click_sql},
        {icon_class: "fa fa-cogs", label: "Settings", desc: "Manage Settings", onclick: todo}
    ]
});

}));
}(jQuery));

