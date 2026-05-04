#include "config.h"
#include <iostream>

namespace dune {

bool Config::load(const std::string& filepath) {
    try {
        boost::property_tree::read_json(filepath, pt_);
        return true;
    } catch (const boost::property_tree::json_parser_error& e) {
        std::cerr << "[Config] JSON parse error: " << e.message()
                  << " (line " << e.line() << ")" << std::endl;
        return false;
    } catch (const std::exception& e) {
        std::cerr << "[Config] Failed to load " << filepath
                  << ": " << e.what() << std::endl;
        return false;
    }
}

} // namespace dune
