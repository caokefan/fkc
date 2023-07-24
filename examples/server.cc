#include "../src/TcpServer.h"
#include <string>
#include <functional>
#include <iostream>

using namespace std;
using namespace placeholders;

class EchoServer
{
public:
    EchoServer(EventLoop *loop, InetAddress &addr, string name)
        : server_(loop, addr, name), loop_(loop)
    {
        server_.set_connection_callback(bind(&EchoServer::on_connection, this, _1));
        server_.set_message_callback(bind(&EchoServer::on_message, this, _1, _2));

        server_.set_thread_num(3);
    }
    void start()
    {
        server_.start();
    }

private:
    void on_connection(const TcpConnectionPtr &conn)
    {
        if (conn->connected())
        {
            cout << "conn up" << conn->get_peeraddr().get_ip_port().c_str() << endl;

        } else {
            cout << "conn down" << conn->get_peeraddr().get_ip_port().c_str();
        }
    }

    void on_message(const TcpConnectionPtr &conn, Buffer *buffer)
    {
        string msg = buffer->retrieve_all_asString();
        conn->send(msg);
        conn->shutdown();
    }

private:
    EventLoop *loop_;
    TcpServer server_;
};

int main()
{
    EventLoop loop;
    InetAddress addr(8000);
    EchoServer server(&loop, addr, "echo 01");
    server.start();
    loop.loop(); 
    return 0;
}