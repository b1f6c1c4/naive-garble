#include <iostream>
#include <tomcrypt.h>
#include "simple_min.hpp"

template <typename T>
inline auto make_char(T *ptr)
{
	return reinterpret_cast<char *>(ptr);
}

template <typename T>
inline const auto make_char(const T *ptr)
{
	return reinterpret_cast<const char *>(ptr);
}

int main()
{
	register_all_ciphers();
	register_all_hashes();
	register_all_prngs();
	ltc_mp = ltm_desc;

	prng_state prng;
	RUN(rng_make_prng(128, find_prng("yarrow"), &prng, NULL));

	simple_min_alice<4> alice(3);
	simple_min_bob<4> bob(2);

	decltype(auto) buff1 = raw_vector<unsigned char>(alice.garble_size());
	alice.garble(&*buff1.begin(), prng);
	decltype(auto) buff2 = raw_vector<>(bob.inquiry_size());
	bob.inquiry(&*buff1.begin(), prng, &*buff2.begin());
	decltype(auto) buff3 = raw_vector<>(alice.receive_size());
	alice.receive(&*buff2.begin(), &*buff3.begin());
	std::cout << bob.evaluate(&*buff3.begin());

	return 0;
}
