//Source: https://godbolt.org/z/YsWEoczsW
//Presentation: https://www.youtube.com/watch?v=Oy-VTqz1_58

#include <cassert>
#include <future>
#include <iostream>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include <stdexcept>
#include <source_location>
#include <map>
#include <stacktrace>

template<typename DATA_T>
class OmegaException {
public:
    OmegaException(const char* str, const DATA_T& data, const std::source_location& loc = std::source_location::current(),
      std::stacktrace trace = std::stacktrace::current())
      :err_str(str), data_(data), location_{loc}, backtrace_{trace}{}
    OmegaException(std::string str, DATA_T& data, const std::source_location& loc = std::source_location::current(),
      std::stacktrace trace = std::stacktrace::current())
      :err_str(std::move(str)),data_(std::move(data)), location_{loc}, backtrace_{trace}{}
    DATA_T& data(){ return data_;}
    const DATA_T& data() const noexcept { return data_;}
    std::string& what(){return err_str;}
    const std::string& what() const noexcept {return err_str;}
    const std::source_location& where() const {return location_;}
    const std::stacktrace& stack() const {return backtrace_;}
    
private:
    std::string err_str;
    DATA_T data_;
    const std::source_location location_;
    const std::stacktrace backtrace_;
};

std::ostream& operator << (std::ostream& os, const std::source_location& location) {
    os << location.file_name() << "("
       << location.line() << ":"
       << location.column() << "), function `"
       << location.function_name() << "`";
    return os;
}

std::ostream& operator << (std::ostream& os, const std::stacktrace& backtrace) {
    for(auto iter = backtrace.begin(); iter != (backtrace.end()-3); ++iter) {
        os << iter->source_file() << "(" << iter->source_line() 
        << ") : " << iter->description() << "\n";
    }
    return os;
}

using MyExceptionErrsVoid = OmegaException<void*>;

struct Order {
  unsigned int id_;
  double value_;
};

std::ostream& operator << (std::ostream& os, const Order& order) {
    os << "value : " << order.value_;
    return os;
}

std::map<unsigned int, Order> orders{{1,Order{1,2.0}},{11, Order{11,5.0}}};

Order UpdateOrder(const Order& ord) {
    auto it = orders.find(ord.id_);
    if(it == orders.end())
      throw MyExceptionErrsVoid("update error : ", nullptr);
    return it->second = ord;
}

Order findOrder(unsigned int id) {
    auto it = orders.find(id);
    if(it == orders.end())
      throw MyExceptionErrsVoid("Bad Order id", nullptr);
    return it->second;
}

bool processOrder(unsigned int id) {
    try {
      Order ord = findOrder(id);
      std::cout << "Found order id : " << id << " : " << ord << std::endl;
      return true;
    }
    catch(MyExceptionErrsVoid& e) {
      std::cout << "[where]Failed to process : " << e.what() << " : " << e.where() << std::endl;
      std::cout << "[stack]Failed to process : " << e.what() << "\n" << e.stack() << std::endl;
    }
    return false;
}


int main() {
    try {
      unsigned int id = 10;
      if(processOrder(id) == 1)
          std::cout << "success" << std::endl;
    }
    catch(...) {
        std::cout << "Unknown exception" << std::endl;
    }
    std::cout << "End" << std::endl;
}
