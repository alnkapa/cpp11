#include "async_server/async_server.h"
#include <boost/asio.hpp>
#include <iostream>
#include <string>

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    std::cerr << "Usage: " << argv[0] << " [<host>:]<port>" << "<bulk_size>" << std::endl;
    return 1;
  }
  try
  {
    unsigned short port = 1234;
    std::string ip_address = "0.0.0.0";
    std::size_t N{static_cast<std::size_t>(std::stoi(argv[2]))};

    std::string input{argv[1]};
    size_t colon_pos = input.find(':');
    if (colon_pos != std::string::npos)
    {
      ip_address = input.substr(0, colon_pos);
      port = static_cast<unsigned short>(std::stoi(input.substr(colon_pos + 1)));
    }
    else
    {
      port = static_cast<unsigned short>(std::stoi(input));
    }
    boost::asio::io_context io_context{};
    std::make_shared<async_server::ServerTCP>(io_context, ip_address, port, N)->start();
    io_context.run();
  }
  catch (const std::exception &e)
  {
    std::cerr << "error: " << e.what() << std::endl;
  };

  return 0;
}
