#pragma once

#include <tomcrypt.h>
#include <exception>
#include <algorithm>
#include <vector>
#include "util.hpp"

template <typename Iter>
inline auto mp_dump(void *v, Iter c, size_t sz)
{
	size_t n = mp_unsigned_bin_size(v);
	auto mid = get_ptr(c) + (sz - n);
	std::fill(get_ptr(c), mid, 0);
	return mp_to_unsigned_bin(v, mid);
}

template <typename Container>
inline auto mp_dump(void *v, Container &c)
{
	return mp_dump(v, get_ptr(c), get_sz(c));
}

template <size_t KN = 1024 / 8>
class oblivious_transfer_sender
{
	typedef std::array<byte_t, KN> msg_t;

public:
	oblivious_transfer_sender(size_t m) : _state{Invalid} { _x.resize(m); }
	oblivious_transfer_sender(const oblivious_transfer_sender &) = delete;
	oblivious_transfer_sender(oblivious_transfer_sender &&other)
		: _state(other._state), _rsa(std::move(other._rsa)),
		 _x(std::move(other._x))
	{
		other._state = Invalid;
		other._rsa.N = nullptr;
		other._rsa.e = nullptr;
		other._rsa.d = nullptr;
		other._rsa.p = nullptr;
		other._rsa.q = nullptr;
		other._rsa.qP = nullptr;
		other._rsa.dP = nullptr;
		other._rsa.dQ = nullptr;
	}
	~oblivious_transfer_sender() { rsa_free(&_rsa); }

	void initiate(prng_state &prng)
	{
		if (_state != Invalid)
			throw std::runtime_error("state is invalid");

		RUN(rsa_make_key(nullptr, find_prng("sprng"), KN, 65537, &_rsa));

		for (decltype(auto) x : _x)
			random_fill(x, prng);

		_state = Initiated;
	}

	template <typename Iter, typename Func>
	void receive(Iter it, Func fun)
	{
		if (_state != Initiated)
			throw std::runtime_error("state is invalid");

		void *v, *k, *tmp1, *tmp2;
		RUN(mp_init_multi(&v, &k, &tmp1, &tmp2, nullptr));
		RUN(mp_read_unsigned_bin(tmp1, get_ptr(it), KN));
		RUN(mp_add(tmp1, _rsa.N, v));

		for (size_t i = 0; i < _x.size(); i++)
		{
			decltype(auto) x = _x[i];

			RUN(mp_read_unsigned_bin(k, get_ptr(x), get_sz(x)));
			RUN(mp_sub(v, k, tmp1));
			RUN(mp_exptmod(tmp1, _rsa.d, _rsa.N, k));

			{
				decltype(auto) res = fun(i);
				decltype(auto) workaround = const_cast<unsigned char *>(get_ptr(res));
				RUN(mp_read_unsigned_bin(tmp1, workaround, get_sz(res)));
			}

			RUN(mp_add(k, tmp1, tmp2));
			RUN(mp_mod(tmp2, _rsa.N, tmp1));
			RUN(mp_dump(tmp1, x));
		}

		mp_cleanup_multi(&v, &k, &tmp1, &tmp2, nullptr);

		_state = Received;
	}

	auto dump_size() const
	{
		switch (_state)
		{
			case Initiated:
				return KN + get_sz(_x);
			case Received:
				return get_sz(_x);
			default:
				throw std::runtime_error("state is invalid");
		}
	}

	template <typename Iter>
	void dump(Iter it) const
	{
		switch (_state)
		{
			case Initiated:
				RUN(mp_dump(_rsa.N, it, KN));
				it += KN;
				// fallthrough;
			case Received:
				copy(_x, it);
				break;
			default:
				throw std::runtime_error("state is invalid");
		}
	}

private:
	enum {
		Invalid,
		Initiated,
		Received,
	} _state;

	rsa_key _rsa;
	std::vector<msg_t> _x;
};


template <size_t KN = 1024 / 8>
class oblivious_transfer_receiver
{
	typedef std::array<byte_t, KN> msg_t;

public:
	oblivious_transfer_receiver(size_t m) : _state{Invalid} { }
	oblivious_transfer_receiver(const oblivious_transfer_receiver &) = delete;
	oblivious_transfer_receiver(oblivious_transfer_receiver &&other)
		: _state(other._state), _b(other._b), _N(other._N),
		  _e(other._e), _k(other._k), _v(std::move(other._v))
	{
		other._state = Invalid;
		other._N = nullptr;
		other._e = nullptr;
		other._k = nullptr;
	}

	~oblivious_transfer_receiver() { mp_cleanup_multi(&_N, &_e, &_k, nullptr); }

	template <typename Iter>
	void initiate(Iter it, size_t b, prng_state &prng)
	{
		if (_state != Invalid)
			throw std::runtime_error("state is invalid");

		_b = b;

		void *v, *tmp1, *tmp2;
		RUN(mp_init_multi(&_k, &v, &tmp1, &tmp2, &_N, &_e, nullptr));

		{
			decltype(auto) k = random_vector<unsigned char>(KN, prng);
			RUN(mp_read_unsigned_bin(_k, get_ptr(k), KN));
		}

		RUN(mp_read_unsigned_bin(_N, get_ptr(it), KN));
		RUN(mp_set_int(_e, 65537));

		it += KN + _b * KN;
		RUN(mp_read_unsigned_bin(v, get_ptr(it), KN));

		RUN(mp_exptmod(_k, _e, _N, tmp1));
		RUN(mp_add(v, tmp1, tmp2));
		RUN(mp_mod(tmp2, _N, v));
		RUN(mp_dump(v, _v));

		mp_cleanup_multi(&v, &tmp1, &tmp2, nullptr);

		_state = Initiated;
	}

	template <typename Iter>
	void receive(Iter it)
	{
		if (_state != Initiated)
			throw std::runtime_error("state is invalid");

		void *m, *tmp;
		RUN(mp_init_multi(&m, &tmp, nullptr));

		it += _b * KN;
		RUN(mp_read_unsigned_bin(tmp, get_ptr(it), KN));
		RUN(mp_add(tmp, _N, m));
		RUN(mp_sub(m, _k, tmp));
		RUN(mp_mod(tmp, _N, m));
		RUN(mp_dump(m, _v));

		mp_cleanup_multi(&m, &tmp, nullptr);

		_state = Received;
	}

	auto dump_size() const
	{
		switch (_state)
		{
			case Initiated:
			case Received:
				return get_sz(_v);
			default:
				throw std::runtime_error("state is invalid");
		}
	}

	template <typename Iter>
	void dump(Iter it)
	{
		int err;
		switch (_state)
		{
			case Initiated:
			case Received:
				copy(_v, it);
				break;
			default:
				throw std::runtime_error("state is invalid");
		}
	}

private:
	enum {
		Invalid,
		Initiated,
		Received,
	} _state;

	size_t _b;
	void *_N, *_e, *_k;
	msg_t _v;
};
