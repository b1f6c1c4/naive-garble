#include <iostream>
#include <tomcrypt.h>
#include <emscripten.h>
#include <emscripten/bind.h>
#include "simple_min.hpp"

inline char int2char(int input)
{
	if (input < 0 || input >= 16)
		throw std::invalid_argument("Invalid output string");
	if (input >= 10)
		return input - 10 + 'a';
	return input + '0';
}

template <typename Iter, typename Container>
inline void dump_hex(Iter d, const Container &c)
{
	auto ptr = get_ptr(c);
	for (size_t i = 0; i < get_sz(c); i++)
	{
		d[i * 2] = int2char(ptr[i] / 16);
		d[i * 2 + 1] = int2char(ptr[i] % 16);
	}
}

inline unsigned int char2int(char input)
{
	if (input >= '0' && input <= '9')
		return input - '0';
	if (input >= 'A' && input <= 'F')
		return input - 'A' + 10;
	if (input >= 'a' && input <= 'f')
		return input - 'a' + 10;
	throw std::invalid_argument("Invalid input string");
}

template <typename Iter, typename Container>
inline void load_hex(Iter s, Container &c)
{
	auto ptr = get_ptr(c);
	for (size_t i = 0; i < get_sz(c); i++)
		ptr[i] = char2int(s[i * 2]) * 16 + char2int(s[i * 2 + 1]);
}

int main()
{
	register_all_ciphers();
	register_all_hashes();
	register_all_prngs();
	ltc_mp = ltm_desc;
}

using namespace emscripten;

template <size_t M>
constexpr size_t garble_size()
{
	return simple_min_base<M>::Garble_size();
}

template <size_t M>
constexpr size_t inquiry_size()
{
	return simple_min_base<M>::Inquiry_size();
}

template <size_t M>
constexpr size_t receive_size()
{
	return simple_min_base<M>::Receive_size();
}

template <size_t M>
class alice_t
{
public:
	alice_t(size_t a) : _alice(a) { }

	void garble(char *out)
	{
		prng_state prng;
		RUN(rng_make_prng(128, find_prng("yarrow"), &prng, NULL));

		_alice.garble(out, prng);
	}

	void receive(const char *in, char *out)
	{
		_alice.receive(in, out);
	}

private:
	simple_min_alice<M> _alice;
};

template <size_t M>
class bob_t
{
public:
	bob_t(size_t b) : _bob(b) { }

	void inquiry(const char *in, char *out)
	{
		prng_state prng;
		RUN(rng_make_prng(128, find_prng("yarrow"), &prng, NULL));

		_bob.inquiry(in, prng, out);
	}

	size_t evaluate(const char *in)
	{
		return _bob.evaluate(in);
	}

private:
	simple_min_bob<M> _bob;
};

#define MAKE(M) \
EMSCRIPTEN_BINDINGS(alice##M##_t_b) { \
	class_<alice_t<M>>("Alice" #M) \
		.constructor<size_t>() \
		.function("garble", &alice_t<M>::garble, allow_raw_pointers()) \
		.function("receive", &alice_t<M>::receive, allow_raw_pointers()) \
		; \
} \
EMSCRIPTEN_BINDINGS(bob##M##_t_b) { \
	class_<bob_t<M>>("Bob" #M) \
		.constructor<size_t>() \
		.function("inquiry", &bob_t<M>::inquiry, allow_raw_pointers()) \
		.function("evaluate", &bob_t<M>::evaluate, allow_raw_pointers()) \
		; \
} \
EMSCRIPTEN_BINDINGS(base##M##_t_b) { \
	function("garble_size", &garble_size<M>); \
	function("inquiry_size", &inquiry_size<M>); \
	function("receive_size", &receive_size<M>); \
}

MAKE(2);
MAKE(4);
