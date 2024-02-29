#include "chatservice.hpp"
#include "public.hpp"

// using namespace muduo;
// 获取单例对象的接口函数
// ChatService* ChatService::instance()
// {
//     static ChatService service;
//     return &service;
// }
ChatService& ChatService::instance() {
    static ChatService instance;
    return instance;
}


// 注册消息以及对应的Handler回调操作
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, _1, _2, _3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg, this, _1, _2, _3)});
}

// 获取消息对应的处理器
MsgHandler ChatService::getHandler(int msgid)
{
    // 记录错误日志，msg没有对应的事件处理回调
    auto it = _msgHandlerMap.find(msgid);
    if (it == _msgHandlerMap.end())     // 没有此类型
    {
        // LOG_ERROR << "megid: " << msgid << "can not find handler!!!";
        return [=](const TcpConnectionPtr &conn, json &js, Timestamp){
            LOG_ERROR << "megid: " << msgid << "can not find handler!!!";
            // LOG_ERROR << "megid: " ;
        };
    }
    else
    {
        return _msgHandlerMap[msgid];
    }
    
    
}

// 处理登录业务
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO << "DO LOGIN !!!";

}

// 处理注册业务
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    LOG_INFO << "DO REG !!!";
}