#ifndef FIXED_SIZE_FUNCTION_HPP_INCLUDED
#define FIXED_SIZE_FUNCTION_HPP_INCLUDED

#include <stdexcept>
#include <functional>
#include <tuple>
#include <type_traits>

template<typename Signature, size_t MaxSize>
class fixed_size_function;

template<typename Ret, typename ...Args, size_t MaxSize>
class fixed_size_function<Ret (Args...), MaxSize>
{
public:
	using result_type = Ret;

	static const std::size_t arity = sizeof...(Args);

	template <std::size_t N>
	struct argument
	{
		static_assert(N < arity, "invalid argument index");
		using type = typename std::tuple_element<N, std::tuple<Args...>>::type;
	};

	fixed_size_function()
		: call_(nullptr)
		, destroy_(nullptr)
		, copy_(nullptr)
		, move_(nullptr)
	{
	}

	fixed_size_function(std::nullptr_t)
		: fixed_size_function()
	{
	}

	~fixed_size_function()
	{
		reset();
	}

	fixed_size_function& operator=(std::nullptr_t)
	{
		reset();
		return *this;
	}

	fixed_size_function(fixed_size_function const& src)
	{
		copy(src);
	}

	fixed_size_function& operator=(fixed_size_function const& src)
	{
		assign(src);
		return *this;
	}

	fixed_size_function(fixed_size_function& src)
	{
		copy(src);
	}
	
	fixed_size_function& operator=(fixed_size_function& src)
	{
		assign(src);
		return *this;
	}

	fixed_size_function(fixed_size_function&& src)
	{
		move(std::forward<fixed_size_function>(src));
	}

	fixed_size_function& operator=(fixed_size_function&& src)
	{
		assign(std::forward<fixed_size_function>(src));
		return *this;
	}

	template<typename Functor>
	fixed_size_function(Functor&& f)
	{
		create(std::forward<Functor>(f));
	}

	template<typename Functor>
	fixed_size_function& operator=(Functor&& f)
	{
		assign(std::forward<Functor>(f));
		return *this;
	}

	void assign(fixed_size_function const& src)
	{
		reset();
		copy(src);
	}

	void assign(fixed_size_function& src)
	{
		reset();
		copy(src);
	}

	void assign(fixed_size_function&& src)
	{
		reset();
		move(std::forward<fixed_size_function>(src));
	}

	template<typename Functor>
	void assign(Functor&& f)
	{
		reset();
		create(std::forward<Functor>(f));
	}

	void reset()
	{
		if (destroy_)
		{
			destroy_(&storage_);
			call_ = nullptr;
			copy_ = nullptr;
			move_ = nullptr;
			destroy_ = nullptr;
		}
	}

	explicit operator bool() const { return call_ != nullptr; }

	Ret operator()(Args&& ... args)
	{
		return call_ ? call_(&storage_, std::forward<Args>(args)...) : throw std::bad_function_call();
	}

	void swap(fixed_size_function& other)
	{
		using std::swap;

		swap(call_, other.call_);
		swap(copy_, other.copy_);
		swap(move_, other.move_);
		swap(destroy_, other.destroy_);
		swap(storage_, other.storage_);
	}

	friend void swap(fixed_size_function& lhs, fixed_size_function& rhs)
	{
		lhs.swap(rhs);
	}

	friend bool operator==(std::nullptr_t, fixed_size_function const& f)
	{
		return !f;
	}

	friend bool operator==(fixed_size_function const& f, std::nullptr_t)
	{
		return !f;
	}

	friend bool operator!=(std::nullptr_t, fixed_size_function const& f)
	{
		return f;
	}

	friend bool operator!=(fixed_size_function const& f, std::nullptr_t)
	{
		return f;
	}

private:
	template<typename Functor>
	void create(Functor&& f)
	{
		typedef typename std::decay<Functor>::type functor_type;
		static_assert(sizeof(functor_type) <= MaxSize, "Functor must be smaller than storage buffer");

		new (&storage_) functor_type(std::forward<Functor>(f));

		call_ = &call_impl<functor_type>;
		destroy_ = &destroy_impl<functor_type>;
		copy_ = &copy_impl<functor_type>;
		move_ = &move_impl<functor_type>;
	}

	void copy(fixed_size_function const& src)
	{
		if (src.copy_)
		{
			src.copy_(&src.storage_, &storage_);

			call_ = src.call_;
			copy_ = src.copy_;
			move_ = src.move_;
			destroy_ = src.destroy_;
		}
	}

	void move(fixed_size_function&& src)
	{
		if (src.move_)
		{
			src.move_(&src.storage_, &storage_);

			call_ = src.call_; src.call_ = nullptr;
			copy_ = src.copy_; src.copy_ = nullptr;
			move_ = src.move_; src.move_ = nullptr;
			destroy_ = src.destroy_; src.destroy_ = nullptr;
		}
	}

private:
	template<typename Functor>
	static Ret call_impl(void* functor, Args&& ... args)
	{
		return (*static_cast<Functor*>(functor))(std::forward<Args>(args)...);
	}

	template<typename Functor>
	static void destroy_impl(void* functor)
	{
		static_cast<Functor*>(functor)->~Functor();
	}

	template<typename Functor>
	static void copy_impl(void const* functor, void* dest)
	{
		new (dest) Functor(*static_cast<Functor const*>(functor));
	}

	template<typename Functor>
	static void move_impl(void* functor, void* dest)
	{
		new (dest) Functor(std::move(*static_cast<Functor*>(functor)));
	}

	Ret  (*call_)(void* functor, Args&& ... args);
	void (*destroy_)(void* functor);
	void (*copy_)(void const* functor, void* dest);
	void (*move_)(void* functor, void* dest);

	typename std::aligned_storage<MaxSize>::type storage_;
};

#endif // FIXED_SIZE_FUNCTION_HPP_INCLUDED
