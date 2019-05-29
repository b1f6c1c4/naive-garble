#include <iostream>
#include <tomcrypt.h>
#include "simple_min.hpp"

inline char int2char(int input)
{
	if (input < 0 || input >= 16)
		throw std::invalid_argument("Invalid output string");
	if (input >= 10)
		return input - 10 + 'a';
	return input + '0';
}

template <typename Container>
inline void dump_hex(std::ostream &os, const Container &c)
{
	auto ptr = get_ptr(c);
	for (size_t i = 0; i < get_sz(c); i++)
	{
		os << int2char(ptr[i] / 16);
		os << int2char(ptr[i] % 16);
	}
	os << std::endl;
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

template <typename Container>
inline void load_hex(std::istream &is, Container &c)
{
	std::string s;
	is >> s;
	if (get_sz(s) != 2 * get_sz(c))
	{
		std::cerr << "get_sz(c) = " << get_sz(c) << std::endl;
		std::cerr << "get_sz(s) = " << get_sz(s) << std::endl;
		throw std::invalid_argument("Input length not match");
	}

	auto ptr = get_ptr(c);
	for (size_t i = 0; i < get_sz(c); i++)
		ptr[i] = char2int(s[i * 2]) * 16 + char2int(s[i * 2 + 1]);
}

int main(int argc, char *argv[])
{
	register_all_ciphers();
	register_all_hashes();
	register_all_prngs();
	ltc_mp = ltm_desc;

	if (argc != 3)
	{
		std::cout << "Usage: garble [--alice|--bob] <N>" << std::endl;
		return 1;
	}

	std::string type(argv[1]);
	if (type == "--alice")
	{
		prng_state prng;
		RUN(rng_make_prng(128, find_prng("yarrow"), &prng, NULL));
		simple_min_alice<4> alice(std::atoi(argv[2]));

		{
			decltype(auto) out = raw_vector<>(alice.Garble_size());
			alice.garble(&*out.begin(), prng);
			dump_hex(std::cout, out);
		}
		{
			decltype(auto) in = raw_vector<>(alice.Inquiry_size());
			decltype(auto) out = raw_vector<>(alice.Receive_size());
			load_hex(std::cin, in);
			alice.receive(&*in.begin(), &*out.begin());
			dump_hex(std::cout, out);
		}
		return 0;
	}
	if (type == "--bob")
	{
		prng_state prng;
		RUN(rng_make_prng(128, find_prng("yarrow"), &prng, NULL));
		simple_min_bob<4> bob(std::atoi(argv[2]));

		{
			decltype(auto) in = raw_vector<>(bob.Garble_size());
			decltype(auto) out = raw_vector<>(bob.Inquiry_size());
			load_hex(std::cin, in);
			bob.inquiry(&*in.begin(), prng, &*out.begin());
			dump_hex(std::cout, out);
		}
		{
			decltype(auto) in = raw_vector<>(bob.Receive_size());
			load_hex(std::cin, in);
			std::cout << bob.evaluate(&*in.begin());
			std::cout << std::endl;
		}
		return 0;
	}

	std::cout << "Usage: garble [--alice|--bob] <N>" << std::endl;
	return 1;
}
