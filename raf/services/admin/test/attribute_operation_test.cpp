
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#if defined(__GNUC__) || defined(__clang__)
#include "idgs_gch.h"
#endif // GNUC_ $


#include <gtest/gtest.h>
#include "idgs/admin/admin_server.h"
#include "idgs/admin/admin_util.h"
#include "idgs/admin/rdd_admin.h"
#include "idgs/application.h"

using namespace idgs::admin;
using namespace idgs::rdd;
using namespace std;
using namespace idgs;

TEST(rdd, AttributePath_Base) {
  AttributePath path;
  EXPECT_EQ(true, path.parse("actor_framework.stateful_actor.queue.size;stateful_actor=XXXX"));

  LOG(INFO) << "path.getAttributePath(): " << path.getAttributePath();
  EXPECT_EQ("actor_framework.stateful_actor.queue.size",path.getAttributePath());

  std::string value;
  std::string name = "stateful_actor";
  path.getParameterValue(name, value);
  LOG(INFO) << "getParameterValue for stateful_actor " << value;
  EXPECT_EQ(value, "XXXX");

  int i = 0;
  std::string tmp = "";
  while(path.hasNext()) {
    if (i == 0) {
      tmp = "actor_framework";
    } else if (i == 1) {
      tmp = "stateful_actor";
    } else if (i == 2) {
      tmp = "queue";
    } else if (i == 3) {
      tmp = "size";
    }

    std::string next = path.next();
    LOG(INFO) << "next: " << next;
    EXPECT_EQ(0, next.compare(tmp));

    i++;
  }
}

TEST(rdd, AttributePath_TRIM) {
  AttributePath path;
  EXPECT_EQ(true, path.parse(" actor_framework.stateful_actor.queue.size; stateful_actor=XXXX "));

  LOG(INFO) << "path.getAttributePath(): " << path.getAttributePath();
  EXPECT_EQ("actor_framework.stateful_actor.queue.size",path.getAttributePath());

  std::string value;
  std::string name = "stateful_actor";
  path.getParameterValue(name, value);
  LOG(INFO) << "getParameterValue for stateful_actor " << value;
  EXPECT_EQ(value, "XXXX");

  int i = 0;
  std::string tmp = "";
  while(path.hasNext()) {
    if (i == 0) {
      tmp = "actor_framework";
    } else if (i == 1) {
      tmp = "stateful_actor";
    } else if (i == 2) {
      tmp = "queue";
    } else if (i == 3) {
      tmp = "size";
    }

    std::string next = path.next();
    LOG(INFO) << "next: " << next;
    EXPECT_EQ(0, next.compare(tmp));

    i++;
  }
}

TEST(rdd, AttributePath_NO_PARAM) {
  AttributePath path;
  EXPECT_EQ(true, path.parse(" actor_framework.stateful_actor.queue.size"));

  LOG(INFO) << "path.getAttributePath(): " << path.getAttributePath();
  EXPECT_EQ("actor_framework.stateful_actor.queue.size",path.getAttributePath());

  std::string value;
  std::string name = "stateful_actor";
  path.getParameterValue(name, value);
  LOG(INFO) << "getParameterValue for stateful_actor " << value;
  EXPECT_EQ(value, "");

  int i = 0;
  std::string tmp = "";
  while(path.hasNext()) {
    if (i == 0) {
      tmp = "actor_framework";
    } else if (i == 1) {
      tmp = "stateful_actor";
    } else if (i == 2) {
      tmp = "queue";
    } else if (i == 3) {
      tmp = "size";
    }

    std::string next = path.next();
    LOG(INFO) << "next: " << next;
    EXPECT_EQ(0, next.compare(tmp));

    i++;
  }
}

TEST(rdd, AttributePath_NO_PATH) {
  AttributePath path;
  EXPECT_EQ(false, path.parse(";stateful_actor=XXXX"));
}

TEST(rdd, AttributePath_ERROR_COMMA) {
  AttributePath path;
  EXPECT_EQ(false, path.parse("actor_framework.stateful_actor.queue.size;stateful_actor=XXXX;queue=YYYY"));
}

