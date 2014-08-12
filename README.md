Fixed size function
===================

Fixed size function wrapper like [std::function](http://en.cppreference.com/w/cpp/utility/functional/function)

Usage:

```c++
#include "fixed_size_function.hpp"

int f(int a)
{
	std::cout << __FUNCTION__ << "\n";
	return a; 
}

int g(int a, int b)
{
	std::cout << __FUNCTION__ << "\n";
	return a;
}

struct X
{
	int operator()(int a) const
	{
		std::cout << __FUNCTION__ << "\n";
		return a;
	}

	int mem_fun(int a)
	{
		std::cout << __FUNCTION__ << "\n";
		return a;
	}
};

int main()
{
	fixed_size_function<int(int), 256> fun;

	// Functor
	X x;
	fun = x;
	fun(1);

	// Free function
	fun = f;
	fun(2);

	// Bind function
	fun = std::bind(g, std::placeholders::_1, 0);
	fun(3);

	// Bind member function
	fun = std::bind(&X::mem_fun, x, std::placeholders::_1);
	fun(4);

	// Lambda
	fun = [](int a) -> int
	{
		std::cout << __FUNCTION__ << "\n";
		return a;
	};
	fun(5);

}
```