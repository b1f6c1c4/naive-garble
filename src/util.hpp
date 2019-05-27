#include <tomcrypt.h>
#include <utility>
#include <type_traits>
#include <tuple>
#include <vector>

template <typename ... Iters>
class zipped_iter
{
public:
	zipped_iter(std::tuple<Iters ...> its) : _it(its) { }

	decltype(auto) operator++()
	{
		std::apply([](auto & ... x){std::make_tuple(++x ...);}, _it);
		return *this;
	}
	decltype(auto) operator++(int)
	{
		return zipped_iter(std::apply([](auto & ... x){return std::make_tuple(x++ ...);}, _it));
	}
	decltype(auto) operator*()
	{
		return std::apply([](auto & ... x){return std::tie(*x ...);}, _it);
	}
	auto operator==(const zipped_iter<Iters ...> &other)
	{
		return std::get<0>(_it) == std::get<0>(other._it);
	}
	auto operator!=(const zipped_iter<Iters ...> &other)
	{
		return !(operator==(other));
	}

private:
	std::tuple<Iters ...> _it;
};

template <typename Container>
struct iterator_helper
{
	typedef typename Container::iterator iterator;
};

template <typename Container>
struct iterator_helper<const Container>
{
	typedef typename Container::const_iterator iterator;
};

template <typename ... Containers>
class zipped
{
public:
	zipped(Containers & ... cs) : _cs(cs ...) { }

	decltype(auto) begin()
	{
		return zipped_iter<typename iterator_helper<Containers>::iterator ...>(
				std::apply([](auto & ... x){return std::make_tuple(x.begin() ...);}, _cs));
	}

	decltype(auto) end()
	{
		return zipped_iter<typename iterator_helper<Containers>::iterator ...>(
				std::apply([](auto & ... x){return std::make_tuple(x.end() ...);}, _cs));
	}

private:
	std::tuple<Containers & ...> _cs;
};

template <typename ... Containers>
auto zip(Containers & ... cs)
{
	return zipped<Containers ...>(cs ...);
}

template <typename T>
inline auto get_ptr(T *ptr)
{
	return reinterpret_cast<unsigned char *>(ptr);
}

template <typename T>
inline const auto get_ptr(const T *ptr)
{
	return reinterpret_cast<const unsigned char *>(ptr);
}

template <typename Container>
inline auto get_ptr(Container &c)
{
	return get_ptr(&*c.begin());
}

template <typename Container>
inline const auto get_ptr(const Container &c)
{
	return get_ptr(&*c.begin());
}

template <typename Container>
inline auto get_sz(const Container &c)
{
	return sizeof(typename Container::value_type) * c.size();
}

template <typename T>
struct raw_t final
{
    T value;

    raw_t() { } // value is purposefully uninitialized here
	raw_t(const raw_t &v) { value = v; }

	~raw_t() noexcept { }

	raw_t &operator=(const raw_t &v) noexcept { value = v; return *this; }
	raw_t &operator=(const T &v) { value = v; return *this; }
	raw_t &operator=(T &&v) noexcept { value = std::move(v); return *this; }

	operator T() const { return value; }
};

typedef raw_t<unsigned char> byte_t;

template <typename T>
inline auto raw_vector(size_t n)
{
	std::vector<raw_t<T>> c;
	c.resize(n);
	return c;
}

template <typename Container>
inline void random_fill(Container &c, prng_state &prng)
{
	yarrow_read(get_ptr(c), get_sz(c), &prng);
}

template <typename T>
inline auto random_vector(size_t n, prng_state &prng)
{
	decltype(auto) c = raw_vector<T>(n);
	random_fill(c, prng);
	return c;
}

template <typename T>
inline decltype(auto) make_const(const T &obj)
{
	return obj;
}