TEST(rdd, AttributePath_MULTI_PARAM) {
  AttributePath path;
  EXPECT_EQ(true, path.parse("actor_framework.stateful_actor.queue.size;stateful_actor=XXXX, queue=YYYY"));

  LOG(INFO) << "path.getAttributePath(): " << path.getAttributePath();
  EXPECT_EQ("actor_framework.stateful_actor.queue.size",path.getAttributePath());

  std::string value;
  std::string name = "stateful_actor";
  path.getParameterValue(name, value);
  LOG(INFO) << "getParameterValue for stateful_actor " << value;
  EXPECT_EQ(value, "XXXX");

  name = "queue";
  path.getParameterValue(name, value);
  LOG(INFO) << "getParameterValue for queue " << value;
  EXPECT_EQ(value, "YYYY");

  int i = 0;
  std::string tmp = "";
  while(path.hasNext()) {
    if (i == 0) {
      tmp = "actor_framework";
    } else if (i == 1) {
      tmp = "stateful_actor";
    } else if (i == 2) {
      tmp = "queue";
    } else if (i == 3) {
      tmp = "size";
    }

    std::string next = path.next();
    LOG(INFO) << "next: " << next;
    EXPECT_EQ(0, next.compare(tmp));

    i++;
  }
}

static const std::string attr1 = "admin.test.ADD_GET_MANAGENODE";
static const std::string MODULE_NAME = "admin.test";

class ManagedeNode1: public idgs::admin::ManageableNode {
  public:
    ManagedeNode1() {
    }
    ~ManagedeNode1() {
      attributesPath.clear();
    }

    bool init() {
      return true;
    }

    bool processRequest(const std::string& _model_name, AttributePathPtr& attr,
        idgs::actor::ActorMessagePtr& actorMsg) {
      if (_model_name == MODULE_NAME) {
        attr->getAttributePath() == attr1;
        return true;
      }
      return false;
    }
};

TEST(admin_server, ADD_GET_MANAGENODE) {
  ManagedeNode1* node1 = new ManagedeNode1();
  node1->setModuleName(MODULE_NAME);

  node1->addAttribute(attr1);

  idgs::admin::AdminServer& admin = ::idgs::util::singleton<idgs::admin::AdminServer>::getInstance();

  admin.init();
  admin.addManageableNode(node1);

  ManageableNode* node2 = admin.getManageableNode(MODULE_NAME, attr1);
  LOG(INFO) << "if get node2: " << node2;
  //EXPECT_NE(node2, NULL);

  LOG(INFO) << "get node2: " << node2;
  //EXPECT_NE(node2, NULL);

  LOG(INFO) << "get node2 module name: " << node2->getModuleName();
  EXPECT_EQ(node2->getModuleName(), MODULE_NAME);

  std::vector<std::string>& attrs =  node2->getAllAttributes();
  EXPECT_EQ(attrs.size(), 1);

/*  idgs::admin::AttributePathPtr attrPath(NULL);
  idgs::actor::ActorMessagePtr actorMsg(NULL);
  std::string operation = "get";
  OperationContext context;
  context.module_name = MODULE_NAME;
  context.attr = attrPath;
  context.actorMsg = actorMsg;
  EXPECT_EQ(true, node2->processRequest(context));*/

  LOG(INFO) << "get node2 attribute: " << attrs.at(0);
  EXPECT_EQ(attrs.at(0), attr1);

  ManageableNode* node3 = admin.getManageableNode("admin.test", "admin.test.ERROR");
  LOG(INFO) << "get node3 : " << node3;
  if (node3) {
    EXPECT_EQ(1,0);
  }
}

TEST(admin_util, toJsonItem) {
  std::string _test1;
  idgs::admin::util::toJsonItem("IP","10.10.10.1",_test1);
  LOG(INFO) << "IP is " << _test1;
  EXPECT_EQ(_test1,"\"IP\":\"10.10.10.1\"");

  std::string _test2;
  idgs::admin::util::toJsonItem("ID",10,_test2);
  LOG(INFO) << "ID is " << _test2;
  EXPECT_EQ(_test2,"\"ID\":\"10\"");

  std::string _test3;
  idgs::admin::util::toJsonItem("Is Local Store",true,_test3);
  LOG(INFO) << "Is Local Store is " << _test3;
  EXPECT_EQ(_test3,"\"Is Local Store\":\"1\"");

  std::string* ss = AGGREGATE_JSON_ITEMS(_test1, _test2, _test3);
  std::string content;
  content = *ss;
  delete ss;
  LOG(INFO) << "total string: " << content;
  EXPECT_EQ("{\"IP\":\"10.10.10.1\",\"ID\":\"10\",\"Is Local Store\":\"1\"}", content);

}

