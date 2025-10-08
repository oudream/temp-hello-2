#include <iostream>
#include <fstream>
#include <yaml-cpp/yaml.h>

int main() {
    // 1. 加载 YAML 文件
    YAML::Node root = YAML::LoadFile("app.yaml");

    // =======================
    // 2. 读取单个对象 (app)
    // =======================
    YAML::Node appNode = root["app"];
    std::string appName = appNode["name"].as<std::string>();
    int version = appNode["version"].as<int>();
    std::string host = appNode["server"]["host"].as<std::string>();
    int port = appNode["server"]["port"].as<int>();

    std::cout << "App: " << appName
              << " v" << version
              << " (" << host << ":" << port << ")\n";

    // =======================
    // 3. 读取对象数组 (users)
    // =======================
    YAML::Node usersNode = root["users"];
    for (std::size_t i = 0; i < usersNode.size(); i++) {
        std::string uname = usersNode[i]["name"].as<std::string>();
        int uage = usersNode[i]["age"].as<int>();
        std::cout << "User[" << i << "]: " << uname << ", age=" << uage << "\n";
    }

    // =======================
    // 4. 修改内容
    // =======================
    root["app"]["version"] = version + 1;   // 改版本号
    YAML::Node newUser;
    newUser["name"] = "Carol";
    newUser["age"] = 28;
    root["users"].push_back(newUser);       // 加新用户

    // =======================
    // 5. 保存到新文件
    // =======================
    std::ofstream fout("app.out.yaml");
    fout << root;
    fout.close();

    std::cout << "已保存到 app.out.yaml\n";
    return 0;
}
