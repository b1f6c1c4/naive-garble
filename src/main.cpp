#include <iostream>
#include <tomcrypt.h>
#include <functional>
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
	ltc_mp = ltm_desc;

	prng_state prng;
	RUN(rng_make_prng(128, find_prng("yarrow"), &prng, NULL));

	std::array<size_t, 2> malice{ 2, 3 };
	std::array<size_t, 1> mbob{ 4 };
	garbled_table<2, 1> tbl(malice, mbob, 7);
	tbl.garble([](const auto &a, const auto &b){
				return a[0] + a[1] + b[0];
			}, prng);

	/* std::cout.write(make_char(get_ptr(tbl.get_table())), get_sz(tbl.get_table())); */

	std::array<size_t, 2> my{ 1, 0 };
	std::array<size_t, 1> their{ 3 };
	for (size_t i = 0; i < my.size(); i++)
	{
		decltype(auto) l = tbl.get_label_alice(i, my[i]);
		/* std::cout.write(make_char(get_ptr(l)), get_sz(l)); */
	}

	std::array<unsigned char, 16> empty{ 0x00 };
	std::vector<oblivious_transfer_sender<>> ots;
	std::vector<oblivious_transfer_receiver<>> otr;
	for (size_t i = 0; i < mbob.size(); i++)
	{
		auto v = mbob[i];
		auto th = their[i];

		ots.emplace_back(v);
		otr.emplace_back(v);
		decltype(auto) ot = ots.back();
		decltype(auto) ox = otr.back();
		{
			ot.initiate(prng);
			decltype(auto) buff = raw_vector<unsigned char>(ot.dump_size());
			ot.dump(&*buff.begin());
			std::cout.write(make_char(get_ptr(buff)), get_sz(buff));
			ox.initiate(&*buff.begin(), th, prng);
		}
		std::cout.write(make_char(get_ptr(empty)), get_sz(empty));
		{
			decltype(auto) buff = raw_vector<unsigned char>(ox.dump_size());
			ox.dump(&*buff.begin());
			std::cout.write(make_char(get_ptr(buff)), get_sz(buff));
			ot.receive(&*buff.begin(), std::bind(&decltype(tbl)::get_label_bob, &tbl, i, std::placeholders::_1));
		}
		std::cout.write(make_char(get_ptr(empty)), get_sz(empty));
		{
			decltype(auto) buff = raw_vector<unsigned char>(ot.dump_size());
			ot.dump(&*buff.begin());
			std::cout.write(make_char(get_ptr(buff)), get_sz(buff));
			ox.receive(&*buff.begin());
		}
		std::cout.write(make_char(get_ptr(empty)), get_sz(empty));
		{
			decltype(auto) buff = raw_vector<unsigned char>(ox.dump_size());
			ox.dump(&*buff.begin());
			std::cout.write(make_char(get_ptr(buff)), get_sz(buff));
		}
		std::cout.write(make_char(get_ptr(empty)), get_sz(empty));
		std::cout.write(make_char(get_ptr(tbl.get_label_bob(i, th))), get_sz(tbl.get_label_bob(i, th)));

		/* std::cout.write(make_char(get_ptr(buff)), get_sz(buff)); */
		break;
	}

	return 0;
}
