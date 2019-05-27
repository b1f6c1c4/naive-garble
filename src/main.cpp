#include <iostream>
#include <tomcrypt.h>
#include "garbled_table.hpp"

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

	std::array<size_t, 2> malice{ 2, 3 };
	std::array<size_t, 1> mbob{ 4 };
	garbled_table<2, 1> tbl(malice, mbob, 7);
	tbl.garble([](const auto &a, const auto &b){
				return a[0] + a[1] + b[0];
			});

	std::cout.write(make_char(get_ptr(tbl.get_table())), get_sz(tbl.get_table()));

	std::array<size_t, 2> my{ 1, 0 };
	for (size_t i = 0; i < my.size(); i++)
	{
		decltype(auto) l = tbl.get_label_alice(i, my[i]);
		std::cout.write(make_char(get_ptr(l)), get_sz(l));
	}

	return 0;
}
