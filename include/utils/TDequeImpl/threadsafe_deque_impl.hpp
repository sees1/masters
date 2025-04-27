template <typename T>
ThreadsafeDeque<T>::ThreadsafeDeque()
{ }

template <typename T>
void ThreadsafeDeque<T>::push_back(T new_value)
{
  std::lock_guard<std::mutex> lk(mut);

  data_deque.push_back(std::move(new_value));
  data_cond.notify_one();
}

template <typename T>
void ThreadsafeDeque<T>::wait_pop_back(T& value)
{
  std::unique_lock<std::mutex> lk(mut);
  data_cond.wait(lk,
    [this]
    {
      return !data_deque.empty();
    }
  );

  value = std::move(data_deque.back());
  data_deque.pop_back();
}

template <typename T>
std::shared_ptr<T> ThreadsafeDeque<T>::wait_pop_back()
{
  std::unique_lock<std::mutex> lk(mut);
  data_cond.wait(lk,
    [this]
    {
      return !data_deque.empty();
    }
  );
  
  std::shared_ptr<T> res = std::make_shared<T>(std::move(data_deque.back()));
  data_deque.pop_back();

  return res;
}

template <typename T>
bool ThreadsafeDeque<T>::try_pop_back(T& value)
{
  std::lock_guard<std::mutex> lk(mut);
  
  if(data_deque.empty())
    return false;
  
  value = std::move(data_deque.back());
  data_deque.pop_back();
  
  return true;
}

template <typename T>
std::shared_ptr<T> ThreadsafeDeque<T>::try_pop_back()
{
  std::lock_guard<std::mutex> lk(mut);

  if(data_deque.empty())
    return std::shared_ptr<T>();
 
  std::shared_ptr<T> res = std::make_shared<T>(std::move(data_deque.back()));
  data_deque.pop_back();
  
  return res;
}

template <typename T>
bool ThreadsafeDeque<T>::empty() {
  std::lock_guard<std::mutex> lk(mut);
  return data_deque.empty();
}