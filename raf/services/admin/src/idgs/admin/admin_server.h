
/*
Copyright (c) <2013>, Intel Corporation All Rights Reserved.

The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
*/
#pragma once

#include "manageable_node.h"
#include <map>
#include <string>
#include <vector>
#include "admin_node.h"
#include "idgs/httpserver/http_servlet.h"

namespace idgs {
namespace admin {
typedef std::map<std::string, ManageableNode*> Attr2NodeMapType;
typedef std::map<std::string, Attr2NodeMapType*> Module2NodesMapType;
class AdminServer {
  public:
    void addManageableNode(ManageableNode* node);
    bool init();
    ManageableNode* getManageableNode(const std::string& module_name, const std::string& attribute_path);
  private:
    bool initAdminNode(idgs::admin::AdminNode* node);
    Module2NodesMapType module2NodesMap;
    std::vector<idgs::admin::AdminNode*> adminNodes;
};

class AdminAsyncServlet : public idgs::http::server::HttpAsyncServlet {
  public:
    AdminAsyncServlet();
    ~AdminAsyncServlet();
    void init() {}
    std::string& getName() {
      return name;
    }
    void doGet(idgs::http::server::HttpRequest& req);

  public:
    bool getPayload(std::string& url, std::shared_ptr<google::protobuf::Message>& payload);
    void getAttachments(idgs::http::server::HttpRequest& req, google::protobuf::Message* message);
  private:
    std::string name;
    std::string ADMIN_PB_TYPE="idgs.admin.pb.AdminRequest";
    std::string MODULE_PLACE_HOLDER_STR = "XXXX";
    std::string ATTR_PLACE_HOLDER_STR = "YYYY";
    std::string PARAM_PLACE_HOLDER_STR = "ZZZZ";
    std::string ONLY_MODULE_NAME_PAYLOAD =
        "{\"module_op_request\":[{\"module_name\":\"XXXX\"}]}";
    std::string MODULE_NAME_ATTRIBUE_PAYLOAD =
        "{\"module_op_request\":[{\"module_name\":\"XXXX\",\"attributes\":[{\"attribute\":\"YYYY\"}]}]}";
    std::string FULL_PAYLOAD =
        "{\"module_op_request\":[{\"module_name\":\"XXXX\",\"attributes\":[{\"attribute\":\"YYYY;ZZZZ\"}]}]}";
};

}
}
