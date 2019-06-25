
#ifndef SIM_COMMON_H_INCLUDED
#define SIM_COMMON_H_INCLUDED

#include "cpp_utils.h"
#include <string>
#include <map>

namespace sim {

using PropertyList = std::unordered_map<std::string, acpp::unstructured_value>;

} // namespace sim

#endif // SIM_COMMON_H_INCLUDED