struct ApplicationSetting {
  ApplicationSetting():clusterConfig("") {}
  std::string clusterConfig;
};

TEST(rdd_admin, toJsonItem) {

  ApplicationSetting setting;
  // default valuel
  setting.clusterConfig = "framework/conf/cluster.conf";

  Application& app = ::idgs::util::singleton<Application>::getInstance();
  ResultCode rc;

  DVLOG(0) << "Loading configuration .";
  rc = app.init(setting.clusterConfig);
  if (rc != RC_SUCCESS) {
    LOG(ERROR) << "Failed to initialize server: " << getErrorDescription(rc);
    exit(1);
  }

  DVLOG(0) << "Server is starting.";
  rc = app.start();
  if (rc != RC_SUCCESS) {
    LOG(ERROR) << "Failed to start server: " << getErrorDescription(rc);
    return;
  }

  sleep(3);

  idgs::admin::rdd::RddDependencyNode* node = new idgs::admin::rdd::RddDependencyNode;
  RddActor* actor1 = new RddActor;
  RddActor* actor2 = new RddActor;
  RddActor* actor3 = new RddActor;

  idgs::pb::ActorId* actor1Id = new idgs::pb::ActorId;
  actor1Id->set_actor_id("@rdd1");
  actor1Id->set_member_id(0);

  idgs::pb::ActorId* actor2Id = new idgs::pb::ActorId;
  actor2Id->set_actor_id("@rdd2");
  actor2Id->set_member_id(1);

  idgs::pb::ActorId* actor3Id = new idgs::pb::ActorId;
  actor3Id->set_actor_id("@rdd3");
  actor3Id->set_member_id(2);

  actor1->setRddName("rdd1");
//  actor1->setActorId("@rdd1");
  actor2->setRddName("rdd2");
//  actor2->setActorId("@rdd2");
  actor3->setRddName("rdd3");
//  actor3->setActorId("@rdd3");

  actor1->addDependingRdd(*actor2Id, "rdd2");
  actor1->addDependingRdd(*actor3Id, "rdd3");
  actor2->addDependingRdd(*actor3Id, "rdd3");

  idgs::actor::ActorFramework* actorFramework =
      ::idgs::util::singleton<idgs::actor::RpcFramework>::getInstance().getActorFramework();

  actorFramework->Register(actor1->getActorId(), actor1);
  actorFramework->Register(actor2->getActorId(), actor2);
  actorFramework->Register(actor3->getActorId(), actor3);

  actor3->takeSnapShot()->save();
  actor2->takeSnapShot()->save();
  actor1->takeSnapShot()->save();

  string json;
  EXPECT_EQ(true, node->dependingRddInfo2Json(actor1, json));
  LOG(INFO) <<" rdd_admin, toJsonItem: " << json;

  map<string, shared_ptr<RddSnapshot>> childRdds;
  shared_ptr<RddSnapshot> actor1Snap = actor1->takeSnapShot();
  EXPECT_EQ(true, node->getDependency(*actor1Snap, childRdds));
  LOG(INFO) << "childs size " << childRdds.size();
  map<string, shared_ptr<RddSnapshot>>::iterator itr = childRdds.begin();
  for(;itr != childRdds.end();itr++) {
    LOG(INFO) << "rdd: " << itr->second->getSelfInfo().getRddName();
  }

  json.clear();
  LOG(INFO) << "json size " << json.size();
  node->dependency2Json(*actor1Snap, childRdds, json);
  LOG(INFO) << "dependency: " << json;
  //EXPECT_EQ("{\"rdd1\":[{\"rdd_name\":\"rdd2\",\"rdd_state\":\"0\",\"actor_state\":\"0\",\"pending_message\":\"0\",\"actor_id\":\"@rdd2\"},{\"rdd_name\":\"rdd3\",\"rdd_state\":\"0\",\"actor_state\":\"0\",\"pending_message\":\"0\",\"actor_id\":\"@rdd3\"}],\"rdd2\":[{\"rdd_name\":\"rdd3\",\"rdd_state\":\"0\",\"actor_state\":\"0\",\"pending_message\":\"0\",\"actor_id\":\"@rdd3\"}],\"rdd3\":[]}",json);


  sleep(2);

  DVLOG(1) << "Server is shutting down.";
  rc = app.stop();
  if (rc != RC_SUCCESS) {
    LOG(ERROR) << "Failed to shutdown server: " << getErrorDescription(rc);
  }
}

