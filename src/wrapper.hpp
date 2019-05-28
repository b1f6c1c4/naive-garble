#pragma once

#include <tomcrypt.h>
#include <utility>
#include <functional>
#include "garbled_table.hpp"
#include "oblivious_transfer.hpp"

#define BIND(obj, fun, ...) std::bind(&decltype(obj)::fun, &obj, __VA_ARGS__)

using namespace std::placeholders;

template <size_t Na, size_t Nb, size_t K = 128 / 8, size_t KN = 1024 / 8>
class wrapper_alice
{
public:
	template <typename ContainerA, typename ContainerB>
	wrapper_alice(const ContainerA &mita, const ContainerB &mitb, size_t mc)
		: _tbl(mita, mitb, mc)
	{
		_ot.reserve(Nb);
		for (size_t i = 0; i < Nb; i++)
			_ot.emplace_back(mitb[i]);
	}

	auto garble_size() const
	{
		size_t sz = 0;
		for (decltype(auto) o : _ot)
		{
			sz += o.dump_crypt_size();
			sz += o.dump_size();
		}
		return sz;
	}

	template <typename Func, typename Iter>
	auto garble(Iter it, prng_state &prng, Func fun)
	{
		_tbl.garble(fun, prng);
		for (decltype(auto) o : _ot)
			o.initiate(prng);

		for (decltype(auto) o : _ot)
		{
			it += o.dump_crypt(it);
			it += o.dump(it);
		}

		return it;
	}

	auto receive_size() const
	{
		size_t sz = 0;
		for (decltype(auto) o : _ot)
			sz += o.dump_size();
		sz += _tbl.dump_size();
		return sz;
	}

	template <typename IterA, typename Func, typename IterB>
	auto receive(IterA ita, IterB itb, Func fun)
	{
		for (size_t i = 0; i < Nb; i++)
		{
			decltype(auto) o = _ot[i];
			ita += o.receive(ita, BIND(_tbl, get_label_bob, i, _1));
			itb += o.dump(itb);
		}
		itb += _tbl.dump(itb, fun);

		return itb;
	}

private:
	garbled_table<Na, Nb, K> _tbl;
	std::vector<oblivious_transfer_sender<KN>> _ot;
};

template <size_t Na, size_t Nb, size_t K = 128 / 8, size_t KN = 1024 / 8>
class wrapper_bob
{
public:
	template <typename ContainerA, typename ContainerB>
	wrapper_bob(const ContainerA &mita, const ContainerB &mitb, size_t mc)
		: _sz{1}, _mc{mc}
	{
		for (auto m : mita)
			_sz *= m;
		for (auto m : mitb)
			_sz *= m;

		_ot.reserve(Nb);
		for (size_t i = 0; i < Nb; i++)
			_ot.emplace_back(mitb[i]);
	}

	auto inquiry_size() const
	{
		return Nb * KN;
	}

	template <typename IterA, typename Func, typename IterB>
	auto inquiry(IterA ita, prng_state &prng, IterB itb, Func fun)
	{
		for (size_t i = 0; i < Nb; i++)
		{
			decltype(auto) o = _ot[i];
			decltype(auto) b = fun(i);
			ita += o.initiate(ita, b, prng);
		}

		for (decltype(auto) o : _ot)
			itb += copy(o.get_result(), itb);

		return itb;
	}

	template <typename Iter>
	auto evaluate(Iter it)
	{
		for (decltype(auto) o : _ot)
			it += o.receive(it);

		return garbled_table<Na, Nb, K>::evaluate(_sz, _mc, it,
				[this](size_t id){
					return get_ptr(_ot[id].get_result()) + KN - K;
				});
	}

private:
	size_t _sz, _mc;
	std::vector<oblivious_transfer_receiver<KN>> _ot;
};

#undef BIND
