#include <iostream>

#include "fixed_size_function.hpp"

int f(int a)
{
	std::cout << a << ' ' << __FUNCTION__ << '\n';
	return a;
}

int g(int a, int b)
{
	std::cout << a << ' ' << __FUNCTION__ << '\n';
	return a;
}

struct X
{
	X()
	{
		//std::cout << "X::X()\n";
	}

	X(X const&)
	{
		//std::cout << "X::X(X const&)\n";
	}

	X& operator=(X const&)
	{
		//std::cout << "X::operator=(X const&)\n";
		return *this; 
	}

	X(X&&)
	{
		//std::cout << "X::X(X&&)\n";
	}

	X& operator=(X&&)
	{
		//std::cout << "X::operator=(X&&)\n";
		return *this;
	}

	~X()
	{
		//std::cout << "X::~X()\n";
	}

	int operator()(int a) const
	{
		std::cout << a << ' ' << __FUNCTION__ << '\n';
		return a;
	}

	int mem_fun(int a)
	{
		std::cout << a << ' ' << __FUNCTION__ << '\n';
		return a;
	}
};

#define ensure(condition) if (!(condition)) { std::cerr << __FILE__ << ':' << __LINE__ << ' ' << #condition << " failed\n"; }

int main()
{
	using function = fixed_size_function<int(int), 256>;

	X x;

	// Default ctor
	function fun0;
	ensure(!fun0);

	// Template ctors
	function fun(x), fun2(f);
	ensure(fun(1) == 1);
	ensure(fun2(2) == 2);

	// Copy ctor
	function fun3(fun);
	ensure(fun3(3) == 3);

	// Move
	fun2 = std::move(fun);
	ensure(fun2(4) == 4);
	ensure(!fun);

	// Assign function
	fun = fun2;
	ensure(fun);
	ensure(fun2);

	// Swap
	fun.reset();
	fun2.swap(fun);
	ensure(fun && !fun2);

	// Assign free function
	fun = f;
	ensure(fun(5) == 5);

	// Bind free function
	fun = std::bind(g, std::placeholders::_1, 0);
	ensure(fun(6) == 6);

	// Bind member function
	fun = std::bind(&X::mem_fun, x, std::placeholders::_1);
	ensure(fun(7) == 7);

	// Lambda
	fun = [](int a) -> int
	{
		std::cout << a << ' ' << __FUNCTION__ << '\n';
		return a;
	};
	ensure(fun(8) == 8);

	// Reset
	fun = nullptr;
	fun3.reset();
	ensure(!fun && !fun3);

	// std::bad_function_call exception on empty function call
	bool bad_call_catched = false;
	if (fun == nullptr)
	{
		try
		{
			fun(0);
		}
		catch (std::bad_function_call const&)
		{
			bad_call_catched = true;
		}
	}
	ensure(bad_call_catched);
}
