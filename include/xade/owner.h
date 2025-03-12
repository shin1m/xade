#ifndef XADE__OWNER_H
#define XADE__OWNER_H

namespace xade
{

template<typename T, auto A_destroy, T A_none = {}>
class t_owner
{
	T v_p;

public:
	t_owner(T a_p = A_none) : v_p(a_p)
	{
	}
	t_owner(const t_owner&) = delete;
	~t_owner()
	{
		if (v_p != A_none) A_destroy(v_p);
	}
	t_owner& operator=(const t_owner&) = delete;
	t_owner& operator=(T a_p)
	{
		if (v_p != A_none) A_destroy(v_p);
		v_p = a_p;
		return *this;
	}
	operator T() const
	{
		return v_p;
	}
	T operator->() const
	{
		return v_p;
	}
};

}

#endif
