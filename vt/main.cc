#include "window.h"
#include <sys/wait.h>
#include <pty.h>
#include <signal.h>
#ifdef XADE__UTMP
#include <cstring>
#include <pwd.h>
#include <utmp.h>
#endif
#include <xade/layered.h>
#include <include/core/SkFontMgr.h>
#include <include/ports/SkFontMgr_fontconfig.h>

namespace
{

#ifdef XADE__UTMP

uid_t v_euid = geteuid();
gid_t v_egid = getegid();

void f_log(struct utmp& a_utmp)
{
	seteuid(v_euid);
	setegid(v_egid);
	setutent();
	pututline(&a_utmp);
	endutent();
	updwtmp("/var/log/wtmp", &a_utmp);
	seteuid(getuid());
	setegid(getgid());
}

void f_login(t_application& a_application, struct utmp& a_utmp, int a_master)
{
	a_utmp.ut_type = UT_UNKNOWN;
	a_utmp.ut_pid = getpid();
	const char* line = ptsname(a_master);
	if (!line || std::strncmp(line, "/dev/", 5) != 0) return;
	line += 5;
	std::strncpy(a_utmp.ut_line, line, UT_LINESIZE);
	if (std::strncmp(line, "pts/", 4) == 0) {
		a_utmp.ut_id[0] = 'p';
		std::strncpy(a_utmp.ut_id + 1, line + 4, 3);
	} else if (std::strncmp(line, "tty", 3) == 0) {
		std::strncpy(a_utmp.ut_id, line + 3, 4);
	} else {
		return;
	}
	uid_t uid = getuid();
	struct passwd* passwd = getpwuid(uid);
	if (passwd)
		std::strncpy(a_utmp.ut_user, passwd->pw_name, UT_NAMESIZE);
	else
		std::snprintf(a_utmp.ut_user, UT_NAMESIZE, "%d", uid);
	std::strncpy(a_utmp.ut_host, DisplayString(a_application.f_x11_display()), UT_HOSTSIZE);
	a_utmp.ut_time = time(NULL);
	a_utmp.ut_addr = 0;
	a_utmp.ut_type = USER_PROCESS;
	f_log(a_utmp);
}

void f_logout(struct utmp& a_utmp)
{
	if (a_utmp.ut_type != USER_PROCESS) return;
	a_utmp.ut_type = DEAD_PROCESS;
	std::memset(a_utmp.ut_line, 0, UT_LINESIZE);
	std::memset(a_utmp.ut_user, 0, UT_NAMESIZE);
	std::memset(a_utmp.ut_host, 0, UT_HOSTSIZE);
	a_utmp.ut_time = 0;
	f_log(a_utmp);
}

#endif

int f_main(int argc, char* argv[], int a_master)
{
	struct sigaction action;
	action.sa_flags = 0;
	action.sa_handler = [](auto)
	{
		suisha::f_loop().f_exit();
	};
	sigaction(SIGTERM, &action, NULL);
	sigaction(SIGINT, &action, NULL);
	sigaction(SIGQUIT, &action, NULL);
	sigaction(SIGHUP, &action, NULL);
	action.sa_handler = [](auto)
	{
		wait(NULL);
		suisha::f_loop().f_exit();
	};
	struct sigaction old_sigchld;
	sigaction(SIGCHLD, &action, &old_sigchld);
	std::setlocale(LC_ALL, "");
#ifdef XADE__UTMP
	struct utmp utmp;
	f_login(application, utmp, a_master);
#endif
	{
		using namespace std::literals;
		auto fm = SkFontMgr_New_FontConfig(nullptr);
		auto typeface = fm->matchFamilyStyle(nullptr, {});
		auto symbols = fm->makeFromFile((argv[0] + ".symbols"s).c_str());
		if (!symbols) symbols = typeface;
		suisha::t_loop loop;
		zwlr_layer_shell_v1* shell = NULL;
		t_client client([&](auto a_this, auto a_name, auto a_interface, auto a_version)
		{
			if (std::strcmp(a_interface, zwlr_layer_shell_v1_interface.name) == 0) shell = static_cast<zwlr_layer_shell_v1*>(wl_registry_bind(a_this, a_name, &zwlr_layer_shell_v1_interface, std::min<uint32_t>(a_version, zwlr_layer_shell_v1_interface.version)));
		});
		t_theme theme({symbols, 20}, 8);
		SkGlyphID bar_glyphs[2];
		{
			SkUnichar cs[] = {L'\ue5c7', L'\ue5c5'};
			theme.v_font.unicharsToGlyphs(cs, 2, bar_glyphs);
		}
		size_t log = 192;
		size_t columns = 80;
		size_t rows = 24;
		auto run = [&](auto& frame, auto& on_measure, auto& on_map, auto measure_frame, auto draw_frame, auto prepare)
		{
			if (shell) zwlr_layer_shell_v1_destroy(shell);
			t_window::t_host host{
				frame,
				on_measure,
				on_map,
				frame.v_on_unmap,
				[&]
				{
					frame.f_swap_buffers();
				},
				measure_frame,
				draw_frame,
				theme.v_color_background,
				theme.v_unit,
				[&](auto& a_canvas, auto a_index, auto a_x, auto a_y, auto a_state)
				{
					theme.f_draw(a_canvas, bar_glyphs[a_index], a_x, a_y, a_state);
				},
				theme.v_cursor_text,
				theme.v_cursor_arrow
			};
			t_window window(log, columns, rows, a_master, {typeface, 14}, host);
			prepare();
			loop.f_run();
		};
		bool dock = false;
		int margins[] = {-1, -1, -1, -1};
		auto from_chars = [](std::string_view s, auto& i)
		{
			std::from_chars(s.data(), s.data() + s.size(), i);
		};
		for (size_t i = 1; i < argc; ++i)
			if (std::strncmp(argv[i], "--log=", 6) == 0) {
				from_chars(argv[i] + 6, log);
			} else if (std::strncmp(argv[i], "--size=", 7) == 0) {
				std::string_view s = argv[i] + 7;
				if (auto i = s.find('x'); i != s.npos) {
					from_chars(s.substr(0, i), columns);
					from_chars(s.substr(i + 1), rows);
				}
			} else if (std::strncmp(argv[i], "--dock=", 7) == 0) {
				dock = true;
				std::string_view s = argv[i] + 7;
				for (size_t i = 0, m = 0;;) {
					auto j = s.find(',', i);
					if (j == s.npos) break;
					if (j > i) from_chars(s.substr(i, j - i), margins[m]);
					i = j + 1;
					if (++m < 3) continue;
					if (i < s.size()) from_chars(s.substr(i), margins[m]);
					break;
				}
			}
		if (dock) {
			if (!shell) throw std::runtime_error("layer shell");
			t_layered frame(shell, NULL, ZWLR_LAYER_SHELL_V1_LAYER_BOTTOM, "", false);
			std::function<void(int32_t&, int32_t&)> on_measure;
			frame.v_on_measure = [&](auto a_width, auto a_height)
			{
				int32_t width = a_width;
				int32_t height = a_height;
				on_measure(width, height);
				a_width = width;
				a_height = height;
			};
			std::function<void(int32_t, int32_t)> on_map;
			frame.v_on_map = [&](auto a_width, auto a_height)
			{
				on_map(a_width, a_height);
			};
			frame.v_on_closed = [&]
			{
				loop.f_exit();
			};
			run(frame, on_measure, on_map, [&]
			{
				return SkIRect::MakeEmpty();
			}, [&](auto& a_canvas, auto a_width, auto a_height)
			{
			}, [&]
			{
				int32_t width = 0;
				int32_t height = 0;
				on_measure(width, height);
				zwlr_layer_surface_v1_set_size(frame, width, height);
				int anchor = 0;
				if (margins[0] >= 0) anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP;
				if (margins[1] >= 0) anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
				if (margins[2] >= 0) anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
				if (margins[3] >= 0) anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT;
				zwlr_layer_surface_v1_set_anchor(frame, anchor);
				zwlr_layer_surface_v1_set_margin(frame, std::max(margins[0], 0), std::max(margins[1], 0), std::max(margins[2], 0), std::max(margins[3], 0));
				zwlr_layer_surface_v1_set_keyboard_interactivity(frame, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND);
				wl_surface_commit(frame);
			});
		} else {
			t_frame frame(false);
			t_decoration decoration(theme);
			std::function<void(int32_t, int32_t)> on_map;
			frame.v_on_map = [&](auto a_width, auto a_height)
			{
				on_map(a_width, a_height);
				decoration.f_invalidate();
			};
			frame.v_on_close = [&]
			{
				loop.f_exit();
			};
			run(frame, frame.v_on_measure, on_map, [&]
			{
				return theme.f_frame(frame);
			}, [&](auto& a_canvas, auto a_width, auto a_height)
			{
				decoration.f_draw(frame, a_canvas, a_width, a_height);
			}, [&]
			{
				decoration.f_hook(frame);
				xdg_toplevel_set_title(frame, argv[0]);
			});
		}
	}
	sigaction(SIGCHLD, &old_sigchld, NULL);
#ifdef XADE__UTMP
	f_logout(utmp);
#endif
	return EXIT_SUCCESS;
}

}

int main(int argc, char* argv[])
{
#ifdef XADE__UTMP
	seteuid(getuid());
	setegid(getgid());
#endif
	int master;
	switch (forkpty(&master, NULL, NULL, NULL)) {
	case -1:
		return EXIT_FAILURE;
	case 0:
		{
			struct sigaction action;
			action.sa_flags = 0;
			action.sa_handler = SIG_DFL;
			sigaction(SIGINT, &action, NULL);
			sigaction(SIGQUIT, &action, NULL);
			const char* shell = std::getenv("SHELL");
			if (shell == NULL) shell = "/bin/sh";
			setenv("TERM", "xterm", 1);
			return execlp(shell, shell, NULL);
		}
	default:
		{
			int n = f_main(argc, argv, master);
			close(master);
			return n;
		}
	}
}
