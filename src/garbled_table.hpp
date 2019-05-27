#include <tomcrypt.h>
#include <cstddef>

inline constexpr auto aes_cipher_size(size_t input)
{
	constexpr size_t block_size = 128 / 8;
	input++;
	return input + block_size - input % block_size;
}

#define K_SZ static_cast<size_t>(128 / 8) // Size of label in bytes
#define C_SZ aes_cipher_size(K_SZ) // Size of aes(label) in bytes

class label_t : public std::array<unsigned char, K_SZ>
{
public:
	void fill(prng_state &prng)
	{
		if (rng_make_prng(128, find_prng("yarrow"), &prng, NULL) != CRYPT_OK)
			throw std::runtime_exception();
		auto sz = sizeof(value_type) * size();
		if (yarrow_read(begin(), sz, &prng) != sz)
			throw std::runtime_exception();
	}
};

class clabel_t : public std::array<unsigned char, C_SZ>
{
public:
	clabel_t()
};

template <size_t Na, size_t Nb>
class garbled_table
{
public:
	template <typename Iter>
	garbled_table(Iter mita, Iter mitb, size_t mc, Func fun)
	{
		for (decltype(auto) [ls, m] : std::zip(_la, mita))
			ls.resize(m);
		for (decltype(auto) [ls, m] : std::zip(_lb, mitb))
			ls.resize(m);
		lc.resize(mc);

		size_t cs = 1;
		for (auto m : mita)
			cs *= m;
		for (auto m : mitb)
			cs *= m;
		_t.reserve(cs);
	}

	template <typename Func>
	void garble(Func fun)
	{
		prng_state prng;
		for (decltype(auto) ls : _la)
			for (decltype(auto) l : ls)
				l.fill(prng);
		for (decltype(auto) ls : _lb)
			for (decltype(auto) l : ls)
				l.fill(prng);
		for (decltype(auto) l : lc)
			l.fill(prng);

		std::array<size_t, Na> a, b;
		while (true)
		{
			auto flag = false;
			if (!flag)
				for (decltype(auto) [v, m] : std::zip(a, _la))
				{
					if (v == m - 1)
					{
						v = 0;
						continue;
					}

					v++;
					flag = true;
					break;
				}

			if (!flag)
				for (decltype(auto) [v, m] : std::zip(b, _lb))
				{
					if (v == m - 1)
					{
						v = 0;
						continue;
					}

					v++;
					flag = true;
					break;
				}

			if (!flag)
				break;

			size_t c = fun(
					const_cast<const std::array<size_t, Na>>(a),
					const_cast<const std::array<size_t, Na>>(b));

			// TODO: h = Hash(_la[a], _lb[b])
			// TODO: c = Enc_h(c)

			_t.push_back(c); // TODO: randomize it
		}
	}

private:
	std::array<std::vector<label_t>, Na> _la;
	std::array<std::vector<label_t>, Nb> _lb;
	std::vector<label_t> _lc;
	std::vector<clabel_t> _t;
};
