#ifndef DUNE_CONFIG_H_
#define DUNE_CONFIG_H_

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace dune {

class Config {
public:
    Config() = default;

    // 从 JSON 文件加载配置，失败返回 false
    bool load(const std::string& filepath);

    // 获取指定 key 的值，不存在时返回默认值
    template<typename T>
    T get(const std::string& key, const T& default_val = T{}) const {
        return pt_.get<T>(key, default_val);
    }

    // 获取子配置树
    boost::property_tree::ptree get_child(const std::string& key) const {
        return pt_.get_child(key);
    }

    // 是否包含某个 key
    bool has(const std::string& key) const {
        return pt_.find(key) != pt_.not_found();
    }

    const boost::property_tree::ptree& pt() const { return pt_; }

private:
    boost::property_tree::ptree pt_;
};

} // namespace dune

#endif // DUNE_CONFIG_H_
