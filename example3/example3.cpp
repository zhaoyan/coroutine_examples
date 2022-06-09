template<typename T>
void println(const char *title, T content)
 {
    static std::mutex mtx;
    const std::lock_guard<std::mutex> lock(mtx);
    std::time_t tm = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()); 
    std::cout<<"["<<tm<<","<<std::this_thread::get_id()<<"]"<<title<<" "<<content<<std::endl;    
}
//==============================================================================
template <typename T, typename... Args>
requires(!std::is_void_v<T> && !std::is_reference_v<T>)
struct std::coroutine_traits<std::future<T>, Args...> 
{
  struct promise_type:std::promise<T> 
  {
    std::suspend_never initial_suspend() const noexcept { println("initial_suspend","");   return {}; }
    std::suspend_never final_suspend() const noexcept {  println("final_suspend","");  return {}; }
    void unhandled_exception() noexcept { this->set_exception(std::current_exception()); }    
    std::future<T> get_return_object() noexcept 
    {
         println("get_return_object","");
        return this->get_future();
    }
    void return_value(const T &value) noexcept(std::is_nothrow_copy_constructible_v<T>) 
    {
      this->set_value(value);
    }
    void return_value(T &&value) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        println("return_value","");  
      this->set_value(std::move(value));
    }
  };
};
using namespace std::chrono_literals;
template <typename T>
auto operator co_await(std::future<T> future) noexcept
requires(!std::is_reference_v<T>) 
{
  struct awaiter : std::future<T> 
  {
    bool await_ready() const noexcept 
    {
         println("await_ready","");  
        return this->wait_for(0s) != std::future_status::timeout;
    }
    void await_suspend(std::coroutine_handle<> cont) const 
    {
        println("await_suspend","");      
      std::thread([this, cont] {
          
        println("wait for worker","");     
        this->wait();
        println("worker OK","");          
        cont.resume();
      }).detach();
    }
    T await_resume()
    { 
        println("await_resume","");             
        return this->get(); 
    }
  };
  return awaiter{std::move(future)};
}
 
std::future<int> co_fun4(float a, float b) 
{
   println("co_fun4","");
   int r=2;
  float f = co_await std::async(std::launch::async,[a](){
           println("async start","");
          std::this_thread::sleep_for(1s); 
           println("end of sleep","");      
          return a*2.0;          
      });
 
  println("end of co_await",f*r);    
  co_return (int)(f*b);
}
int main() 
{
   println("main","");   
   auto res2= co_fun4(2.1,2.0);
   println(" wait for co_fun4 return:","");  
   int i=res2.get();
   println("end of  main",i);     
}
