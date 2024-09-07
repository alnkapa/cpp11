#ifndef ASYNC_SERVER_H
#define ASYNC_SERVER_H

#include "../pub_sub/pub_sub.h"
#include <boost/asio.hpp>
#include <chrono>
#include <memory>
#include <string>
#include <mutex>
#include <vector>
#include <fstream>
#include <iostream>

namespace async_server
{

  inline void print(
      std::vector<std::string> &&store,
      std::size_t suffix,
      std::size_t prefix,
      long long time_stamp)
  {
    if (!store.empty())
    {
      {
        std::stringstream stream;
        stream << "bulk: ";
        auto it = store.begin();
        while (it != store.end())
        {
          stream << *it;
          if (++it != store.end())
          {
            stream << ", ";
          }
        }
        stream << std::endl;
        suffix++;
        std::ofstream file("bulk_" +
                           std::to_string(prefix) +
                           "_" +
                           std::to_string(time_stamp) +
                           "_" +
                           std::to_string(suffix) +
                           ".log");
        if (file.is_open())
        {
          file << stream.str();
        }
        std::cout << stream.str();
      };
    }
  }

  class TimeStamp
  {
  public:
    using value_type = long long;

  private:
    value_type m_time_stamp{0};

  public:
    TimeStamp(value_type in = 0) : m_time_stamp(in) {};
    void reset() { m_time_stamp = 0; };
    void update()
    {
      if (m_time_stamp == 0)
      {
        m_time_stamp = std::chrono::duration_cast<std::chrono::seconds>(
                           std::chrono::system_clock::now().time_since_epoch())
                           .count();
      }
    }
    operator value_type() const { return m_time_stamp; };

    std::string String() const
    {
      return std::to_string(m_time_stamp);
    };
  };

  const char delim{'\n'};
  const std::string OPEN{"{"};
  const std::string CLOSE{"}"};

  enum BlockType
  {
    NoBlock,
    Block,
    BlockPlus
  };

  class PeerConnectionTCP;

  // без блоков
  // самое простое сделать глобальные переменные
  inline std::mutex g_lo;
  inline std::vector<std::string> g_store;
  inline std::size_t g_suffix{0};
  inline TimeStamp g_time_stamp;
  class NoBlockHandler
  {
  public:
    NoBlockHandler(std::size_t N, std::weak_ptr<PeerConnectionTCP>);
    ~NoBlockHandler();
    void operator()(std::string &&);

  private:
    std::size_t n{3};
    std::weak_ptr<PeerConnectionTCP> self;
  };

  using sub_type = pubsub::Publisher<std::vector<std::string> &&>;

  // первый блок
  class BlockHandler : public sub_type::Subscriber
  {
  public:
    BlockHandler(std::size_t N, std::size_t prefix, std::weak_ptr<PeerConnectionTCP>);
    ~BlockHandler();
    void operator()(std::string &&);
    void callback(std::vector<std::string> &&);

  private:
    void clear();
    std::size_t n{3};
    std::size_t m_suffix{0};
    std::size_t m_prefix{0};
    std::vector<std::string> m_store;
    TimeStamp m_time_stamp;
    int m_counter;
    std::weak_ptr<PeerConnectionTCP> self;
  };

  // второй и более блок
  class BlockPlusHandler : public sub_type
  {
  public:
    BlockPlusHandler(std::size_t N, std::weak_ptr<PeerConnectionTCP>);
    ~BlockPlusHandler();
    void operator()(std::string &&);

  private:
    using stack_t = std::pair<std::vector<std::string>, std::size_t>;
    std::size_t n{3};
    std::size_t m_level{0};
    std::vector<std::string> m_store;
    bool m_stop{false};
    std::size_t m_stop_level{0};
    std::size_t m_counter{0};
    std::weak_ptr<PeerConnectionTCP> self;
    std::vector<stack_t> m_stack;
  };

  using namespace boost::asio::ip;
  // use(socket) for read and write
  class PeerConnectionTCP
      : public std::enable_shared_from_this<PeerConnectionTCP>
  {
  private:
    friend class NoBlockHandler;   // assess to m_block
    friend class BlockHandler;     // assess to m_block
    friend class BlockPlusHandler; // assess to m_block
    tcp::socket m_socket;
    boost::asio::streambuf m_buffer{};
    std::size_t n{3};
    std::size_t m_connection_number{0};
    boost::asio::steady_timer m_timer;
    BlockType m_block{BlockType::NoBlock};
    std::unique_ptr<NoBlockHandler> m_run1;
    std::shared_ptr<BlockHandler> m_run2;
    std::unique_ptr<BlockPlusHandler> m_run3;
    // установить таймер на получение команды
    void set_timeout();

    void do_read();

  public:
    PeerConnectionTCP(boost::asio::io_context &io_context, tcp::socket &&socket,
                      std::size_t N, std::size_t connection_number);
    ~PeerConnectionTCP();
    void start();
  };

  class ServerTCP : public std::enable_shared_from_this<ServerTCP>
  {
  public:
    ServerTCP(boost::asio::io_context &io_context, const std::string &ip_address,
              unsigned short port, std::size_t N);
    void start();

  private:
    std::size_t n{3};
    std::size_t m_connection_counter{0};
    void do_accept();
    boost::asio::io_context &m_context;
    tcp::acceptor m_acceptor;
  };
} // namespace async_server
#endif

// TODO:
// listen and accept -> make (socket)
// boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
// signals.async_wait(handler);
// acceptor_.close();
// void handler(
//     const boost::system::error_code& error,
//     int signal_number)
// {
//   if (!error)
//   {
//     // A signal occurred.
//   }
// }
