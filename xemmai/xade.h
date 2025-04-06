#ifndef XEMMAIX__XADE__XADE_H
#define XEMMAIX__XADE__XADE_H

#include <xade/layered.h>
#include <xemmai/convert.h>

namespace xemmaix::xade
{

using namespace ::xade;
using namespace xemmai;

class t_entry
{
protected:
	t_entry* v_previous;
	t_entry* v_next;

	t_entry() : v_previous(this), v_next(this)
	{
	}
	t_entry(t_entry* a_previous) : v_previous(a_previous), v_next(a_previous->v_next)
	{
		v_previous->v_next = v_next->v_previous = this;
	}

public:
	virtual void f_dispose();
};

class t_session : public t_entry, public t_client
{
	static inline XEMMAI__PORTABLE__THREAD t_session* v_instance;
	static inline XEMMAI__PORTABLE__THREAD zwlr_layer_shell_v1* v_layer;

public:
	static t_session* f_instance()
	{
		if (!v_instance) f_throw(L"must be inside main."sv);
		return v_instance;
	}
	static zwlr_layer_shell_v1* f_layer()
	{
		if (!v_layer) throw std::runtime_error("layer shell not available");
		return v_layer;
	}

	t_root v_object;
	std::map<void*, t_object*> v_objects;

	t_session();
	~t_session();
};

class t_proxy : t_entry
{
protected:
	t_session* v_session = static_cast<t_session*>(v_previous);
	t_root v_object = t_object::f_of(this);

	t_proxy() : t_entry(t_session::f_instance())
	{
	}

public:
	virtual void f_dispose();
	bool f_valid()
	{
		return v_session == t_session::f_instance() && v_object;
	}
};

template<typename T>
struct t_proxy_of : t_proxy, T
{
	using t_base = t_proxy_of;

	template<typename... T_n>
	t_proxy_of(T_n&&... a_n) : T(std::forward<T_n>(a_n)...)
	{
	}
	virtual void f_dispose()
	{
		T::~T();
		t_proxy::f_dispose();
	}
};

struct t_client;
struct t_surface;
struct t_frame;
struct t_cursor;
struct t_input;
struct t_layered;

class t_library : public xemmai::t_library
{
	t_slot_of<t_type> v_type_client;
	t_slot_of<t_type> v_type_pointer_axis;
	t_slot_of<t_type> v_type_proxy;
	t_slot_of<t_type> v_type_surface;
	t_slot_of<t_type> v_type_frame_state;
	t_slot_of<t_type> v_type_frame_wm_capabilities;
	t_slot_of<t_type> v_type_frame_resize_edge;
	t_slot_of<t_type> v_type_frame;
	t_slot_of<t_type> v_type_cursor;
	t_slot_of<t_type> v_type_input;
	t_slot_of<t_type> v_type_layer;
	t_slot_of<t_type> v_type_anchor;
	t_slot_of<t_type> v_type_keyboard_interactivity;
	t_slot_of<t_type> v_type_layered;

	static void f_main(t_library* a_library, const t_pvalue& a_callable);

public:
	using xemmai::t_library::t_library;
	XEMMAI__LIBRARY__MEMBERS
};

XEMMAI__LIBRARY__BASE(t_library, t_global, f_global())
XEMMAI__LIBRARY__TYPE(t_library, client)
XEMMAI__LIBRARY__TYPE_AS(t_library, wl_pointer_axis, pointer_axis)
XEMMAI__LIBRARY__TYPE(t_library, proxy)
XEMMAI__LIBRARY__TYPE(t_library, surface)
XEMMAI__LIBRARY__TYPE_AS(t_library, xdg_toplevel_state, frame_state)
XEMMAI__LIBRARY__TYPE_AS(t_library, xdg_toplevel_wm_capabilities, frame_wm_capabilities)
XEMMAI__LIBRARY__TYPE_AS(t_library, xdg_toplevel_resize_edge, frame_resize_edge)
XEMMAI__LIBRARY__TYPE(t_library, frame)
XEMMAI__LIBRARY__TYPE(t_library, cursor)
XEMMAI__LIBRARY__TYPE(t_library, input)
XEMMAI__LIBRARY__TYPE_AS(t_library, zwlr_layer_shell_v1_layer, layer)
XEMMAI__LIBRARY__TYPE_AS(t_library, zwlr_layer_surface_v1_anchor, anchor)
XEMMAI__LIBRARY__TYPE_AS(t_library, zwlr_layer_surface_v1_keyboard_interactivity, keyboard_interactivity)
XEMMAI__LIBRARY__TYPE(t_library, layered)

}

namespace xemmai
{

template<>
struct t_type_of<xemmaix::xade::t_proxy> : t_bears<xemmaix::xade::t_proxy>
{
	template<typename T>
	static T& f_cast(auto&& a_object)
	{
		auto& p = static_cast<t_object*>(a_object)->f_as<T>();
		if (!p.f_valid()) f_throw(L"not valid."sv);
		return p;
	}
	template<typename T>
	struct t_cast
	{
		static T f_as(auto&& a_object)
		{
			return std::forward<T>(f_cast<typename t_fundamental<T>::t_type>(std::forward<decltype(a_object)>(a_object)));
		}
		static bool f_is(t_object* a_object)
		{
			return reinterpret_cast<uintptr_t>(a_object) >= c_tag__OBJECT && a_object->f_type()->f_derives<typename t_fundamental<T>::t_type>();
		}
	};
	template<typename T>
	struct t_cast<T*>
	{
		static T* f_as(auto&& a_object)
		{
			return static_cast<t_object*>(a_object) ? &f_cast<typename t_fundamental<T>::t_type>(std::forward<decltype(a_object)>(a_object)) : nullptr;
		}
		static bool f_is(t_object* a_object)
		{
			return reinterpret_cast<uintptr_t>(a_object) == c_tag__NULL || reinterpret_cast<uintptr_t>(a_object) >= c_tag__OBJECT && a_object->f_type()->f_derives<typename t_fundamental<T>::t_type>();
		}
	};

	using t_library = xemmaix::xade::t_library;

	static void f_define(t_library* a_library);

	using t_base::t_base;
};

}

#endif
