#ifndef XADE__CONVERTER_H
#define XADE__CONVERTER_H

#include <algorithm>
#include <system_error>
#include <cerrno>
#include <climits>
#include <cwchar>
#include <iconv.h>

namespace xade
{

class t_iconv
{
protected:
	iconv_t v_cd;

public:
	t_iconv(const char* a_from, const char* a_to) : v_cd(iconv_open(a_to, a_from))
	{
	}
	~t_iconv()
	{
		iconv_close(v_cd);
	}
	template<typename C>
	int f_to(char** a_p, size_t* a_n, auto a_out) const
	{
		char cs[16];
		char* p = cs;
		size_t n = sizeof(cs);
		while (iconv(v_cd, a_p, a_n, &p, &n) == size_t(-1)) {
			auto e = errno;
			if (e == EINTR) continue;
			a_out(reinterpret_cast<const C*>(cs), (p - cs) / sizeof(C));
			if (e != E2BIG) return e;
			p = cs;
			n = sizeof(cs);
		}
		a_out(reinterpret_cast<const C*>(cs), (p - cs) / sizeof(C));
		return 0;
	}
};

template<typename C0, typename C1>
struct t_converter : private t_iconv
{
	using t_iconv::t_iconv;
	const C0* operator()(const C0* a_p, size_t a_n, auto a_out) const
	{
		auto p = reinterpret_cast<char*>(const_cast<C0*>(a_p));
		size_t n = a_n * sizeof(C0);
		auto e = f_to<C1>(&p, &n, a_out);
		if (e == EINVAL) return reinterpret_cast<C0*>(p);
		if (e == 0) e = f_to<C1>(nullptr, nullptr, a_out);
		if (e == 0) return nullptr;
		throw std::system_error(e, std::generic_category());
	}
};

template<typename T>
class t_encoder : t_iconv
{
	T& v_target;
	wchar_t v_cs[16];
	size_t v_n = 0;

	void f_convert()
	{
		char* p = reinterpret_cast<char*>(const_cast<wchar_t*>(v_cs));
		size_t n = v_n * sizeof(wchar_t);
		v_n = 0;
		auto e = f_to<char>(&p, &n, v_target);
		if (e != 0) throw std::system_error(e, std::generic_category());
	}

public:
	t_encoder(T& a_target, const char* a_encoding) : t_iconv("wchar_t", a_encoding), v_target(a_target)
	{
	}
	~t_encoder()
	{
		f_flush();
	}
	void operator()(wchar_t a_c)
	{
		if (v_n >= sizeof(v_cs) / sizeof(wchar_t)) f_convert();
		v_cs[v_n++] = a_c;
	}
	void operator()(const wchar_t* a_cs)
	{
		while (*a_cs != L'\0') (*this)(*a_cs++);
	}
	void operator()(auto a_f, auto a_l)
	{
		for (; a_f != a_l; ++a_f) (*this)(*a_f);
	}
	void f_flush()
	{
		f_convert();
		auto e = f_to<char>(nullptr, nullptr, v_target);
		if (e != 0) throw std::system_error(e, std::generic_category());
	}
};

template<typename T>
class t_decoder : t_iconv
{
	T& v_source;
	char v_mbs[MB_LEN_MAX * 16];
	char* v_p0 = v_mbs;
	char* v_q0 = v_mbs;
	wchar_t v_cs[16];
	wchar_t* v_p = v_cs;
	wchar_t* v_q = v_cs;

	void f_convert()
	{
		v_p = v_q = v_cs;
		do {
			if (v_p0 <= v_mbs) {
				size_t n = v_source(v_q0, v_mbs + sizeof(v_mbs) - v_q0);
				if (n <= 0) break;
				v_q0 += n;
			}
			size_t m = v_q0 - v_p0;
			size_t n = sizeof(v_cs);
			while (true) {
				if (iconv(v_cd, &v_p0, &m, reinterpret_cast<char**>(&v_q), &n) == size_t(-1)) {
					if (errno == EINTR) continue;
					if (errno == E2BIG) return;
					if (errno != EINVAL) throw std::system_error(errno, std::generic_category());
					v_q0 = std::copy(v_p0, v_q0, static_cast<char*>(v_mbs));
					v_p0 = v_mbs;
				} else {
					v_p0 = v_q0 = v_mbs;
				}
				break;
			}
		} while (v_q <= v_cs);
	}

public:
	t_decoder(T& a_source, const char* a_encoding) : t_iconv(a_encoding, "wchar_t"), v_source(a_source)
	{
	}
	wint_t operator()()
	{
		if (v_p >= v_q) f_convert();
		return v_p < v_q ? *v_p++ : WEOF;
	}
	void f_reset()
	{
		iconv(v_cd, NULL, NULL, NULL, NULL);
		v_p0 = v_q0 = v_mbs;
		v_p = v_q = v_cs;
	}
};

}

#endif
