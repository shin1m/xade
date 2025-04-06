#ifndef XEMMAIX__XADE__CLIENT_H
#define XEMMAIX__XADE__CLIENT_H

#include "xade.h"
#include <xade/converter.h>

namespace xemmaix::xade
{

struct t_client
{
	t_session* v_client = t_session::f_instance();

	void f_check()
	{
		if (!v_client || v_client != t_session::f_instance()) f_throw(L"not valid."sv);
	}
	t_object* f_pointer_focus();
	t_object* f_pointer()
	{
		f_check();
		return f_tuple(v_client->f_pointer_x(), v_client->f_pointer_y());
	}
	t_object* f_keyboard_focus();
	t_object* f_cursor();
	void f_cursor__(const t_cursor* a_value);
	t_object* f_input_focus();
	std::shared_ptr<::xade::t_input> f_new_input()
	{
		f_check();
		return v_client->f_new_input();
	}
};

#define XEMMAIX__XADE__ON(name, arguments, index, ...)\
		v_on_##name = [&]arguments\
		{\
			if (auto& on = t_object::f_of(this)->f_fields()[index]) on(__VA_ARGS__);\
		};
#define XEMMAIX__XADE__SURFACE__ONS\
		XEMMAIX__XADE__ON(frame, (auto a_time), 0, a_time)\
		XEMMAIX__XADE__ON(pointer_enter, , 1, )\
		XEMMAIX__XADE__ON(pointer_leave, , 2, )\
		XEMMAIX__XADE__ON(pointer_move, , 3, )\
		XEMMAIX__XADE__ON(button_press, (auto a_button), 4, a_button)\
		XEMMAIX__XADE__ON(button_release, (auto a_button), 5, a_button)\
		XEMMAIX__XADE__ON(scroll, (auto a_axis, auto a_value), 6, t_object::f_of(this)->f_type()->v_module->f_as<t_library>().f_as(a_axis), a_value)\
		XEMMAIX__XADE__ON(focus_enter, , 7, )\
		XEMMAIX__XADE__ON(focus_leave, , 8, )\
		XEMMAIX__XADE__ON(key_press, (auto a_sym, auto a_c), 9, a_sym, a_c)\
		XEMMAIX__XADE__ON(key_release, (auto a_sym, auto a_c), 10, a_sym, a_c)\
		XEMMAIX__XADE__ON(key_repeat, (auto a_sym, auto a_c), 11, a_sym, a_c)\
		XEMMAIX__XADE__ON(input_enable, , 12, )\
		XEMMAIX__XADE__ON(input_disable, , 13, )\
		XEMMAIX__XADE__ON(input_done, , 14, )

struct t_surface : t_proxy_of<::xade::t_surface>
{
	t_surface(bool a_depth) : t_base(a_depth)
	{
		XEMMAIX__XADE__SURFACE__ONS
	}
};

struct t_frame : t_proxy_of<::xade::t_frame>
{
	t_frame(bool a_depth) : t_base(a_depth)
	{
		XEMMAIX__XADE__SURFACE__ONS
		v_on_measure = [&](auto& a_width, auto& a_height)
		{
			if (auto& on = t_object::f_of(this)->f_fields()[15]) {
				auto size = on(a_width, a_height);
				auto width = size.f_get_at(0);
				xemmai::f_check<int32_t>(width, L"width");
				a_width = f_as<int32_t>(width);
				auto height = size.f_get_at(1);
				xemmai::f_check<int32_t>(height, L"height");
				a_height = f_as<int32_t>(height);
			}
		};
		XEMMAIX__XADE__ON(map, (auto a_width, auto a_height), 16, a_width, a_height)
		XEMMAIX__XADE__ON(unmap, , 17, )
		XEMMAIX__XADE__ON(close, , 18, )
	}
};

struct t_cursor : t_proxy_of<::xade::t_cursor>
{
	t_cursor(std::wstring_view a_name) : t_base(portable::f_convert(a_name).c_str())
	{
	}
};

struct t_input : t_proxy_of<std::shared_ptr<::xade::t_input>>
{
	static auto f_appender(auto& a_s)
	{
		return [&](auto p, auto n)
		{
			a_s << std::wstring_view(p, n);
		};
	}

	t_converter<char, wchar_t> v_utf8towc{"utf-8", "wchar_t"};

