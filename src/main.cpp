#include <iostream>
#include <tomcrypt.h>
#include "wrapper.hpp"

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

	std::array<size_t, 2> malice{ 2, 3 };
	std::array<size_t, 1> mbob{ 4 };

	wrapper_alice<2, 1> alice(malice, mbob, 7);
	wrapper_bob<2, 1> bob(malice, mbob, 7);

	std::array<size_t, 2> valice{ 0, 2 };
	std::array<size_t, 1> vbob{ 1 };

	{
		decltype(auto) buff1 = raw_vector<unsigned char>(alice.garble_size());
		alice.garble(&*buff1.begin(), prng, [](const auto &a, const auto &b){
				return a[0] + a[1] + b[0];
				});
		decltype(auto) buff2 = raw_vector<unsigned char>(bob.inquiry_size());
		bob.inquiry(&*buff1.begin(), prng, &*buff2.begin(), [&vbob](size_t id){
				return vbob[id];
				});
		decltype(auto) buff3 = raw_vector<unsigned char>(alice.receive_size());
		alice.receive(&*buff2.begin(), &*buff3.begin(), [&valice](size_t id){
				return valice[id];
				});
		std::cout << bob.evaluate(&*buff3.begin());
	}

	return 0;
}
