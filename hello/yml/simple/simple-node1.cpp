#include <iostream>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

// 定义对象
struct Person {
    std::string name;
    int age;
};

// 从 YAML Node 转换为 Person
namespace YAML {
    template<>
    struct convert<Person> {
        static bool decode(const Node& node, Person& p) {
            if (!node.IsMap()) {
                return false;
            }
            p.name = node["name"].as<std::string>();
            p.age  = node["age"].as<int>();
            return true;
        }
    };
}

int main() {
    try {
        YAML::Node config = YAML::LoadFile("example.yaml");

        // 加载单个对象
        if (config["person"]) {
            Person single = config["person"].as<Person>();
            std::cout << "Single Person -> Name: " << single.name
                      << ", Age: " << single.age << std::endl;
        }

        // 加载数组
        if (config["people"]) {
            std::vector<Person> people = config["people"].as<std::vector<Person>>();
            std::cout << "People list:" << std::endl;
            for (const auto& p : people) {
                std::cout << "- " << p.name << " (" << p.age << ")" << std::endl;
            }
        }

    } catch (const YAML::Exception& e) {
        std::cerr << "YAML Error: " << e.what() << std::endl;
    }
    return 0;
}
