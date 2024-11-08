#ifndef ENGINE_H
#define ENGINE_H

#include "dictionary.h"
#include <xkbcommon/xkbcommon.h>

class t_engine
{
public:
	struct t_state
	{
		size_t v_base;
		size_t v_text = 0;
		size_t v_entry = 0;
		size_t v_okuri = 0;

		t_state(size_t a_base = 0) : v_base(a_base)
		{
		}
	};
	enum t_attribute
	{
		c_attribute__ROMAN,
		c_attribute__TEXT,
		c_attribute__ENTRY,
		c_attribute__OKURI,
		c_attribute__CANDIDATE
	};

private:
	struct t_action
	{
		char v_key;

		t_action(char a_key) : v_key(a_key)
		{
		}
		virtual void f_key_pressed(t_engine* a_engine) const = 0;
	};
	struct t_map
	{
		struct t_less
		{
			bool operator()(const t_action* a_x, char a_y) const
			{
				return a_x->v_key < a_y;
			}
			bool operator()(char a_x, const t_action* a_y) const
			{
				return a_x < a_y->v_key;
			}
			bool operator()(const t_action* a_x, const t_action* a_y) const
			{
				return a_x->v_key < a_y->v_key;
			}
		};

		const t_action* const* v_first;
		const t_action* const* v_last;

		t_map(const t_action* const* a_first, const t_action* const* a_last) : v_first(a_first), v_last(a_last)
		{
		}
		const t_action* f_find(char a_key) const;
	};
	struct t_action_push : t_action
	{
		t_map v_next;

		t_action_push(char a_key, const t_action* const* a_first, const t_action* const* a_last) : t_action(a_key), v_next(a_first, a_last)
		{
		}
		virtual void f_key_pressed(t_engine* a_engine) const;
	};
	friend struct t_action_push;
	struct t_action_reduce : t_action
	{
		const wchar_t* v_cs0;
		const wchar_t* v_cs1;
		const t_action* v_next;

		t_action_reduce(char a_key, const wchar_t* a_cs0, const wchar_t* a_cs1, const t_action* a_next = 0) : t_action(a_key), v_cs0(a_cs0), v_cs1(a_cs1), v_next(a_next)
		{
		}
		virtual void f_key_pressed(t_engine* a_engine) const;
	};
	friend struct t_action_reduce;
	friend struct t_roman_table;

	t_dictionary& v_dictionary;
	void (t_engine::*v_mode)(xkb_keysym_t, char) = &t_engine::f_mode_roman;
	std::vector<wchar_t> v_buffer;
	bool v_katakana = false;
	const t_map* v_map;
	std::vector<t_state> v_states;
	std::deque<t_candidate> v_candidates;
	size_t v_chosen;
	bool v_choosing;

	void f_roman_reset();
	void f_roman_push(wchar_t a_c, const t_map* a_map);
	void f_roman_reduce(const wchar_t* a_cs, size_t a_n);
	void f_roman_commit();
	void f_text_push(char a_ascii);
	void f_text_pop();
	void f_text_commit();
	void f_entry_push(char a_ascii);
	void f_entry_pop();
	void f_entry_commit();
	void f_choose();
	void f_mode_latin(xkb_keysym_t a_key, char a_ascii);
	void f_mode_word(xkb_keysym_t a_key, char a_ascii);
	void f_mode_roman(xkb_keysym_t a_key, char a_ascii);
	void f_mode_kanji(xkb_keysym_t a_key, char a_ascii);
	void f_mode_choose(xkb_keysym_t a_key, char a_ascii);

protected:
	virtual void f_on_forward() = 0;
	virtual void f_on_compose(size_t a_i, size_t a_m, const wchar_t* a_cs, const t_attribute* a_as, size_t a_n) = 0;
	virtual void f_on_commit(const wchar_t* a_cs, size_t a_n) = 0;
	virtual void f_on_status() = 0;
	virtual void f_on_candidates() = 0;
	virtual void f_on_choose() = 0;

public:
	t_engine(t_dictionary& a_dictionary);
	std::wstring f_status() const;
	std::wstring f_entry(size_t a_i) const;
	size_t f_caret() const
	{
		return v_states.back().v_text;
	}
	const std::vector<wchar_t>& f_buffer() const
	{
		return v_buffer;
	}
	const std::vector<t_state>& f_states() const
	{
		return v_states;
	}
	const std::deque<t_candidate>& f_candidates() const
	{
		return v_candidates;
	}
	size_t f_chosen() const
	{
		return v_chosen;
	}
	bool f_choosing() const
	{
		return v_choosing;
	}
	void f_reset();
	void f_key_pressed(xkb_keysym_t a_key, char a_ascii)
	{
		(this->*v_mode)(a_key, a_ascii);
	}
};

#endif
