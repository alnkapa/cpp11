#ifndef PUB_SUB_H
#define PUB_SUB_H
#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

namespace pubsub
{

  template <typename... T>
  class Publisher
  {
  public:
    struct Subscriber
    {
      virtual void callback(T...) = 0;
      virtual ~Subscriber() = default;
    };

    class Function : public Subscriber
    {
    private:
      std::function<void(T...)> m_fu;

    public:
      Function(std::function<void(T...)> fu) : m_fu(fu) {};
      void callback(T... args) override { m_fu(std::forward<T>(args)...); };
    };

    virtual ~Publisher() = default;
    virtual void subscribe(std::weak_ptr<Subscriber> in)
    {
      subscribers.push_back(in);
    }
    virtual void unsubscribe(std::weak_ptr<Subscriber> in)
    {
      auto in_locked = in.lock();
      subscribers.erase(std::remove_if(subscribers.begin(), subscribers.end(),
                                       [&in_locked](std::weak_ptr<Subscriber> v)
                                       {
                                         return v.expired() ||
                                                in_locked == v.lock();
                                       }),
                        subscribers.end());
    }
    virtual void notify(T... args)
    {
      for (auto it = subscribers.begin(); it != subscribers.end();)
      {
        if (auto l = it->lock())
        {
          (l->callback)(std::forward<T>(args)...);
          ++it;
        }
        else
        {
          it = subscribers.erase(it);
        }
      }
    }

  private:
    std::vector<std::weak_ptr<Subscriber>> subscribers;
  };

  template <typename... T>
  class SingletonPublisher final : public Publisher<T...>
  {
  public:
    SingletonPublisher(const SingletonPublisher &) = delete;
    SingletonPublisher &operator=(const SingletonPublisher &) = delete;
    static SingletonPublisher &getInstance()
    {
      static SingletonPublisher instance;
      return instance;
    }

  private:
    SingletonPublisher() {};
    ~SingletonPublisher() {};
  };

  using block_type = SingletonPublisher<std::vector<std::string> &&, long long>;
  using no_block_type = SingletonPublisher<std::string &&>;

} // namespace pubsub
#endif
