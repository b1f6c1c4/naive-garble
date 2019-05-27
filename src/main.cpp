#include <iostream>
#include <tomcrypt.h>
#include <functional>
#include "garbled_table.hpp"
#include "oblivious_transfer.hpp"

using namespace std::placeholders;
#define BIND(obj, fun, ...) std::bind(&decltype(obj)::fun, &obj, __VA_ARGS__)

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

	std::array<size_t, 2> my{ 0, 2 };
	std::array<size_t, 1> their{ 1 };

	{
		decltype(auto) buff = raw_vector<unsigned char>(tbl.dump_size());
		tbl.dump(&*buff.begin(), [&my](size_t id){return my[id];});
		std::cout << decltype(tbl)::evaluate(2*3*4, 7, get_ptr(buff), [&tbl,&their](size_t id){return tbl.get_label_bob(id, their[id]);});
		/* std::cout.write(make_char(get_ptr(buff)), get_sz(buff)); */
		return 0;
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
			ot.receive(&*buff.begin(), BIND(tbl, get_label_bob, i, _1));
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
