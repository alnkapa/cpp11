#include "../pub_sub/pub_sub.h"
#include "async_server.h"

async_server::BlockHandler::BlockHandler(std::size_t N,
                                         std::size_t prefix,
                                         std::weak_ptr<PeerConnectionTCP> ptr)
    : n(N), m_prefix(prefix), self(std::move(ptr))
{
}
async_server::BlockHandler::~BlockHandler() {};
void async_server::BlockHandler::operator()(std::string &&line)
{
  if (auto ptr = self.lock())
  {
    if (line == OPEN)
    {
      ptr->m_block = BlockType::BlockPlus;
    }
    else if (line == CLOSE)
    {
      if (m_counter <= n)
      {
        print(
            std::move(m_store),
            m_suffix,
            m_prefix,
            m_time_stamp);
        ++m_suffix;
      }
      clear();
      ptr->m_block = BlockType::NoBlock;
    }
    else
    {
      m_time_stamp.update();
      m_store.emplace_back(std::move(line));
      m_counter++;
    }
  }
}

void async_server::BlockHandler::callback(std::vector<std::string> &&in)
{
  // m_store =  m_store + in;
  std::vector<std::string> n_store(m_store.size() + in.size());
  // TODO: !!! use constructor with pos !!!
  // n_store{pos=m_store.size(), std::move(in)}
  std::copy(m_store.begin(), m_store.end(), n_store.begin());
  std::copy(in.begin(), in.end(), n_store.begin() + m_store.size());
  m_store = std::move(n_store);
}

void async_server::BlockHandler::clear()
{
  m_store.clear();
  m_time_stamp.reset();
  m_counter = 0;
}
