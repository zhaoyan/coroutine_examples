#include<iostream>
#include<coroutine>
using namespace std;

struct future_type_int{
    struct promise_type{
    int ret_val;
    using co_handle_type = std::coroutine_handle<promise_type>;
    promise_type(){
        std::cout<<"promise_type constructor"<<std::endl;
    }
    ~promise_type(){
        std::cout<<"promise_type destructor"<<std::endl;
    }
    auto get_return_object(){
    	std::cout<<"get_return_object"<<std::endl;
        return co_handle_type::from_promise(*this);
    }
    auto initial_suspend(){
    	std::cout<<"initial_suspend"<<std::endl;
        return std::suspend_always();
    }
    auto final_suspend() noexcept(true) {
    	std::cout<<"final_suspend"<<std::endl;
        return std::suspend_never();
    }
    void return_value(int val){
    	std::cout<<"return_value : "<<val<<std::endl;
        ret_val = val;
	}
    void unhandled_exception(){
    	std::cout<<"unhandled_exception"<<std::endl;
        std::terminate();
    }
    auto yield_value(int val){
        std::cout<<"yield_value : "<<val<<std::endl;
        ret_val = val;
        return std::suspend_always();
    }
};
    using co_handle_type = std::coroutine_handle<promise_type>;
    future_type_int(co_handle_type co_handle){
        std::cout<<"future_type_int constructor"<<std::endl;
        co_handle_ = co_handle;
    }
    ~future_type_int(){
        std::cout<<"future_type_int destructor"<<std::endl;
    }
    future_type_int(const future_type_int&) = delete;
    future_type_int(future_type_int&&) = delete;

    bool resume(){
        if(!co_handle_.done()){
            co_handle_.resume();
        }
        return !co_handle_.done();
    }
    bool await_ready() { 
        return false; 
    }
    bool await_suspend(std::coroutine_handle<> handle) {
        resume();
        return false;
    }
    auto await_resume() {
        return co_handle_.promise().ret_val;
    }

    co_handle_type co_handle_;
};

future_type_int three_step_coroutine(){
    std::cout<<"three_step_coroutine begin"<<std::endl;
    co_yield 222;
    std::cout<<"three_step_coroutine running"<<std::endl;
    co_yield 333;
    std::cout<<"three_step_coroutine end"<<std::endl;
    co_return 444;
}

struct future_type_void{
	struct promise_type;
	using co_handle_type = std::coroutine_handle<promise_type>;
	struct promise_type{
	    promise_type(){
	        std::cout<<"promise_type constructor void"<<std::endl;
	    }
	    ~promise_type(){
	        std::cout<<"promise_type destructor void"<<std::endl;
	    }
	    auto get_return_object(){
	    	std::cout<<"get_return_object void"<<std::endl;
	        return co_handle_type::from_promise(*this);
	    }
	    auto initial_suspend(){
	    	std::cout<<"initial_suspend void"<<std::endl;
	        return std::suspend_always();
	    }
	    auto final_suspend() noexcept(true) {
	    	std::cout<<"final_suspend void"<<std::endl;
	        return std::suspend_never();
	    }
	    void unhandled_exception(){
	    	std::cout<<"unhandled_exception void"<<std::endl;
	        std::terminate();
	    }
	    void return_void(){
	    	std::cout<<"return_void void: "<<std::endl;
		}
	};

    future_type_void(co_handle_type co_handle){
        std::cout<<"future_type_void constructor"<<std::endl;
        co_handle_ = co_handle;
    }
    ~future_type_void(){
        std::cout<<"future_type_void destructor"<<std::endl;
        co_handle_.destroy();
    }
    future_type_void(const future_type_void&) = delete;
    future_type_void(future_type_void&&) = delete;

    bool resume(){
        if(!co_handle_.done()){
            co_handle_.resume();
        }
        return !co_handle_.done();
    }

    co_handle_type co_handle_;
};
future_type_void call_coroutine(){
    auto future = three_step_coroutine();
    std::cout<<"++++++++call three_step_coroutine first++++++++"<<std::endl;
    auto val = co_await future;
    std::cout<<"++++++++call three_step_coroutine second++++++++, val: "<<val<<std::endl;
    val = co_await future; 
    std::cout<<"++++++++call three_step_coroutine third++++++++, val: "<<val<<std::endl;
    val = co_await future; 
    std::cout<<"++++++++call three_step_coroutine end++++++++, val: "<<val<<std::endl;
    co_return;
}
int main(){
    auto ret = call_coroutine();
    std::cout<<"++++++++begine call_coroutine resume in main++++++++"<<std::endl;
    ret.resume();
    std::cout<<"++++++++end call_coroutine resume in main++++++++"<<std::endl;
    return 0;
}

