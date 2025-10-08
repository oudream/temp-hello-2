// MyStruct::* 这种写法专门表示“指向成员”的类型。
//
//成员指针本身不是偏移量，而是编译器实现的内部描述（多继承时可能比较复杂）。
//
//使用时要通过 obj.*pMember 或 pObj->*pMember 来解引用。

#include <iostream>
using namespace std;

struct MyStruct {
    void print(int x) { cout << "print: " << x << endl; }
};

// 定义一个成员函数指针：返回类型 void，参数 (int)
void (MyStruct::*pFunc)(int) = &MyStruct::print;

int main() {
    MyStruct obj;
    // 调用成员函数指针
    (obj.*pFunc)(10);

    MyStruct* pObj = &obj;
    (pObj->*pFunc)(20);
}
