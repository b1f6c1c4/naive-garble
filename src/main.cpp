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

template <size_t N>
class alice_t
{
public:
	alice_t(size_t a) : _alice(a) { }

	std::string garble()
	{
		prng_state prng;
		RUN(rng_make_prng(128, find_prng("yarrow"), &prng, NULL));

		decltype(auto) out = raw_vector<>(_alice.Garble_size());
		_alice.garble(&*out.begin(), prng);

		std::string res;
		res.resize(get_sz(out) * 2);
		dump_hex(res, out);
		return res;
	}

	std::string receive(std::string &input)
	{
		decltype(auto) in = raw_vector<>(_alice.Inquiry_size());
		load_hex(input, in);

		decltype(auto) out = raw_vector<>(_alice.Receive_size());
		_alice.receive(&*in.begin(), &*out.begin());

		std::string res;
		res.resize(get_sz(out) * 2);
		dump_hex(res, out);
		return res;
	}

private:
	simple_min_alice<N> _alice;
};

#define MAKE_ALICE(N) \
EMSCRIPTEN_BINDINGS(alice##N##_t_b) { \
	class_<alice_t<N>>("Alice" #N) \
		.constructor<size_t>() \
		.function("garble", &alice_t<N>::garble) \
		.function("receive", &alice_t<N>::receive) \
		; \
}

MAKE_ALICE(2);
MAKE_ALICE(4);


template <size_t N>
class bob_t
{
public:
	bob_t(size_t b) : _bob(b) { }

	std::string inquiry(std::string &input)
	{
		prng_state prng;
		RUN(rng_make_prng(128, find_prng("yarrow"), &prng, NULL));

		decltype(auto) in = raw_vector<>(_bob.Garble_size());
		load_hex(input, in);

		decltype(auto) out = raw_vector<>(_bob.Inquiry_size());
		_bob.inquiry(&*in.begin(), prng, &*out.begin());

		std::string res;
		res.resize(get_sz(out) * 2);
		dump_hex(res, out);
		return res;
	}

	size_t evaluate(std::string &input)
	{
		decltype(auto) in = raw_vector<>(_bob.Receive_size());
		load_hex(input, in);

		return _bob.evaluate(&*in.begin());
	}

private:
	simple_min_bob<N> _bob;
};

#define MAKE_BOB(N) \
EMSCRIPTEN_BINDINGS(bob##N##_t_b) { \
	class_<bob_t<N>>("Bob" #N) \
		.constructor<size_t>() \
		.function("inquiry", &bob_t<N>::inquiry) \
		.function("evaluate", &bob_t<N>::evaluate) \
		; \
}

MAKE_BOB(2);
MAKE_BOB(4);
