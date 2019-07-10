#pragma once
#include "TcpConnection.h"

class TcpServer
{
public:
	TcpServer(io_service& ioService) : m_acceptor(ioService, ip::tcp::endpoint(ip::tcp::v4(), 6000))
	{
		m_funcConnectionHandler = NULL;
		m_nUserData = 0;
		m_bStop = false;
	}

	//新建连接回调函数
	typedef boost::function<void(TcpConnection * new_connection, int nUserData)>	CreateConnnectionCallbackHandler;

	//设置新建连接回调函数
	void setNewConnectionCallback(CreateConnnectionCallbackHandler fnHandler, int nUserData)
	{
		m_funcConnectionHandler = fnHandler;
		m_nUserData = nUserData;
	}

	//开始工作
	void startWork()
	{
		m_bStop = false;
		start_accept();
	}

	//停止工作
	void stopWork()
	{
		m_bStop = true;
		m_acceptor.close();
	}
private:
	void start_accept()
	{
		if (m_bStop)
		{
			return;
		}
		TcpConnection *new_connection = TcpConnection::create(m_acceptor.get_io_service());
		m_acceptor.async_accept(
		new_connection->socket(),
		boost::bind(&TcpServer::handle_accept,
		this,
		new_connection,
		boost::asio::placeholders::error));
	}
	void handle_accept(TcpConnection * new_connection,
		const boost::system::error_code& error)
	{
		if (!error)
		{
			if (m_funcConnectionHandler != NULL)
			{
				m_funcConnectionHandler(new_connection, m_nUserData);
			}
			new_connection->sendData("abcdefg", strlen("abcdefg"), NULL, 0, 0);
			start_accept();
		}
	}

	ip::tcp::acceptor m_acceptor;
	CreateConnnectionCallbackHandler m_funcConnectionHandler;
	int	    m_nUserData;
	bool	m_bStop;
};

