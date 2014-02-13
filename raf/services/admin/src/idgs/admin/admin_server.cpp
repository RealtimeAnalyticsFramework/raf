
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#include "admin_server.h"
#include "cluster_admin.h"
#include "rdd_admin.h"
#include "net_admin.h"
#include "idgs/application.h"
#include "idgs/util/utillity.h"
#include "protobuf/message_helper.h"

using namespace idgs::admin::cluster;
using namespace idgs::admin::rdd;
using namespace idgs::admin::net;

namespace idgs {
namespace admin {
void AdminServer::addManageableNode(ManageableNode* node) {
  Module2NodesMapType::iterator itr = module2NodesMap.find(node->getModuleName());
  Attr2NodeMapType* nodesMap = NULL;
  if (itr == module2NodesMap.end()) {
    nodesMap = new Attr2NodeMapType();
    DVLOG(4) << "create Module2NodesMap for " << node->getModuleName();
    module2NodesMap.insert(make_pair(node->getModuleName(), nodesMap));
  } else {
    nodesMap = itr->second;
  }

  std::vector<std::string>& attrPaths = node->getAllAttributes();
  std::vector<std::string>::iterator itr2;
  for (itr2 = attrPaths.begin(); itr2 != attrPaths.end(); ++itr2) {
    DVLOG(4) << "insert attribute " << *itr2 << ", node:" << node << " for " << node->getModuleName();
    nodesMap->insert(make_pair(*itr2, node));
  }
}

ManageableNode* AdminServer::getManageableNode(const std::string& module_name,
    const std::string& attribute_path) {
  ManageableNode* node = NULL;
  Module2NodesMapType::iterator itr = module2NodesMap.find(module_name);
  Attr2NodeMapType* attr2NodesMap = NULL;
  if (itr == module2NodesMap.end()) {
    LOG(WARNING) << "can not find Module2Nodes map for " << module_name << "," << attribute_path ;
    return node;
  }

  attr2NodesMap = itr->second;
  Attr2NodeMapType::iterator it = attr2NodesMap->find(attribute_path);
  if (it == attr2NodesMap->end()) {
    LOG(WARNING) << "can not find Attr2NodesMap for " << module_name << "," << attribute_path ;
    return node;
  }

  node = it->second;
  return node;
}

bool AdminServer::init() {
  ClusterAdmin* clusterAdmin = new ClusterAdmin;
  clusterAdmin->init();
  initAdminNode(clusterAdmin);

  RddAdmin* rddAdmin = new RddAdmin;
  rddAdmin->init();
  initAdminNode(rddAdmin);

  NetAdmin* netAdmin = new NetAdmin;
  netAdmin->init();
  initAdminNode(netAdmin);

  adminNodes.push_back(clusterAdmin);
  adminNodes.push_back(rddAdmin);
  adminNodes.push_back(netAdmin);


  return true;
}

bool AdminServer::initAdminNode(idgs::admin::AdminNode* admiNode) {
  std::vector<ManageableNode*>& nodes = admiNode->getAllManageableNode();
  std::vector<ManageableNode*>::iterator itr = nodes.begin();
  for (;itr < nodes.end();++itr) {
    this->addManageableNode(*itr);
  }

  return true;
}

//////////////////////// Admin HttpAsyncServlet /////////////////////////////////////

AdminAsyncServlet::AdminAsyncServlet():
    name("AdminAsyncServlet"){
}

AdminAsyncServlet::~AdminAsyncServlet(){
  VLOG(3) << "AdminAsyncServlet is deleted";
}


// 1) The request uri should be /actor_name/admin_module/admin_attributes?parameter=parameter_value,parameter=parameter_value
//                              /  root    /cell[0]     /   cell[1]      / parameters
//                                         /<-- request path --------------------------------------------------------------->/
void AdminAsyncServlet::doGet(idgs::http::server::HttpRequest& req) {
  VLOG(2) << "doGet for http req: " << req.getRequestPath();
  std::string actorName = req.getRootPath();

  std::shared_ptr<google::protobuf::Message> payload;
  if (!getPayload(req.getRequestPath(), payload) ) {
    LOG(ERROR) << "get pay load error for request " << req.getRequestPath();
    idgs::http::server::HttpResponse rep =
        idgs::http::server::HttpResponse::createReply(idgs::http::server::HttpResponse::bad_request, "get pay load error");
    handler(rep);
    return;
  }

  if (req.getBody() != "") {
    ///
  }

  idgs::actor::ActorMessagePtr actorMsg(new idgs::actor::ActorMessage);

  actorMsg->setSourceActorId(getActorId());
  int32_t localMemId = idgs::util::singleton<idgs::Application>::getInstance().getMemberManager()->getLocalMemberId();
  actorMsg->setSourceMemberId(localMemId);
  actorMsg->setOperationName("get");
  actorMsg->setPayload(payload);
  actorMsg->setDestActorId(actorName);
  actorMsg->setDestMemberId(actorMsg->getSourceMemberId());

  VLOG(3) << "has sent http actor message: " << actorMsg->toString();
  idgs::util::singleton<idgs::actor::RpcFramework>::getInstance().getActorFramework()->sendMessage(actorMsg);
}

bool AdminAsyncServlet::getPayload(std::string& url, std::shared_ptr<google::protobuf::Message>& payload) {

  VLOG(3) << "will parse URL[" << url << "] to json";

  std::vector<std::string> cells;
  idgs::str::split(url, "/", cells);

  int size = cells.size();
  VLOG(3) << "cells size is " << size;

  std::string& module_name = cells[0];
  payload =::idgs::util::singleton<protobuf::MessageHelper>::getInstance().createMessage(ADMIN_PB_TYPE);

  if (payload.get() == NULL) {
    LOG(ERROR) << "can not get payload for pb type" << ADMIN_PB_TYPE;
    return false;
  }

  VLOG(4) << "create payload with PB type " << payload->GetTypeName();

  std::string attributes="";
  int i=1;
  for (;i<cells.size() -1;++i) { // add attributes, exclude the last attribute
    std::string& attr = cells[i];
    attributes.append(attr);
    attributes.append(".");
  }

  std::string parameter="";
  if (cells.size() >= 1) { // at least has one attributes.
    std::string last_attr = cells[i];
    VLOG(3) << "the last one is " << last_attr;

    std::vector<std::string> attr_plus_param;
    idgs::str::split(last_attr, "?", attr_plus_param);
    std::string& attr = attr_plus_param[0];

    attributes.append(attr);
    attributes.append(".");

    if (attr_plus_param.size() == 2) {// has parameters
      parameter = attr_plus_param[1];
    }
  }

  if (!attributes.empty()) {
    attributes.pop_back();
  }

  DVLOG(3) << "get attributes: " << attributes;
  DVLOG(3) << "get parameter: " << parameter;

  std::string json_payload;
  if (attributes.empty()) {
    int pos = ONLY_MODULE_NAME_PAYLOAD.find(MODULE_PLACE_HOLDER_STR);
    ONLY_MODULE_NAME_PAYLOAD.replace(pos, MODULE_PLACE_HOLDER_STR.size(), module_name);
    json_payload = ONLY_MODULE_NAME_PAYLOAD;
  } else {
    if (!attributes.empty() && parameter.empty()) {
      int pos = MODULE_NAME_ATTRIBUE_PAYLOAD.find(MODULE_PLACE_HOLDER_STR);
      MODULE_NAME_ATTRIBUE_PAYLOAD.replace(pos, MODULE_PLACE_HOLDER_STR.size(), module_name);

      pos = MODULE_NAME_ATTRIBUE_PAYLOAD.find(ATTR_PLACE_HOLDER_STR);
      MODULE_NAME_ATTRIBUE_PAYLOAD.replace(pos, ATTR_PLACE_HOLDER_STR.size(), attributes);
      json_payload = MODULE_NAME_ATTRIBUE_PAYLOAD;

    } else if (!attributes.empty() && !parameter.empty()) {
      int pos = FULL_PAYLOAD.find(MODULE_PLACE_HOLDER_STR);
      FULL_PAYLOAD.replace(pos, MODULE_PLACE_HOLDER_STR.size(), module_name);

      pos = FULL_PAYLOAD.find(ATTR_PLACE_HOLDER_STR);
      FULL_PAYLOAD.replace(pos, MODULE_PLACE_HOLDER_STR.size(), attributes);

      pos = FULL_PAYLOAD.find(PARAM_PLACE_HOLDER_STR);
      FULL_PAYLOAD.replace(pos, PARAM_PLACE_HOLDER_STR.size(), parameter);
      json_payload = FULL_PAYLOAD;
    }
  }

  DVLOG(3) << "After replacement, the payload is : \n" << json_payload;


  idgs::ResultCode code = protobuf::JsonMessage::parseJsonFromString(payload.get(), json_payload);
  if (code != RC_OK) {
    LOG(ERROR) << "can not get payload, result code is " << idgs::getErrorDescription(code);
    return false;
  }

  return true;
}

void AdminAsyncServlet::getAttachments(idgs::http::server::HttpRequest& req, google::protobuf::Message* message) {

}

}
}

