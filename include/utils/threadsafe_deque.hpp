#include <deque>
#include <condition_variable>
#include <mutex>
#include <memory>

template<typename T>
class ThreadsafeDeque {
public:
  ThreadsafeDeque();

  void push_back(T new_value);

  void wait_pop_back(T& value);
  std::shared_ptr<T> wait_pop_back();

  bool try_pop_back(T& value);
  std::shared_ptr<T> try_pop_back();

  bool empty();

private:
  mutable std::mutex mut;
  std::deque<T> data_deque;
  std::condition_variable data_cond;
};


#include "utils/TDequeImpl/threadsafe_deque_impl.hpp"