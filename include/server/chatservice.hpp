#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <muduo/net/TcpConnection.h>
#include <unordered_map>
#include <functional>
#include "json.hpp"
#include "usermodel.hpp"
#include "offlinemessagemodel.hpp"
#include <mutex>
#include <muduo/base/Logging.h> // 确保包含了Muduo的日志头文件
using namespace std;
using namespace muduo;
using namespace muduo::net;


using json = nlohmann::json;

// 表示处理消息的事件回调方法类型
using MsgHandler = std::function<void(const TcpConnectionPtr &conn, json &js, Timestamp)>; // 函数封装

// 聊天服务器业务类
class ChatService
{
public:
    // 获取单例对象的接口函数
    // 禁止拷贝和赋值
    ChatService(const ChatService&) = delete;
    ChatService& operator=(const ChatService&) = delete;
    static ChatService& instance();
    // 处理登录业务
    void login(const TcpConnectionPtr &conn, json &js, Timestamp);
    // 处理注册业务
    void reg(const TcpConnectionPtr &conn, json &js, Timestamp);
    // 处理一对一聊天服务
    void oneChat(const TcpConnectionPtr &conn, json &js, Timestamp);
    // 获取消息对应的处理器
    MsgHandler getHandler(int msgid);
    // 处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr &conn);
private:
    ChatService();

    // 存储详细id和对象的业务处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;

    // 存储在线用户的通信连接
    unordered_map<int, TcpConnectionPtr> _userConnMap;

    // 互斥锁保证_user_ConnMap线程安全
    mutex _connMutex;

    // 数据操作类对象
    UserModel _userModel;
    OfflineMessageModel _offlineMsgModel;

};




#endif