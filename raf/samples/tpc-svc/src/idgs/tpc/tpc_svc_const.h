/*
 Copyright (c) <2013>, Intel Corporation All Rights Reserved.

 The source code, information and material ("Material") contained herein is owned by Intel Corporation or its suppliers or licensors, and title to such Material remains with Intel Corporation or its suppliers or licensors. The Material contains proprietary information of Intel or its suppliers and licensors. The Material is protected by worldwide copyright laws and treaty provisions. No part of the Material may be used, copied, reproduced, modified, published, uploaded, posted, transmitted, distributed or disclosed in any way without Intel's prior express written permission. No license under any patent, copyright or other intellectual property rights in the Material is granted to or conferred upon you, either expressly, by implication, inducement, estoppel or otherwise. Any license under such intellectual property rights must be express and approved by Intel in writing.

 Unless otherwise agreed by Intel in writing, you may not remove or alter this notice or any other notice embedded in Materials by Intel or Intel’s suppliers or licensors in any way.
 */
#pragma once
namespace idgs {
namespace tpc {

/// TPCH Store Name
static const std::string& TPCH_STORE_STORE_LINEITEM = "LineItem";
static const std::string& TPCH_STORE_CUSTOMER = "Customer";
static const std::string& TPCH_STORE_SUPPLIER = "Supplier";
static const std::string& TPCH_STORE_PART = "Part";
static const std::string& TPCH_STORE_NATION = "Nation";
static const std::string& TPCH_STORE_PARTSUPP = "PartSupp";
static const std::string& TPCH_STORE_REGION = "Region";
static const std::string& TPCH_STORE_ORDERS = "Orders";

/// SSB Store Name
static const std::string& SSB_STORE_LINEORDER = "ssb_lineorder";
static const std::string& SSB_STORE_CUSTOMER = "ssb_customer";
static const std::string& SSB_STORE_SUPPLIER = "ssb_supplier";
static const std::string& SSB_STORE_PART = "ssb_part";
static const std::string& SSB_STORE_DATE = "ssb_date";

// RDD samples
static const std::string& TPCH_Q6_TRANSFORMER = "TPCH_Q6_TRANSFORMER";
static const std::string& TPCH_Q6_ACTION = "TPCH_Q6_ACTION";

static const std::string& SSB_Q1_1_TRANSFORMER = "SSB_Q1_1_TRANSFORMER";
static const std::string& SSB_Q1_1_ACTION = "SSB_Q1_1_ACTION";

static const std::string& SSB_Q1_1_ORDER_PARAM = "SSB_Q1_1_ORDER_PARAM";
static const std::string& SSB_Q1_1_DATE_PARAM = "SSB_Q1_1_DATE_PARAM";

static const std::string& PARTITION_COUNT_ACTION = "PARTITION_COUNT_ACTION";
}
}

