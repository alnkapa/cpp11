#include "async_server.h"
#include <chrono>
#include <iostream>
#include <memory>

void async_server::PeerConnectionTCP::start()
{
  m_run1 = std::make_unique<NoBlockHandler>(n, shared_from_this());
  m_run2 = std::make_shared<BlockHandler>(n, m_connection_number, shared_from_this());
  m_run3 = std::make_unique<BlockPlusHandler>(n, shared_from_this());
  m_run3->subscribe(m_run2);
  do_read();
};

async_server::PeerConnectionTCP::~PeerConnectionTCP()
{
  if (m_socket.is_open())
  {
    m_socket.close();
  }
}

async_server::PeerConnectionTCP::PeerConnectionTCP(
    boost::asio::io_context &io_context, tcp::socket &&socket, std::size_t N, std::size_t connection_counter)
    : m_socket(std::move(socket)), m_timer(io_context), n(N), m_connection_number(connection_counter) {};

void async_server::PeerConnectionTCP::set_timeout()
{
  m_timer.expires_after(std::chrono::seconds(120));
  m_timer.async_wait(
      [self = shared_from_this()](const boost::system::error_code &ec)
      {
        if (!ec)
        {
          self->m_socket.cancel();
        }
        else if (ec != boost::system::errc::operation_canceled)
        {
          std::cout << "timer error: " << ec.message() << std::endl;
        }
      });
}

void async_server::PeerConnectionTCP::do_read()
{
  set_timeout();
  boost::asio::async_read_until(
      m_socket, m_buffer, delim,
      [self = shared_from_this()](const boost::system::error_code &ec,
                                  std::size_t n)
      {
        self->m_timer.cancel();
        if (!ec)
        {
          std::istream is(&self->m_buffer);
          std::string line;
          std::getline(is, line);
          if (line.empty())
          {
            return;
          }
          switch (self->m_block)
          {
          case BlockType::Block:
            (*self->m_run2)(std::move(line));
            break;
          case BlockType::BlockPlus:
            (*self->m_run3)(std::move(line));
            break;
          default: // BlockType::NoBlock
            (*self->m_run1)(std::move(line));
            break;
          }
          // цепочка вызовов
          self->do_read();
        }
        else
        {
          if (ec != boost::system::errc::operation_canceled)
          {
            std::cout << "read error: " << ec.message() << std::endl;
          }
        }
      });
};
