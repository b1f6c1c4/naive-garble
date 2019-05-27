#include <iostream>
#include <tomcrypt.h>
#include "garbled_table.hpp"
#include "oblivious_transfer.hpp"

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

	prng_state prng1, prng2;
	RUN(rng_make_prng(128, find_prng("yarrow"), &prng1, NULL));
	RUN(rng_make_prng(128, find_prng("sprng"), &prng2, NULL));

	std::array<size_t, 2> malice{ 2, 3 };
	std::array<size_t, 1> mbob{ 4 };
	garbled_table<2, 1> tbl(malice, mbob, 7);
	tbl.garble([](const auto &a, const auto &b){
				return a[0] + a[1] + b[0];
			}, prng1);

	std::cout.write(make_char(get_ptr(tbl.get_table())), get_sz(tbl.get_table()));

	std::array<size_t, 2> my{ 1, 0 };
	for (size_t i = 0; i < my.size(); i++)
	{
		decltype(auto) l = tbl.get_label_alice(i, my[i]);
		std::cout.write(make_char(get_ptr(l)), get_sz(l));
	}

	std::vector<oblivious_transfer_sender<>> ots;
	for (auto v : mbob)
	{
		ots.emplace_back(v);
		decltype(auto) ot = ots.back();
		ot.initiate(prng1, prng2);
		decltype(auto) buff = raw_vector<unsigned char>(ot.dump_size());
		ot.dump(&*buff.begin());
		std::cout.write(make_char(get_ptr(buff)), get_sz(buff));
	}

	return 0;
}
