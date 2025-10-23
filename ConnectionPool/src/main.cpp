#include <iostream>
using namespace std;
#include "CommonConnectionPool.h"
#include "Connection.h"

int main() {
    // for (int i = 0; i < 1000; ++i) {
    //     Connection conn;
    //     char sql[1024] = {0};
    //     sprintf(sql, "insert into user(name, age, sex) values('%s','%d','%s')", "zhangsan", 20, "male");
    //     conn.connect("127.0.0.1", 3306, "root", "123456", "mytest");
    //     conn.update(sql);
    // }
    // return 0;
    // Connection conn;
    // conn.connect("127.0.0.1", 3306, "root", "123456", "mytest");

    clock_t begin = clock();
	
	thread t1([]() {
		ConnectionPool *cp = ConnectionPool::getInstance();
		for (int i = 0; i < 25; ++i)
		{
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
				"zhang san", 20, "male");
			shared_ptr<Connection> sp = cp->getConnection();
			sp->update(sql);
			// Connection conn;
			// char sql[1024] = { 0 };
			// sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
			// 	"zhang san", 20, "male");
			// conn.connect("127.0.0.1", 3306, "root", "123456", "mytest");
			// conn.update(sql);
		}
	});
	thread t2([]() {
		ConnectionPool *cp = ConnectionPool::getInstance();
		for (int i = 0; i < 25; ++i)
		{
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
				"zhang san", 20, "male");
			shared_ptr<Connection> sp = cp->getConnection();
			sp->update(sql);
			// Connection conn;
			// char sql[1024] = { 0 };
			// sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
			// 	"zhang san", 20, "male");
			// conn.connect("127.0.0.1", 3306, "root", "123456", "mytest");
			// conn.update(sql);
		}
	});
	thread t3([]() {
		ConnectionPool *cp = ConnectionPool::getInstance();
		for (int i = 0; i < 25; ++i)
		{
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
				"zhang san", 20, "male");
			shared_ptr<Connection> sp = cp->getConnection();
			sp->update(sql);
			// Connection conn;
			// char sql[1024] = { 0 };
			// sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
			// 	"zhang san", 20, "male");
			// conn.connect("127.0.0.1", 3306, "root", "123456", "mytest");
			// conn.update(sql);
		}
	});
	thread t4([]() {
		ConnectionPool *cp = ConnectionPool::getInstance();
		for (int i = 0; i < 25; ++i)
		{
			char sql[1024] = { 0 };
			sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
				"zhang san", 20, "male");
			shared_ptr<Connection> sp = cp->getConnection();
			sp->update(sql);
			// Connection conn;
			// char sql[1024] = { 0 };
			// sprintf(sql, "insert into user(name,age,sex) values('%s',%d,'%s')",
			// 	"zhang san", 20, "male");
			// conn.connect("127.0.0.1", 3306, "root", "123456", "mytest");
			// conn.update(sql);
		}
	});

	t1.join();
	t2.join();
	t3.join();
	t4.join();

	clock_t end = clock();
	cout << (end - begin) << "ms" << endl;
}