/*
 * network_interface.h
 *
 *  Created on: Jan 21, 2014
 *      Author: root
 */

#ifndef NETWORK_INTERFACE_H_
#define NETWORK_INTERFACE_H_
#include <vector>
#include <iostream>

#include "idgs/result_code.h"

namespace idgs {
namespace net {

struct network {
  static idgs::ResultCode getInterfaceAddress(std::vector<std::string>& addrs);
};

} // end namespace util
} // end namespace idgs
#endif /* NETWORK_INTERFACE_H_ */
