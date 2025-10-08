#include <iostream>
#include <vector>

struct MyType {
    int value;

    MyType() {
        std::cout << "Construct MyType(" << value << ")\n";
    }

    MyType(int v) : value(v) {
        std::cout << "Construct MyType(" << v << ")\n";
    }

    MyType(const MyType &other) : value(other.value) {
        std::cout << "Copy Construct MyType(" << other.value << ")\n";
    }

//    MyType(MyType &&other) noexcept: value(other.value) {
//        std::cout << "Move Construct MyType(" << other.value << ")\n";
//    }

    ~MyType() {
        std::cout << "Destruct MyType(" << value << ")\n";
    }
};

int main() {
    std::vector<std::unique_ptr<MyType>> v1;
    v1.emplace_back(std::make_unique<MyType>(1));
    v1.emplace_back(std::make_unique<MyType>(2));

    std::cout << "--- Copy v1 to v2 ---\n";
//    std::vector<std::unique_ptr<MyType>> v2 = v1;   // 拷贝构造，会调用拷贝构造函数

    std::cout << "--- Move v1 to v3 ---\n";
    std::vector<std::unique_ptr<MyType>> v3 = std::move(v1); // 移动构造，会调用移动构造函数

    std::cout << "--- Addresses ---\n";
//    std::cout << "v2[0] addr = " << &v2[0] << "\n";
    std::cout << "v3[0] addr = " << &v3[0] << "\n";

    return 0;
}
