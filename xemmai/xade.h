#ifndef XEMMAIX__XADE__XADE_H
#define XEMMAIX__XADE__XADE_H

#include <xade/client.h>
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

	t_entry(bool) : v_previous(this), v_next(this)
	{
	}
	t_entry();

public:
	virtual void f_dispose();
};

class t_session : public t_entry, public t_client
{
	static XEMMAI__PORTABLE__THREAD t_session* v_instance;

public:
	static t_session* f_instance()
	{
		if (!v_instance) f_throw(L"must be inside main."sv);
		return v_instance;
	}

	t_root v_object;
	std::map<void*, t_object*> v_objects;

	t_session();
	~t_session();
};

class t_proxy : t_entry
{
protected:
	t_session* v_session = t_session::f_instance();
	t_root v_object = t_object::f_of(this);

	virtual void f_dispose();
	void f_check()
	{
		if (v_session != t_session::f_instance()) f_throw(L"not valid."sv);
		if (!v_object) f_throw(L"already destroyed."sv);
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

class t_library : public xemmai::t_library
{
	t_slot_of<t_type> v_type_client;
	t_slot_of<t_type> v_type_surface;
	t_slot_of<t_type> v_type_frame;
	t_slot_of<t_type> v_type_cursor;
	t_slot_of<t_type> v_type_input;

	static void f_main(t_library* a_library, const t_pvalue& a_callable);

public:
	using xemmai::t_library::t_library;
	XEMMAI__LIBRARY__MEMBERS
};

XEMMAI__LIBRARY__BASE(t_library, t_global, f_global())
XEMMAI__LIBRARY__TYPE(t_library, client)
XEMMAI__LIBRARY__TYPE(t_library, surface)
XEMMAI__LIBRARY__TYPE(t_library, frame)
XEMMAI__LIBRARY__TYPE(t_library, cursor)
XEMMAI__LIBRARY__TYPE(t_library, input)

}

#endif
