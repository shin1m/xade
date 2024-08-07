#include "window.h"
#include <sys/wait.h>
#include <pty.h>
#include <signal.h>
#ifdef XADE__UTMP
#include <cstring>
#include <pwd.h>
#include <utmp.h>
#endif
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
		t_client client;
		t_theme theme({symbols, 20}, 8);
		t_window window(192, 80, 24, a_master, {typeface, 16}, theme, argv[0]);
		loop.f_run();
		sigaction(SIGCHLD, &old_sigchld, NULL);
	}
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
