#ifndef BASIC_DICTIONARY_H
#define BASIC_DICTIONARY_H

#include "dictionary.h"
#include <map>
#include <xade/converter.h>

using namespace xade;

class t_basic_dictionary : public t_dictionary
{
	using t_map0 = std::map<std::wstring, std::vector<std::wstring>>;
	using t_map1 = std::map<std::wstring, t_map0>;

	static constexpr const char* v_encoding = "euc-jisx0213";

	t_converter<wchar_t, char> v_wctoeuc{"wchar_t", v_encoding};
	std::vector<std::string> v_publics;
	std::string v_private;
	t_map0 v_nashis;
	t_map1 v_aris;

public:
	t_basic_dictionary(const std::vector<std::string>& a_publics, const std::string& a_private) : v_publics(a_publics), v_private(a_private)
	{
	}
	virtual void f_load();
	virtual void f_save() const;
	virtual void f_search(const wchar_t* a_entry, size_t a_n, size_t a_okuri, std::deque<t_candidate>& a_candidates) const;
	virtual void f_register(const wchar_t* a_entry, size_t a_n, size_t a_okuri, const wchar_t* a_text, size_t a_m);
};

inline auto f_appender(auto& a_xs)
{
	return [&](auto a_p, auto a_n)
	{
		a_xs.insert(a_xs.end(), a_p, a_p + a_n);
	};
}

#endif
