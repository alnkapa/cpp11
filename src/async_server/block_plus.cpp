#include "async_server.h"
#include <algorithm>

async_server::BlockPlusHandler::BlockPlusHandler(std::size_t N, std::weak_ptr<PeerConnectionTCP> ptr)
    : n(N), self(std::move(ptr)) {}

async_server::BlockPlusHandler::~BlockPlusHandler() {}

void async_server::BlockPlusHandler::operator()(std::string &&line)
{
  if (auto ptr = self.lock())
  {
    if (line == OPEN)
    {
      ++m_level;
      if (!m_stop)
      {
        m_stack.emplace_back(std::move(m_store), m_counter);
        m_counter = 0;
      }
    }
    else if (line == CLOSE)
    {
      if (m_stop && m_stop_level == m_level)
      {
        m_stop = false;
        m_stop_level = 0;
      }

      if (m_level > 0)
      {
        --m_level;

        if (!m_stop)
        {
          if (!m_stack.empty())
          {
            auto pred = std::move(m_stack.back());
            m_stack.pop_back();
            // m_store = pred.first + m_store;
            std::vector<std::string> n_store(m_store.size() +
                                             pred.first.size());
            std::copy(pred.first.begin(), pred.first.end(), n_store.begin());
            std::copy(m_store.begin(), m_store.end(),
                      n_store.begin() + pred.first.size());
            m_store = std::move(n_store);
            m_counter = pred.second;
          }
        }
      }
      else
      {
        ptr->m_block = BlockType::Block;

        if (!m_stop)
        {
          notify(std::move(m_store));
          m_store.clear();
          m_counter = 0;
        }
      }
    }
    else if (!m_stop)
    {
      m_store.emplace_back(std::move(line));
      ++m_counter;
      if (m_stop = m_counter > n)
      {
        m_stop_level = m_level;
        m_store.clear();
        m_counter = 0;
      }
    }
  }
}
