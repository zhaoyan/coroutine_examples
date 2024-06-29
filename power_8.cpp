#include <iostream>
#include <algorithm>
#include <vector>
#include <ranges>
#include <iterator>
#include <set>
#include <typeinfo>
#include <chrono>
#include <thread>
#include <chrono>
#include<coroutine>


template <typename T>
void printRatio(){ 
	std::cout << "  precision: " << T::num << "/" << T::den << " second " << std::endl;
	typedef typename std::ratio_multiply<T,std::kilo>::type MillSec;
	typedef typename std::ratio_multiply<T,std::mega>::type MicroSec;
	std::cout << static_cast<double>(MillSec::num)/MillSec::den << " milliseconds " << std::endl;
	std::cout << static_cast<double>(MicroSec::num)/MicroSec::den << " microseconds " << std::endl;
}

using namespace std::chrono_literals;


struct ReturnType { //This is user-defined return type
	struct promise_type { 	// user-defined coroutine level promise type
		using Handle = std::coroutine_handle<promise_type>;
		ReturnType get_return_object() {
			return ReturnType{Handle::from_promise(*this)};
		}
		std::suspend_never initial_suspend() { return {}; }
		std::suspend_never final_suspend() noexcept { return {}; }
		void return_void() { }
		void unhandled_exception() { }
	};
	explicit ReturnType(promise_type::Handle coro) : coro_(coro) {}
	void destroy() { coro_.destroy(); }
	void resume() { coro_.resume(); }
	// Making Task move-only:
	ReturnType(const ReturnType&) = delete;
	ReturnType& operator=(const ReturnType&) = delete;
	ReturnType(ReturnType&& t) noexcept : coro_(t.coro_) { t.coro_ = {}; }
	ReturnType& operator=(ReturnType&& t) noexcept {
		if (this == &t) return *this;
		if (coro_) coro_.destroy();
		coro_ = t.coro_;
		t.coro_ = {};
		return *this;
	}
private:
	promise_type::Handle coro_;
};

int slow_sqr(const int value)
{
    std::cout<<"thread ID in slow_sqr"<<std::this_thread::get_id()<<std::endl;
    std::this_thread::sleep_for(2s); // 线程很累了，它想睡一觉再算
    std::cout<<" Yan "<<value*value<<std::endl;
    return value * value;
}

template <std::invocable<int> F>
void async_sqr_cbk(const int value, F&& continuation)
{
    std::thread thr([v = value, c = std::forward<F>(continuation)]() mutable
    {
        int i = slow_sqr(v);
        c(i);
    });
    thr.detach();
}

class sqr_awaitable final
{
private:
    int value_ = 0;
    int result_ = 0;

public:
    
    explicit sqr_awaitable(const int value): value_(value) {}

    // 直接返回 false，因为我们总是希望协程直接挂起的
    bool await_ready() const noexcept { return false; }

    // 挂起协程之后就可以开始异步运算了
    void await_suspend(const std::coroutine_handle<> awaiting)
    {
        // 调用回调式异步函数
        async_sqr_cbk(value_, [this, awaiting](const int result)
        {
            result_ = result; // 运算结束以后直接把结果存在等待器里面
            awaiting(); // 然后恢复协程，使用成员函数 awaiting.resume() 也可以
        });
    }

    // 协程恢复执行的时候把结果返回给协程就可以了
    int await_resume() const noexcept { return result_; }
};

// 这个函数实际上只是创建了一个等待器并且直接返回，主要的功能实现是在等待器类里面的
auto async_sqr_coro(const int value) { return sqr_awaitable(value); }


ReturnType power_8_async(const int value)
{
std::cout<<"thread ID in power1 "<<std::this_thread::get_id()<<std::endl;
    const int squared = co_await sqr_awaitable{value};
    std::cout<<"thread ID in power2 "<<std::this_thread::get_id()<<std::endl;
    const int fourth = co_await sqr_awaitable{squared};
    std::cout<<"thread ID in powe3 "<<std::this_thread::get_id()<<std::endl;
    const int result = co_await sqr_awaitable{fourth};
    std::cout << "result = " << result<<std::endl;
}

using namespace std;
int main() {
std::cout<<"thread ID in main"<<std::this_thread::get_id()<<std::endl;
    power_8_async(3);

    
    for(int i = 0;i<8;i++)
    {
        std::this_thread::sleep_for(1s);    
        cout<<"in main thread"<<endl;
    }

    return 0;

    std::vector<int> data{1,2,3,4,5,6,7,8,9};
    for (auto window : data | std::views::slide(4)) {
        // Iterate over all 4 element window positions:
        // {1,2,3,4}
        // {2,3,4,5}
        // {3,4,5,6}
        // {4,5,6,7}
        // {5,6,7,8}
        // {6,7,8,9}
        std::copy(window.begin(), window.end(), std::ostream_iterator<int>(std::cout, " "));
        std::cout << "\n";
    }

    vector v {std::pair{1,2}, {3,4}, {5,6}};

    cout<<v[0].second<<endl;
    cout<<ranges::any_of(v, [](auto e){return e==2;}, &pair<int,int>::second)<<endl;

    vector v1{1, 2};
    cout<<v1[1]<<endl;

    
    set s{1, 2, 3, 4};
    vector v2(begin(s),end(s));

    auto i = begin(s);
    advance(i, 2);
    cout<<*i<<endl;
    reverse_iterator ri {i};
    cout<<*ri<<endl;   
    
    //cout<<v2[3]<<endl;
    cout<<"----------------------"<<endl;
    printRatio<std::chrono::system_clock::period>(); 
    //linux accuracy is 1 nano second. 	

}