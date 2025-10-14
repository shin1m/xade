#include "layered.h"
#include <linux/input-event-codes.h>
#include <xkbcommon/xkbcommon-keysyms.h>

namespace xemmaix::xade
{

void t_entry::f_dispose()
{
	v_previous->v_next = v_next;
	v_next->v_previous = v_previous;
}

t_session::t_session() : t_client([&](auto a_this, auto a_name, auto a_interface, auto a_version)
{
	if (std::strcmp(a_interface, zwlr_layer_shell_v1_interface.name) == 0) v_layer = static_cast<zwlr_layer_shell_v1*>(wl_registry_bind(a_this, a_name, &zwlr_layer_shell_v1_interface, std::min<uint32_t>(a_version, zwlr_layer_shell_v1_interface.version)));
})
{
	v_on_idle.push_back([&]
	{
		if (auto& on = v_object->f_fields()[/*on_idle*/0]) on();
	});
	v_on_selection.push_back([&]
	{
		if (auto& on = v_object->f_fields()[/*on_selection*/1]) on();
	});
	v_instance = this;
}

t_session::~t_session()
{
	while (v_next != this) v_next->f_dispose();
	if (v_layer) {
		zwlr_layer_shell_v1_destroy(v_layer);
		v_layer = NULL;
	}
	v_object->f_as<xemmaix::xade::t_client>().v_client = v_instance = nullptr;
}

void t_proxy::f_dispose()
{
	t_entry::f_dispose();
	v_object = nullptr;
}

void t_library::f_scan(t_scan a_scan)
{
	XEMMAIX__XADE__TYPES(XEMMAI__TYPE__SCAN)
}

std::vector<std::pair<t_root, t_rvalue>> t_library::f_define()
{
	t_type_of<t_client>::f_define(this);
	v_type_button.f_construct(f_type<t_object>()->f_derive(t_define{this}
		(L"LEFT"sv, BTN_LEFT)
		(L"RIGHT"sv, BTN_RIGHT)
		(L"MIDDLE"sv, BTN_MIDDLE)
		(L"SIDE"sv, BTN_SIDE)
		(L"EXTRA"sv, BTN_EXTRA)
		(L"FORWARD"sv, BTN_FORWARD)
		(L"BACK"sv, BTN_BACK)
		(L"TASK"sv, BTN_TASK)
	));
	v_type_key.f_construct(f_type<t_object>()->f_derive(t_define{this}
		(L"BACKSPACE"sv, XKB_KEY_BackSpace)
		(L"TAB"sv, XKB_KEY_Tab)
		(L"LINEFEED"sv, XKB_KEY_Linefeed)
		(L"CLEAR"sv, XKB_KEY_Clear)
		(L"RETURN"sv, XKB_KEY_Return)
		(L"PAUSE"sv, XKB_KEY_Pause)
		(L"SCROLL_LOCK"sv, XKB_KEY_Scroll_Lock)
		(L"SYS_REQ"sv, XKB_KEY_Sys_Req)
		(L"ESCAPE"sv, XKB_KEY_Escape)
		(L"DELETE"sv, XKB_KEY_Delete)
		(L"MULTI_KEY"sv, XKB_KEY_Multi_key)
		(L"CODEINPUT"sv, XKB_KEY_Codeinput)
		(L"SINGLECANDIDATE"sv, XKB_KEY_SingleCandidate)
		(L"MULTIPLECANDIDATE"sv, XKB_KEY_MultipleCandidate)
		(L"PREVIOUSCANDIDATE"sv, XKB_KEY_PreviousCandidate)
		(L"KANJI"sv, XKB_KEY_Kanji)
		(L"MUHENKAN"sv, XKB_KEY_Muhenkan)
		(L"HENKAN_MODE"sv, XKB_KEY_Henkan_Mode)
		(L"HENKAN"sv, XKB_KEY_Henkan)
		(L"ROMAJI"sv, XKB_KEY_Romaji)
		(L"HIRAGANA"sv, XKB_KEY_Hiragana)
		(L"KATAKANA"sv, XKB_KEY_Katakana)
		(L"HIRAGANA_KATAKANA"sv, XKB_KEY_Hiragana_Katakana)
		(L"ZENKAKU"sv, XKB_KEY_Zenkaku)
		(L"HANKAKU"sv, XKB_KEY_Hankaku)
		(L"ZENKAKU_HANKAKU"sv, XKB_KEY_Zenkaku_Hankaku)
		(L"TOUROKU"sv, XKB_KEY_Touroku)
		(L"MASSYO"sv, XKB_KEY_Massyo)
		(L"KANA_LOCK"sv, XKB_KEY_Kana_Lock)
		(L"KANA_SHIFT"sv, XKB_KEY_Kana_Shift)
		(L"EISU_SHIFT"sv, XKB_KEY_Eisu_Shift)
		(L"EISU_TOGGLE"sv, XKB_KEY_Eisu_toggle)
		(L"KANJI_BANGOU"sv, XKB_KEY_Kanji_Bangou)
		(L"ZEN_KOHO"sv, XKB_KEY_Zen_Koho)
		(L"MAE_KOHO"sv, XKB_KEY_Mae_Koho)
		(L"HOME"sv, XKB_KEY_Home)
		(L"LEFT"sv, XKB_KEY_Left)
		(L"UP"sv, XKB_KEY_Up)
		(L"RIGHT"sv, XKB_KEY_Right)
		(L"DOWN"sv, XKB_KEY_Down)
		(L"PRIOR"sv, XKB_KEY_Prior)
		(L"NEXT"sv, XKB_KEY_Next)
		(L"END"sv, XKB_KEY_End)
		(L"BEGIN"sv, XKB_KEY_Begin)
		(L"SELECT"sv, XKB_KEY_Select)
		(L"PRINT"sv, XKB_KEY_Print)
		(L"EXECUTE"sv, XKB_KEY_Execute)
		(L"INSERT"sv, XKB_KEY_Insert)
		(L"UNDO"sv, XKB_KEY_Undo)
		(L"REDO"sv, XKB_KEY_Redo)
		(L"MENU"sv, XKB_KEY_Menu)
		(L"FIND"sv, XKB_KEY_Find)
		(L"CANCEL"sv, XKB_KEY_Cancel)
		(L"HELP"sv, XKB_KEY_Help)
		(L"BREAK"sv, XKB_KEY_Break)
		(L"MODE_SWITCH"sv, XKB_KEY_Mode_switch)
		(L"SCRIPT_SWITCH"sv, XKB_KEY_script_switch)
		(L"NUM_LOCK"sv, XKB_KEY_Num_Lock)
		(L"KP_SPACE"sv, XKB_KEY_KP_Space)
		(L"KP_TAB"sv, XKB_KEY_KP_Tab)
		(L"KP_ENTER"sv, XKB_KEY_KP_Enter)
		(L"KP_F1"sv, XKB_KEY_KP_F1)
		(L"KP_F2"sv, XKB_KEY_KP_F2)
		(L"KP_F3"sv, XKB_KEY_KP_F3)
		(L"KP_F4"sv, XKB_KEY_KP_F4)
		(L"KP_HOME"sv, XKB_KEY_KP_Home)
		(L"KP_LEFT"sv, XKB_KEY_KP_Left)
		(L"KP_UP"sv, XKB_KEY_KP_Up)
		(L"KP_RIGHT"sv, XKB_KEY_KP_Right)
		(L"KP_DOWN"sv, XKB_KEY_KP_Down)
		(L"KP_PRIOR"sv, XKB_KEY_KP_Prior)
		(L"KP_NEXT"sv, XKB_KEY_KP_Next)
		(L"KP_END"sv, XKB_KEY_KP_End)
		(L"KP_BEGIN"sv, XKB_KEY_KP_Begin)
		(L"KP_INSERT"sv, XKB_KEY_KP_Insert)
		(L"KP_DELETE"sv, XKB_KEY_KP_Delete)
		(L"KP_EQUAL"sv, XKB_KEY_KP_Equal)
		(L"KP_MULTIPLY"sv, XKB_KEY_KP_Multiply)
		(L"KP_ADD"sv, XKB_KEY_KP_Add)
		(L"KP_SEPARATOR"sv, XKB_KEY_KP_Separator)
		(L"KP_SUBTRACT"sv, XKB_KEY_KP_Subtract)
		(L"KP_DECIMAL"sv, XKB_KEY_KP_Decimal)
		(L"KP_DIVIDE"sv, XKB_KEY_KP_Divide)
		(L"KP_0"sv, XKB_KEY_KP_0)
		(L"KP_1"sv, XKB_KEY_KP_1)
		(L"KP_2"sv, XKB_KEY_KP_2)
		(L"KP_3"sv, XKB_KEY_KP_3)
		(L"KP_4"sv, XKB_KEY_KP_4)
		(L"KP_5"sv, XKB_KEY_KP_5)
		(L"KP_6"sv, XKB_KEY_KP_6)
		(L"KP_7"sv, XKB_KEY_KP_7)
		(L"KP_8"sv, XKB_KEY_KP_8)
		(L"KP_9"sv, XKB_KEY_KP_9)
		(L"F1"sv, XKB_KEY_F1)
		(L"F2"sv, XKB_KEY_F2)
		(L"F3"sv, XKB_KEY_F3)
		(L"F4"sv, XKB_KEY_F4)
		(L"F5"sv, XKB_KEY_F5)
		(L"F6"sv, XKB_KEY_F6)
		(L"F7"sv, XKB_KEY_F7)
		(L"F8"sv, XKB_KEY_F8)
		(L"F9"sv, XKB_KEY_F9)
		(L"F10"sv, XKB_KEY_F10)
		(L"F11"sv, XKB_KEY_F11)
		(L"F12"sv, XKB_KEY_F12)
		(L"F13"sv, XKB_KEY_F13)
		(L"F14"sv, XKB_KEY_F14)
		(L"F15"sv, XKB_KEY_F15)
		(L"F16"sv, XKB_KEY_F16)
		(L"F17"sv, XKB_KEY_F17)
		(L"F18"sv, XKB_KEY_F18)
		(L"F19"sv, XKB_KEY_F19)
		(L"F20"sv, XKB_KEY_F20)
		(L"F21"sv, XKB_KEY_F21)
		(L"F22"sv, XKB_KEY_F22)
		(L"F23"sv, XKB_KEY_F23)
		(L"F24"sv, XKB_KEY_F24)
		(L"F25"sv, XKB_KEY_F25)
		(L"F26"sv, XKB_KEY_F26)
		(L"F27"sv, XKB_KEY_F27)
		(L"F28"sv, XKB_KEY_F28)
		(L"F29"sv, XKB_KEY_F29)
		(L"F30"sv, XKB_KEY_F30)
		(L"F31"sv, XKB_KEY_F31)
		(L"F32"sv, XKB_KEY_F32)
		(L"F33"sv, XKB_KEY_F33)
		(L"F34"sv, XKB_KEY_F34)
		(L"F35"sv, XKB_KEY_F35)
		(L"SHIFT_L"sv, XKB_KEY_Shift_L)
		(L"SHIFT_R"sv, XKB_KEY_Shift_R)
		(L"CONTROL_L"sv, XKB_KEY_Control_L)
		(L"CONTROL_R"sv, XKB_KEY_Control_R)
		(L"CAPS_LOCK"sv, XKB_KEY_Caps_Lock)
		(L"SHIFT_LOCK"sv, XKB_KEY_Shift_Lock)
		(L"META_L"sv, XKB_KEY_Meta_L)
		(L"META_R"sv, XKB_KEY_Meta_R)
		(L"ALT_L"sv, XKB_KEY_Alt_L)
		(L"ALT_R"sv, XKB_KEY_Alt_R)
		(L"SUPER_L"sv, XKB_KEY_Super_L)
		(L"SUPER_R"sv, XKB_KEY_Super_R)
		(L"HYPER_L"sv, XKB_KEY_Hyper_L)
		(L"HYPER_R"sv, XKB_KEY_Hyper_R)
		(L"SPACE"sv, XKB_KEY_space)
		(L"EXCLAM"sv, XKB_KEY_exclam)
		(L"QUOTEDBL"sv, XKB_KEY_quotedbl)
		(L"NUMBERSIGN"sv, XKB_KEY_numbersign)
		(L"DOLLAR"sv, XKB_KEY_dollar)
		(L"PERCENT"sv, XKB_KEY_percent)
		(L"AMPERSAND"sv, XKB_KEY_ampersand)
		(L"APOSTROPHE"sv, XKB_KEY_apostrophe)
		(L"QUOTERIGHT"sv, XKB_KEY_quoteright)
		(L"PARENLEFT"sv, XKB_KEY_parenleft)
		(L"PARENRIGHT"sv, XKB_KEY_parenright)
		(L"ASTERISK"sv, XKB_KEY_asterisk)
		(L"PLUS"sv, XKB_KEY_plus)
		(L"COMMA"sv, XKB_KEY_comma)
		(L"MINUS"sv, XKB_KEY_minus)
		(L"PERIOD"sv, XKB_KEY_period)
		(L"SLASH"sv, XKB_KEY_slash)
		(L"0"sv, XKB_KEY_0)
		(L"1"sv, XKB_KEY_1)
		(L"2"sv, XKB_KEY_2)
		(L"3"sv, XKB_KEY_3)
		(L"4"sv, XKB_KEY_4)
		(L"5"sv, XKB_KEY_5)
		(L"6"sv, XKB_KEY_6)
		(L"7"sv, XKB_KEY_7)
		(L"8"sv, XKB_KEY_8)
		(L"9"sv, XKB_KEY_9)
		(L"COLON"sv, XKB_KEY_colon)
		(L"SEMICOLON"sv, XKB_KEY_semicolon)
		(L"LESS"sv, XKB_KEY_less)
		(L"EQUAL"sv, XKB_KEY_equal)
		(L"GREATER"sv, XKB_KEY_greater)
		(L"QUESTION"sv, XKB_KEY_question)
		(L"AT"sv, XKB_KEY_at)
		(L"A"sv, XKB_KEY_A)
		(L"B"sv, XKB_KEY_B)
		(L"C"sv, XKB_KEY_C)
		(L"D"sv, XKB_KEY_D)
		(L"E"sv, XKB_KEY_E)
		(L"F"sv, XKB_KEY_F)
		(L"G"sv, XKB_KEY_G)
		(L"H"sv, XKB_KEY_H)
		(L"I"sv, XKB_KEY_I)
		(L"J"sv, XKB_KEY_J)
		(L"K"sv, XKB_KEY_K)
		(L"L"sv, XKB_KEY_L)
		(L"M"sv, XKB_KEY_M)
		(L"N"sv, XKB_KEY_N)
		(L"O"sv, XKB_KEY_O)
		(L"P"sv, XKB_KEY_P)
		(L"Q"sv, XKB_KEY_Q)
		(L"R"sv, XKB_KEY_R)
		(L"S"sv, XKB_KEY_S)
		(L"T"sv, XKB_KEY_T)
		(L"U"sv, XKB_KEY_U)
		(L"V"sv, XKB_KEY_V)
		(L"W"sv, XKB_KEY_W)
		(L"X"sv, XKB_KEY_X)
		(L"Y"sv, XKB_KEY_Y)
		(L"Z"sv, XKB_KEY_Z)
		(L"BRACKETLEFT"sv, XKB_KEY_bracketleft)
		(L"BACKSLASH"sv, XKB_KEY_backslash)
		(L"BRACKETRIGHT"sv, XKB_KEY_bracketright)
		(L"ASCIICIRCUM"sv, XKB_KEY_asciicircum)
		(L"UNDERSCORE"sv, XKB_KEY_underscore)
		(L"GRAVE"sv, XKB_KEY_grave)
		(L"QUOTELEFT"sv, XKB_KEY_quoteleft)
		(L"a"sv, XKB_KEY_a)
		(L"b"sv, XKB_KEY_b)
		(L"c"sv, XKB_KEY_c)
		(L"d"sv, XKB_KEY_d)
		(L"e"sv, XKB_KEY_e)
		(L"f"sv, XKB_KEY_f)
		(L"g"sv, XKB_KEY_g)
		(L"h"sv, XKB_KEY_h)
		(L"i"sv, XKB_KEY_i)
		(L"j"sv, XKB_KEY_j)
		(L"k"sv, XKB_KEY_k)
		(L"l"sv, XKB_KEY_l)
		(L"m"sv, XKB_KEY_m)
		(L"n"sv, XKB_KEY_n)
		(L"o"sv, XKB_KEY_o)
		(L"p"sv, XKB_KEY_p)
		(L"q"sv, XKB_KEY_q)
		(L"r"sv, XKB_KEY_r)
		(L"s"sv, XKB_KEY_s)
		(L"t"sv, XKB_KEY_t)
		(L"u"sv, XKB_KEY_u)
		(L"v"sv, XKB_KEY_v)
		(L"w"sv, XKB_KEY_w)
		(L"x"sv, XKB_KEY_x)
		(L"y"sv, XKB_KEY_y)
		(L"z"sv, XKB_KEY_z)
		(L"BRACELEFT"sv, XKB_KEY_braceleft)
		(L"BAR"sv, XKB_KEY_bar)
		(L"BRACERIGHT"sv, XKB_KEY_braceright)
		(L"ASCIITILDE"sv, XKB_KEY_asciitilde)
	));
	t_type_of<t_proxy>::f_define(this);
	t_type_of<t_surface>::f_define(this);
	t_type_of<t_frame>::f_define(this);
	t_define{this}.f_derive<t_cursor, t_proxy>();
	t_type_of<t_input>::f_define(this);
	t_type_of<t_layered>::f_define(this);
	return t_define{this}
	(L"Client"sv, static_cast<t_object*>(v_type_client))
	(L"Button"sv, static_cast<t_object*>(v_type_button))
	(L"PointerAxis"sv, t_type_of<wl_pointer_axis>::f_define(this))
	(L"Key"sv, static_cast<t_object*>(v_type_key))
	(L"Proxy"sv, static_cast<t_object*>(v_type_proxy))
	(L"Surface"sv, static_cast<t_object*>(v_type_surface))
	(L"FrameState"sv, t_type_of<xdg_toplevel_state>::f_define(this))
	(L"FrameWMCapabilities"sv, t_type_of<xdg_toplevel_wm_capabilities>::f_define(this))
	(L"FrameResizeEdge"sv, t_type_of<xdg_toplevel_resize_edge>::f_define(this))
	(L"Frame"sv, static_cast<t_object*>(v_type_frame))
	(L"Cursor"sv, static_cast<t_object*>(v_type_cursor))
	(L"Input"sv, static_cast<t_object*>(v_type_input))
	(L"Layer"sv, t_type_of<zwlr_layer_shell_v1_layer>::f_define(this))
	(L"Anchor"sv, t_type_of<zwlr_layer_surface_v1_anchor>::f_define(this))
	(L"KeyboardInteractivity"sv, t_type_of<zwlr_layer_surface_v1_keyboard_interactivity>::f_define(this))
	(L"Layered"sv, static_cast<t_object*>(v_type_layered))
	(L"main"sv, t_static<void(*)(t_library*, const t_pvalue&), [](auto a_library, auto a_callable)
	{
		t_session session;
		session.v_object = f_new<t_client>(a_library);
		a_callable();
	}>())
	(L"client"sv, t_static<t_object*(*)(), []
	{
		return static_cast<t_object*>(t_session::f_instance()->v_object);
	}>())
	;
}

}

namespace xemmai
{

void t_type_of<xemmaix::xade::t_proxy>::f_define(t_library* a_library)
{
	using xemmaix::xade::t_proxy;
	t_define{a_library}
	(L"dispose"sv, t_member<void(t_proxy::*)(), &t_proxy::f_dispose>())
	.f_derive<t_proxy, t_object>();
}

}

XEMMAI__MODULE__FACTORY(xemmai::t_library::t_handle* a_handle)
{
	return xemmai::f_new<xemmaix::xade::t_library>(a_handle);
}
