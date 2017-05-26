#include <nm/Config.hpp>
#include <nm/GuileInterpreter.hpp>
#include <nm/ImageSaver.hpp>
#include <nm/Square.hpp>

int num_or(SCM number, int alt)
{
	if (SCM_UNBNDP(number))
		return alt;
	return scm_to_int(number);
}

namespace nm
{
	GuileInterpreter::GuileInterpreter()
	{
		scm_init_guile();
		scm_c_primitive_load("netmine_fns.scm");

		SCM run_code_sym = scm_c_lookup("run-with-err-handler");
		run_code_fn = scm_variable_ref(run_code_sym);

		scm_c_define_gsubr("nm-config-get", 1, 0, 0, (void*)GuileInterpreter::netmine_config_get);
		scm_c_define_gsubr("nm-config-set", 2, 0, 0, (void*)GuileInterpreter::netmine_config_set);

		scm_c_define_gsubr("nm-redraw", 0, 0, 0, (void*)GuileInterpreter::netmine_redraw);
		scm_c_define_gsubr("nm-flag", 0, 0, 0, (void*)GuileInterpreter::netmine_flag);
		scm_c_define_gsubr("nm-center", 0, 2, 0, (void*)GuileInterpreter::netmine_center);
		scm_c_define_gsubr("nm-up", 0, 1, 0, (void*)GuileInterpreter::netmine_up);
		scm_c_define_gsubr("nm-down", 0, 1, 0, (void*)GuileInterpreter::netmine_down);
		scm_c_define_gsubr("nm-left", 0, 1, 0, (void*)GuileInterpreter::netmine_left);
		scm_c_define_gsubr("nm-right", 0, 1, 0, (void*)GuileInterpreter::netmine_right);
		scm_c_define_gsubr("nm-square-data", 0, 0, 0, (void*)GuileInterpreter::netmine_square);
		scm_c_define_gsubr("nm-save-png", 1, 0, 0, (void*)GuileInterpreter::netmine_save_png);
	}

	SCM GuileInterpreter::key_n_times(int key, SCM ntimes)
	{
		for (int i = 0; i < num_or(ntimes, 1); ++i)
		{
			GLOBAL_GUI->current_view->handle_input(key);
		}
		return SCM_UNSPECIFIED;
	}

	SCM GuileInterpreter::netmine_config_get(SCM key)
	{
		std::string str_key = scm_to_utf8_string(key);
		auto object = nm::config[str_key];
		
		SCM output = SCM_UNSPECIFIED;
		if (object.is_number_float())
		{
			output = scm_from_double(object.get<double>());
		} else if (object.is_number())
		{
			output = scm_from_long_long(object.get<long long>());
		} else if (object.is_string())
		{
			std::string str = object.get<std::string>();
			output = scm_from_utf8_string(str.c_str());
		}
		
		return output;
	}
	
	SCM GuileInterpreter::netmine_config_set(SCM key, SCM value)
	{
		std::string str_key = scm_to_utf8_string(key);
		
		return SCM_UNSPECIFIED;
	}

	SCM GuileInterpreter::netmine_redraw()
	{
		GLOBAL_GUI->draw();
		return SCM_UNSPECIFIED;
	}

	SCM GuileInterpreter::netmine_flag()
	{
		// This is super hacky, yes, but it works. Start complaining
		// when it doesn't.
		GLOBAL_GUI->current_view->handle_input('f');
		return SCM_UNSPECIFIED;
	}

	SCM GuileInterpreter::netmine_center(SCM x, SCM y)
	{
		int global_x = SCM_UNBNDP(x) ? (GLOBAL_GUI->self_cursor.x + GLOBAL_GUI->self_cursor.offset_x)
			: scm_to_int(x);

		int global_y = SCM_UNBNDP(x) ? (GLOBAL_GUI->self_cursor.y + GLOBAL_GUI->self_cursor.offset_y)
			: scm_to_int(y);

		GLOBAL_GUI->boardview.center_cursor(global_x, global_y);
		return SCM_UNSPECIFIED;
	}

	SCM GuileInterpreter::netmine_up(SCM num)
	{
		key_n_times(KEY_UP, num);
		return SCM_UNSPECIFIED;
	}

	SCM GuileInterpreter::netmine_down(SCM num)
	{
		key_n_times(KEY_DOWN, num);
		return SCM_UNSPECIFIED;
	}

	SCM GuileInterpreter::netmine_left(SCM num)
	{
		key_n_times(KEY_LEFT, num);
		return SCM_UNSPECIFIED;
	}

	SCM GuileInterpreter::netmine_right(SCM num)
	{
		key_n_times(KEY_RIGHT, num);
		return SCM_UNSPECIFIED;
	}

	SCM GuileInterpreter::netmine_square()
	{
		const Coordinates& cursor_pos = GLOBAL_GUI->self_cursor.to_global();
		Square& sq = GLOBAL_GUI->squareSource.get(cursor_pos);

		SCM squareState, number;
		if (sq.state == SquareState::CLOSED)
		{
			squareState = scm_from_utf8_symbol("closed");
		} else if (sq.state == SquareState::OPENED) {
			squareState = scm_from_utf8_symbol("opened");
		} else {
			squareState = scm_from_utf8_symbol("flagged");
		}

		if (sq.state == SquareState::OPENED)
		{
			number = scm_from_int(sq.number);
		} else {
			number = SCM_UNSPECIFIED;
		}
		
		SCM square_map = SCM_EOL;
		square_map = scm_assv_set_x(square_map, scm_from_utf8_symbol("state"), squareState);
		square_map = scm_assv_set_x(square_map, scm_from_utf8_symbol("number"), number);
		return square_map;
	}

	SCM GuileInterpreter::netmine_save_png(SCM filename)
	{
		const char* filename_cc = scm_to_utf8_string(filename);

		ImageSaver saver(GLOBAL_GUI->squareSource);
		saver.save(filename_cc);

		return scm_from_utf8_string("Image saved successfully.");
	}

	void GuileInterpreter::run_command(std::string command)
	{
		scm_dynwind_begin(static_cast<scm_t_dynwind_flags>(0));

		SCM retval = scm_call_1(run_code_fn, scm_from_utf8_string(command.c_str()));

		const char *output = scm_to_utf8_string(retval);

		GLOBAL_GUI->command << Erase << Move({0, 0}) << output << Refresh;

		scm_dynwind_free((void*)output);
		scm_dynwind_end();
	}
}
