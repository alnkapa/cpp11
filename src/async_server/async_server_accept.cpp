#include "async_server.h"
#include <memory>

void async_server::ServerTCP::start() { do_accept(); }

void async_server::ServerTCP::do_accept()
{
  m_acceptor.async_accept(
      [self = shared_from_this()](boost::system::error_code ec,
                                  tcp::socket socket)
      {
        if (!ec)
        {
          std::cout << "accept " << std::endl;

          std::make_shared<PeerConnectionTCP>(self->m_context,
                                              std::move(socket), self->n, self->m_connection_counter)
              ->start();
          ++self->m_connection_counter;
        }
        else
        {
          std::cout << "server error:" << ec.message() << std::endl;
        }
        // цепочка вызовов
        self->do_accept();
      });
}

async_server::ServerTCP::ServerTCP(boost::asio::io_context &io_context,
                                   const std::string &ip_address,
                                   unsigned short port, std::size_t N)
    : m_context(io_context),
      m_acceptor(
          io_context,
          tcp::endpoint(boost::asio::ip::make_address(ip_address), port)),
      n(N) {}
