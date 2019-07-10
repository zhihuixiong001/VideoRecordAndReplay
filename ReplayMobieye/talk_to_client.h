#pragma once
#include <boost/bind.hpp>

#include <boost/asio.hpp>

#include <boost/shared_ptr.hpp>

#include <boost/enable_shared_from_this.hpp>

using namespace boost::asio;

using namespace boost::posix_time;

extern io_service service;
const int blocksize = 1440;
struct recvbuf//包格式  
{
	char buf[blocksize];//存放数据的变量  
	int flag;//标志   
	int cnt;
};

extern struct recvbuf g_data;

#define MEM_FN(x)       boost::bind(&self_type::x, shared_from_this())

#define MEM_FN1(x,y)    boost::bind(&self_type::x, shared_from_this(),y)

#define MEM_FN2(x,y,z)  boost::bind(&self_type::x, shared_from_this(),y,z)

class talk_to_client : public boost::enable_shared_from_this<talk_to_client>, boost::noncopyable {

	typedef talk_to_client self_type;

	talk_to_client() : sock_(service), started_(false) {}

public:

	typedef boost::system::error_code error_code;

	typedef boost::shared_ptr<talk_to_client> ptr;



	void start() {

		started_ = true;

		do_read();

	}

	static ptr new_() {

		ptr new_(new talk_to_client);

		return new_;

	}

	void stop() {

		if (!started_) return;

		started_ = false;

		sock_.close();

	}

	ip::tcp::socket & sock() { return sock_; }

private:

	void on_read(const error_code & err, size_t bytes) {

		if (!err) {
			memset(read_buffer_, 0, max_msg);
			std::string msg(read_buffer_, bytes);
			memcpy(&g_data, read_buffer_, sizeof(g_data));
			// echo message back, and then stop
			
			do_write("Receive OK\n");

		}

		stop();

	}


	void on_write(const error_code & err, size_t bytes) {

		do_read();

	}

	void do_read() {

		async_read(sock_, buffer(read_buffer_),

			MEM_FN2(read_complete, _1, _2), MEM_FN2(on_read, _1, _2));

	}

	void do_write(const std::string & msg) {

		memset(write_buffer_, 0, max_msg);

		std::copy(msg.begin(), msg.end(), write_buffer_);

		sock_.async_write_some(buffer(write_buffer_, msg.size()),

			MEM_FN2(on_write, _1, _2));

	}

	size_t read_complete(const boost::system::error_code & err, size_t bytes) {

		if (err) return 0;
/*
		bool found = std::find(read_buffer_, read_buffer_ + bytes, '\n') < read_buffer_ + bytes;

		// we read one-by-one until we get to enter, no buffering

		return found ? 0 : 1;
*/
		return 1;
	}

private:

	ip::tcp::socket sock_;

	enum { max_msg = 1500 };

	char read_buffer_[max_msg];

	char write_buffer_[max_msg];

	bool started_;

};
