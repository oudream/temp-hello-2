#ifndef CX_CT_X2_YAML_HELPER_H
#define CX_CT_X2_YAML_HELPER_H


#include <ccxx/configuration.h>


namespace cfg {

// 由于不使用 Visitor，这里通过 dynamic_cast 分派 Field* 的具体类型
    class ConfigIOYAML : public ConfigIOBase {
    public:
        bool loadFromFile(ConfigItem *root, const std::string &path, std::string *err) override;

        bool saveToFile(const ConfigItem *root, const std::string &path, std::string *err) override;

    };

} // namespace cfg

#endif //CX_CT_X2_YAML_HELPER_H
