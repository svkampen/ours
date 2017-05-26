#ifndef _NM_GUILEINTERPRETER_
#define _NM_GUILEINTERPRETER_

#include <ncurses.h>
#include <string>

namespace nm { class Gui; }
extern nm::Gui *GLOBAL_GUI;

#include <libguile.h>

namespace nm
{
	class GuileInterpreter
	{
		public:
			GuileInterpreter();

			SCM run_code_fn;

			void run_command(std::string command);
			static SCM netmine_config_get(SCM key);
			static SCM netmine_config_set(SCM key, SCM value);

			static SCM netmine_redraw();
			static SCM netmine_flag();
			static SCM netmine_center(SCM x, SCM y);
			static SCM netmine_up(SCM num);
			static SCM netmine_down(SCM num);
			static SCM netmine_left(SCM num);
			static SCM netmine_right(SCM num);
			static SCM netmine_square();
			static SCM netmine_save_png(SCM filename);
		private:
			static SCM key_n_times(int key, SCM ntimes);
	};
}

#include <nm/Gui.hpp>

#endif
