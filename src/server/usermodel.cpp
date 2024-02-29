#include "usermodel.hpp"
#include "db.h"
#include <iostream>

using namespace std;

bool UserModel::insert(User &user)
{
    // 1.组装sql语句
    char sql[1024] = {0};
    sprintf(sql, "insert into user(name, password, state) values('%s','%s','%s')",
            user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());

    MySQL mysql;
    if (mysql.connect())
    {
        if (mysql.update(sql))
        {   
            // 获取插入成功的用户数据生成的主键ID
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
        
        
    }
    
    return false;
}