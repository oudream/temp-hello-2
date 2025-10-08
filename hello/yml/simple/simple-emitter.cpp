#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>

struct Person {
    std::string name;
    int age;
};

// 辅助函数：把 Person 输出到 YAML::Emitter
YAML::Emitter& operator<<(YAML::Emitter& out, const Person& p) {
    out << YAML::BeginMap;
    out << YAML::Key << "name" << YAML::Value << p.name;
    out << YAML::Key << "age"  << YAML::Value << p.age;
    out << YAML::Comment("这是一个示例对象");
    out << YAML::EndMap;
    return out;
}

int main() {
    // 构造一个对象
    Person alice{"Alice", 30};

    // 构造一个数组
    std::vector<Person> people = {
            {"Bob", 25}, {"Carol", 28}, {"Dave", 35}
    };

    YAML::Emitter out;

    out << YAML::BeginMap;   // 最外层是一个 map

    // 写入单个对象
    out << YAML::Key << "person" << YAML::Value << alice;

    // 写入数组
    out << YAML::Key << "people" << YAML::Value;
    out << YAML::BeginSeq;   // 开始数组
    for (const auto& p : people) {
        out << p;  // 调用我们重载的 operator<<
    }
    out << YAML::EndSeq;

    out << YAML::EndMap;     // 最外层结束

    // 输出到文件
    std::ofstream fout("saved.yaml");
    fout << out.c_str();
    fout.close();

    std::cout << "YAML 已保存到 saved.yaml" << std::endl;
    return 0;
}
