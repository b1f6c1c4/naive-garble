#include <tomcrypt.h>
#include <exception>
#include <vector>
#include "util.hpp"

inline constexpr auto aes_cipher_size(size_t input)
{
	constexpr size_t block_size = 128 / 8;
	input++;
	return input + block_size - input % block_size;
}

#define K_SZ static_cast<size_t>(128 / 8) // Size of label in bytes
#define C_SZ aes_cipher_size(K_SZ) // Size of aes(label) in bytes

typedef std::array<byte_t, K_SZ> label_t;
typedef std::array<byte_t, C_SZ> clabel_t;

template <size_t Na, size_t Nb>
class garbled_table
{
public:
	template <typename ContainerA, typename ContainerB>
	garbled_table(const ContainerA &mita, const ContainerB &mitb, size_t mc) : _sz{1}
	{
		for (auto &&[ls, m] : zip(_la, mita))
			ls.resize(m);
		for (auto &&[ls, m] : zip(_lb, mitb))
			ls.resize(m);
		_lc.resize(mc);

		for (auto m : mita)
			_sz *= m;
		for (auto m : mitb)
			_sz *= m;
		_t.resize(_sz);
	}

	template <typename Func>
	void garble(Func fun)
	{
		int err;
		prng_state prng;
		if ((err = rng_make_prng(128, find_prng("yarrow"), &prng, NULL)) != CRYPT_OK)
			throw std::runtime_error(error_to_string(err));

		for (decltype(auto) ls : _la)
			for (decltype(auto) l : ls)
				random_fill(l, prng);
		for (decltype(auto) ls : _lb)
			for (decltype(auto) l : ls)
				random_fill(l, prng);
		for (decltype(auto) l : _lc)
			random_fill(l, prng);

		decltype(auto) ids = raw_vector<size_t>(_sz);
		for (size_t i = 0; i < _sz; i++)
			ids[i] = i;

		{
			decltype(auto) rs = random_vector<size_t>(_sz, prng);
			for (size_t i = 0; i < _sz - 1; i++)
				std::swap(ids[rs[i] % (_sz - i)], ids[_sz - i - 1]);
		}

		std::array<size_t, Na> a, b;
		for (auto &&[target, abid] : zip(_t, ids))
		{
			hash_state hash;
			sha256_init(&hash);

			auto r = abid;
			for (auto &&[v, l] : zip(a, _la))
			{
				v = r % l.size();
				r = r / l.size(); // r is raw_t<>
				sha256_process(&hash, get_ptr(l), get_sz(l));
			}
			for (auto &&[v, l] : zip(b, _lb))
			{
				v = r % l.size();
				r = r / l.size(); // r is raw_t<>
				sha256_process(&hash, get_ptr(l), get_sz(l));
			}

			decltype(auto) h = random_vector<unsigned char>(256 / 8, prng);
			sha256_done(&hash, get_ptr(h));

			size_t c = fun(make_const(a), make_const(b));
			decltype(auto) lc = _lc[c];

			symmetric_CTR ctr;
			auto key = get_ptr(h);
			auto iv = key + 128 / 8;
			if ((err = ctr_start(find_cipher("aes"), iv, key, 128 / 8, 0, CTR_COUNTER_LITTLE_ENDIAN, &ctr)) != CRYPT_OK)
				throw std::runtime_error(error_to_string(err));

			if ((err = ctr_encrypt(get_ptr(lc), get_ptr(target), get_sz(lc), &ctr)) != CRYPT_OK)
				throw std::runtime_error(error_to_string(err));
		}
	}

	decltype(auto) get_label_alice(size_t i, size_t v) const
	{
		return _la[i][v];
	}

	decltype(auto) get_table() const
	{
		return _t;
	}

private:
	size_t _sz;
	std::array<std::vector<label_t>, Na> _la;
	std::array<std::vector<label_t>, Nb> _lb;
	std::vector<label_t> _lc;
	std::vector<clabel_t> _t;
};
