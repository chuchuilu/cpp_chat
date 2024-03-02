#include "chatservice.hpp"
#include "public.hpp"
#include <vector>

using namespace std;

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
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, _1, _2, _3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, _1, _2, _3)});
}

// 服务器异常，业务重置方法
void ChatService::reset()
{
    // 把online状态用户，设置成offline
    _userModel.resetState();
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
            // 登录成功，记录用户连接信息     涉及多线程同时增加到map，需要线程互斥操作
            {
                // 加锁
                lock_guard<mutex> lock(_connMutex);
                _userConnMap.insert({id, conn});
                // 解锁
            }
            // 登录成功,更新用户状态信息
            user.setState("online");
            _userModel.updateState(user);

            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0;
            response["id"] = user.getId();
            response["name"] = user.getName();
            // 查询该用户是否有离线消息，代入response并发送
            vector<string> vec = _offlineMsgModel.query(id);
            if (!vec.empty())
            {
                response["offlinemessage"] = vec;
                // 读取该用户的离线消息后，把该用户的所有离线信息删除
                _offlineMsgModel.remove(id);
            }
            // 查询该用户好友信息并返回
            vector<User> userVec = _friendModel.query(id);
            if (!userVec.empty())
            {
                vector<string> vec2;
                for(User &user: userVec)
                {
                    json js;
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }
            
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


// 处理客户端异常退出 逻辑：删除连接指针，用户在线状态 online -> offline
void ChatService::clientCloseException(const TcpConnectionPtr &conn)
{
    // 查找 要加锁 
    User user;
    {
        lock_guard<mutex> lock(_connMutex);

        for(auto it = _userConnMap.begin(); it != _userConnMap.end(); ++it)
        {
            if (it -> second == conn)
            {
                // 从map表删除用户的连接信息
                user.setId(it -> first);
                _userConnMap.erase(it);
                break;
            }
        }

    }

    // 更新用户的状态信息
    if(user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }

}

// 一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int toid = js["to"].get<int>();
    {
        lock_guard<mutex> lock(_connMutex);
        auto it = _userConnMap.find(toid);
        if(it != _userConnMap.end())    // 查询到在线
        {
            // toid在线，转发消息，服务器主动推送消息给toid用户
            it -> second -> send(js.dump());
            return;
        }
    }

    // toid不在线，存储离线消息
    _offlineMsgModel.insert(toid, js.dump());
}

// 添加好友业务 msgid id feiendid
void ChatService::addFriend(const TcpConnectionPtr &conn, json &js, Timestamp)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    // 存储好友信息
    _friendModel.insert(userid, friendid);
}