#include <limits>
#include <fstream>
#include <string.h>
#include <libunwind.h>
#include <stdio.h>
#include <cxxabi.h>
#include <dlfcn.h>
#include <typeinfo>
#include <execinfo.h>
#include <string>
#include <limits.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#define PACKAGE 1
#define PACKAGE_VERSION 1

#include <bfd.h>

static bfd* current_executable = 0;
static asymbol** syms = 0;
static asection* text = 0;

std::string __latest_backtrace;

static std::string demangle(const char* mangled)
{
#ifdef __GNUG__
	int status = -1;
	char *demangled = nullptr;

	demangled = abi::__cxa_demangle(mangled, 0, 0, &status);
	return std::string(demangled);
#else
	return std::string(mangled);
#endif
}

static void resolve(unw_word_t addr, const char** file, unsigned int* line, const char** func)
{
	static bool can_give_lineinfo = true;

	if (!can_give_lineinfo)
	{
		return;
	}

	if (!current_executable)
	{
		static char exec_name[PATH_MAX];
		int len = readlink("/proc/self/exe", exec_name, PATH_MAX);
		if (len < 0)
		{
			std::cerr << "Unable to find the current executable, so backtrace won't display line information." << std::endl;
			can_give_lineinfo = false;
			return;
		}

		bfd_init();
		current_executable = bfd_openr(exec_name, 0);

		if (!current_executable)
		{
			std::cerr << "Unable to initialize libbfd, so backtrace won't display line information." << std::endl;
			can_give_lineinfo = false;
			return;
		}

		/* this is a required check. no idea why. */
		bfd_check_format(current_executable, bfd_object);

		unsigned storage_needed = bfd_get_symtab_upper_bound(current_executable);
		syms = (asymbol**) malloc(storage_needed);
		unsigned cSymbols = bfd_canonicalize_symtab(current_executable, syms);

		/* Get the text section so we can 'un-relocate' symbols. */
		text = bfd_get_section_by_name(current_executable, ".text");
	}

	unsigned offset = addr - text->vma;

	if (offset > 0)
	{
		/* Address is in the current executable! Great! */
		int success = bfd_find_nearest_line(current_executable, text, syms, offset, file, func, line);
		if (*file && success)
		{
			return;
		}
	}
}

inline static void seek_to_line(std::ifstream& file, unsigned line)
{
	file.seekg(std::ios::beg);
	for (int i = 0; i < line - 1; ++i)
	{
		file.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	}
}

static void luw_backtrace()
{
	unw_cursor_t cursor;
	unw_context_t context;

	unw_getcontext(&context);
	unw_init_local(&cursor, &context);

	std::stringstream output;

	output << "Traceback (most recent call first):" << std::endl;

	while (unw_step(&cursor) > 0)
	{
		unw_word_t pc;

		unw_get_reg(&cursor, UNW_REG_IP, &pc);

		if (pc == 0)
			break;

		const char *file = NULL;
		const char *func = NULL;
		unsigned line = 0;
		resolve(pc, &file, &line, &func);

		if (file != NULL)
		{
			if (strcmp("__cxa_throw", func) == 0) continue;
			if (strncmp(func, "_Z", 2) == 0)
			{
				// This is a C++ mangled name (as per the IA-64 spec)
				output << "  0x" << std::hex << pc << std::dec << " " << demangle(func) << " (" << file << ":" << line << ")" << std::endl;
			} else {
				output << "  0x" << std::hex << pc << std::dec << " " << func << " (" << file << ":" << line << ")" << std::endl;
			}

			// Try to load the file and print the offending line
			std::ifstream f;
			f.open(file);
			if (!f.is_open())
			{
				output << "    <unable to find file>" << std::endl;
			} else
			{
				seek_to_line(f, line);
				std::string line;
				std::getline(f, line);
				output << "    " << line << std::endl;
			}
		} else {
			output << "  <unknown> (<unknown file>:<unknown line>)" << std::endl;
		}
	}
	output << std::endl;

	__latest_backtrace = output.str();
}

extern "C"
{
#if __clang__
#define __cxa_info_type std::type_info*
#else
#define __cxa_info_type void*
#endif
	void __cxa_throw(void *except, __cxa_info_type info, void (*dest)(void *))
	{
		auto rethrow = (void (*)(void*, __cxa_info_type, void(*)(void*)))dlsym(RTLD_NEXT, "__cxa_throw");
		luw_backtrace();
		rethrow(except, info, dest);
		__builtin_unreachable(); // rethrow is noreturn
	}
}
