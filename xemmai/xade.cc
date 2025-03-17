#include "client.h"

namespace xemmaix::xade
{

t_entry::t_entry() : v_previous(t_session::f_instance()), v_next(v_previous->v_next)
{
	v_previous->v_next = v_next->v_previous = this;
}

void t_entry::f_dispose()
{
	v_previous->v_next = v_next;
	v_next->v_previous = v_previous;
	v_previous = v_next = nullptr;
}

XEMMAI__PORTABLE__THREAD t_session* t_session::v_instance;

t_session::t_session() : t_entry(false)
{
	if (v_instance) f_throw(L"already inside main."sv);
	v_instance = this;
	v_on_idle.push_back([&]
	{
		if (auto& on = v_object->f_fields()[/*on_idle*/0]) on();
	});
}

t_session::~t_session()
{
	while (v_next != this) v_next->f_dispose();
	v_object->f_as<xemmaix::xade::t_client>().v_client = v_instance = nullptr;
}

void t_proxy::f_dispose()
{
	v_object = nullptr;
	t_entry::f_dispose();
}

void t_library::f_main(t_library* a_library, const t_pvalue& a_callable)
{
	t_session session;
	session.v_object = f_new<t_client>(a_library);
	a_callable(session.v_object);
}

void t_library::f_scan(t_scan a_scan)
{
	a_scan(v_type_client);
	a_scan(v_type_surface);
	a_scan(v_type_frame);
	a_scan(v_type_cursor);
	a_scan(v_type_input);
}

std::vector<std::pair<t_root, t_rvalue>> t_library::f_define()
{
	t_type_of<t_client>::f_define(this);
	t_type_of<t_surface>::f_define(this);
	t_type_of<t_frame>::f_define(this);
	t_define{this}.f_derive<t_cursor, t_object>();
	t_type_of<t_input>::f_define(this);
	return t_define(this)
		(L"Client"sv, static_cast<t_object*>(v_type_client))
		(L"Surface"sv, static_cast<t_object*>(v_type_surface))
		(L"Frame"sv, static_cast<t_object*>(v_type_frame))
		(L"Cursor"sv, static_cast<t_object*>(v_type_cursor))
		(L"Input"sv, static_cast<t_object*>(v_type_input))
		(L"main"sv, t_static<void(*)(t_library*, const t_pvalue&), f_main>())
		(L"client"sv, t_static<t_object*(*)(), []
		{
			return static_cast<t_object*>(t_session::f_instance()->v_object);
		}>())
	;
}

}

XEMMAI__MODULE__FACTORY(xemmai::t_library::t_handle* a_handle)
{
	return xemmai::f_new<xemmaix::xade::t_library>(a_handle);
}
