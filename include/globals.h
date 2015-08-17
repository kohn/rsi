#include <iostream>
#include <map>


#define LOG_ERROR(MSG) std::cout << __FILE__ << ":" << __LINE__ << ":" << MSG << std::endl
#define LOG_INFO(MSG) std::cout << __FILE__ << ":" << __LINE__ << ":" << MSG << std::endl
#define DEBUG(VAR) std::cout << __FILE__ << ":" << __LINE__ << ":" << #VAR << ": " << VAR << std::endl;

