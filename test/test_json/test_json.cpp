#include "json.hpp"
using json = nlohmann::json;

#include <iostream>
#include <vector>
#include <map>
#include <string>
using namespace std;

// json 序列化示例1
string jsonTest1(){
    json js;
    js["mes_type"] = 2;  // 链式哈希表存储
    js["from"] = "zhy";
    js["to"] = "lulu";
    js["meg"] = "hello, chen, I'm learning!";
    cout << "msg1: " << js << endl;
    // msg1: {"from":"zhy","meg":"hello, chen, I'm learning!","mes_type":2,"to":"lulu"}

    string sendBuf = js.dump();  // 序列化json字符串
    cout << sendBuf.c_str() << endl;
    return sendBuf;
}

// 序列化示例2 -> 可以嵌套
void jsonTest2(){
    json js;
    js["mes_type"] = 2;  // 链式哈希表存储
    js["from"]["name"] = "zhy";
    js["to"]["name"] = "lulu";
    js["meg"] = {{"hello, chen, I'm learning!", "test1"},{"copmlete cpp project", "test2"}};
    cout << js << endl;
}

void jsonTest3(){
    vector<int> list;
    map<int, string> m;
    list.push_back(1);
    list.push_back(12);
    list.push_back(123);
    m.insert({1, "apple"});
    m.insert({2, "banana"});
    json js;
    js["list"] = list;  // 序列化一个vector对象
    js["map"] = m;      // 序列化一个map对象
    cout << js << endl;
}

int main() {
    
    auto receiveBuff = jsonTest1();
    // 反序列化
    auto buff = json::parse(receiveBuff);
    cout << buff["from"] << endl;
    cout << buff["to"] << endl;
    return 0;

};