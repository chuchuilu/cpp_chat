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

// 处理登录业务  验证 id pwd 更新在线状态
void ChatService::login(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    int id = js["id"];
    string pwd = js["password"];

    User user = _userModel.query(id);
    if (user.getId() == id && user.getPwd() == pwd)
    {
        if (user.getState() == "online")
        {
            // 用户已登录，不允许重复登陆
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2;
            response["errmsg"] = "账号已登录";
            conn -> send(response.dump());
        }
        else
        {
            // 登录成功,更新用户状态信息
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            conn -> send(response.dump());
        }
    }
    else
    {
        // 用户名或密码错误，登录失败
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1;
        response["errmsg"] = "用户名或密码错误";
        conn -> send(response.dump());

    }
    

}

// 处理注册业务 name password
void ChatService::reg(const TcpConnectionPtr &conn, json &js, Timestamp time)
{
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);
    bool state = _userModel.insert(user);
    if (state)
    {
        // 注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0;
        response["id"] = user.getId();
        conn -> send(response.dump());
    }
    else
    {
        // 注册失败
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1;
        conn -> send(response.dump());
    }
    
}