	t_input(const std::shared_ptr<::xade::t_input>& a_input) : t_base(a_input)
	{
		v_session->v_objects.emplace(get(), t_object::f_of(this));
	}
	virtual void f_dispose()
	{
		v_session->v_objects.erase(get());
		t_base::f_dispose();
	}
	t_object* f_preedit()
	{
		auto [text, begin, end] = get()->f_preedit();
		if (begin > end) return f_tuple(nullptr, begin, end);
		auto p = text.c_str();
		t_stringer s;
		size_t b;
		if (begin < 0) {
			b = begin;
			begin = 0;
		} else {
			v_utf8towc(p, begin, f_appender(s));
			b = static_cast<std::wstring_view>(s).size();
		}
		size_t e;
		if (end < 0) {
			v_utf8towc(p + begin, text.size() - begin, f_appender(s));
			e = end;
		} else {
			v_utf8towc(p + begin, end - begin, f_appender(s));
			e = static_cast<std::wstring_view>(s).size();
			v_utf8towc(p + end, text.size() - end, f_appender(s));
		}
		return f_tuple(static_cast<t_object*>(s), b, e);
	}
	t_object* f_text()
	{
		t_stringer s;
		auto& text = get()->f_text();
		v_utf8towc(text.c_str(), text.size(), f_appender(s));
		return s;
	}
};

inline t_object* t_client::f_pointer_focus()
{
	f_check();
	return t_object::f_of(static_cast<t_surface*>(v_client->f_pointer_focus()));
}

inline t_object* t_client::f_keyboard_focus()
{
	f_check();
	return t_object::f_of(static_cast<t_surface*>(v_client->f_keyboard_focus()));
}

inline t_object* t_client::f_cursor()
{
	f_check();
	return t_object::f_of(const_cast<t_cursor*>(static_cast<const t_cursor*>(v_client->f_cursor())));
}

inline void t_client::f_cursor__(const t_cursor* a_value)
{
	f_check();
	v_client->f_cursor__(a_value);
}

inline t_object* t_client::f_input_focus()
{
	f_check();
	return t_object::f_of(static_cast<t_surface*>(v_client->f_input_focus()));
}

}

namespace xemmai
{

template<>
struct t_type_of<xemmaix::xade::t_client> : t_uninstantiatable<t_bears<xemmaix::xade::t_client>>
{
	using t_library = xemmaix::xade::t_library;

	static void f_define(t_library* a_library);

	using t_base::t_base;
};

template<>
struct t_type_of<wl_pointer_axis> : t_enum_of<wl_pointer_axis, xemmaix::xade::t_library>
{
	static t_object* f_define(t_library* a_library);

	using t_base::t_base;
};

template<>
struct t_fundamental<xade::t_surface>
{
	using t_type = xemmaix::xade::t_surface;
};

template<>
struct t_type_of<xemmaix::xade::t_surface> : t_derivable<t_bears<xemmaix::xade::t_surface, t_type_of<xemmaix::xade::t_proxy>>>
{
	static void f_define(t_library* a_library);

	using t_base::t_base;
	t_pvalue f_do_construct(t_pvalue* a_stack, size_t a_n);
};

template<>
struct t_type_of<xdg_toplevel_state> : t_enum_of<xdg_toplevel_state, xemmaix::xade::t_library>
{
	static t_object* f_define(t_library* a_library);

	using t_base::t_base;
};

template<>
struct t_type_of<xdg_toplevel_wm_capabilities> : t_enum_of<xdg_toplevel_wm_capabilities, xemmaix::xade::t_library>
{
	static t_object* f_define(t_library* a_library);

	using t_base::t_base;
};

template<>
struct t_type_of<xdg_toplevel_resize_edge> : t_enum_of<xdg_toplevel_resize_edge, xemmaix::xade::t_library>
{
	static t_object* f_define(t_library* a_library);

	using t_base::t_base;
};

template<>
struct t_fundamental<xade::t_frame>
{
	using t_type = xemmaix::xade::t_frame;
};

template<>
struct t_type_of<xemmaix::xade::t_frame> : t_derivable<t_bears<xemmaix::xade::t_frame, t_type_of<xemmaix::xade::t_surface>>>
{
	static void f_define(t_library* a_library);

	using t_base::t_base;
	t_pvalue f_do_construct(t_pvalue* a_stack, size_t a_n);
};

template<>
struct t_type_of<xemmaix::xade::t_cursor> : t_bears<xemmaix::xade::t_cursor, t_type_of<xemmaix::xade::t_proxy>>
{
	using t_base::t_base;
	t_pvalue f_do_construct(t_pvalue* a_stack, size_t a_n);
};

template<>
struct t_fundamental<std::shared_ptr<xade::t_input>>
{
	using t_type = xemmaix::xade::t_input;
};

template<>
struct t_type_of<xemmaix::xade::t_input> : t_uninstantiatable<t_bears<xemmaix::xade::t_input, t_type_of<xemmaix::xade::t_proxy>>>
{
	static t_pvalue f_transfer(const t_library* a_library, const std::shared_ptr<xade::t_input>& a_value)
	{
		if (!a_value) return nullptr;
		auto& objects = xemmaix::xade::t_session::f_instance()->v_objects;
		auto i = objects.find(a_value.get());
		return i == objects.end() ? xemmai::f_new<xemmaix::xade::t_input>(a_library, a_value) : i->second;
	}
	static void f_define(t_library* a_library);

	using t_base::t_base;
};

}

#endif
