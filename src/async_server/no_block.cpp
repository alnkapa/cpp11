#include "async_server.h"

async_server::NoBlockHandler::NoBlockHandler(
    std::size_t N,
    std::weak_ptr<PeerConnectionTCP> ptr)
    : n(N), self(std::move(ptr))
{
}
async_server::NoBlockHandler::~NoBlockHandler()
{
  std::lock_guard lo(g_lo);
  print(
      std::move(g_store),
      g_suffix,
      0,
      g_time_stamp);
};

void async_server::NoBlockHandler::operator()(std::string &&line)
{
  if (auto ptr = self.lock())
  {
    if (line == OPEN)
    {
      std::lock_guard lo(g_lo);
      ptr->m_block = BlockType::Block;
      print(
          std::move(g_store),
          g_suffix,
          0,
          g_time_stamp);
      ++g_suffix;
    }
    else if (line != CLOSE)
    {
      g_time_stamp.update();
      std::lock_guard lo(g_lo);
      g_store.emplace_back(std::move(line));
      if (g_store.size() >= n)
      {
        print(
            std::move(g_store),
            g_suffix,
            0,
            g_time_stamp);
        ++g_suffix;
        g_store.clear();
        g_time_stamp.reset();
      }
    }
  }
}
