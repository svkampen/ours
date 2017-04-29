#ifndef CURSESSTREAM_H
#define CURSESSTREAM_H

#include "CursesStreamBuf.hpp"
#include <ostream>
#include <ncurses.h>
#include "Vec.hpp"
#include <utility>

#ifndef NCURSES_NOMACROS
#define NCURSES_NOMACROS
#endif

namespace nm {
	class CursesStream : /*virtual*/ public CursesStreamBuf, public std::wostream
	{
		public:
			CursesStream(WINDOW *win);
			virtual ~CursesStream();

			template<typename T>
				CursesStream& operator<< (T&& val)
				{
					return static_cast<CursesStream&>(*static_cast<std::wostream*>(this) << std::forward<T>(val));
				}

			operator WINDOW *();
		protected:
			WINDOW *win;
	};

		template<>
	inline CursesStream& CursesStream::operator<< (CursesStream& (&func)(CursesStream&))
	{
		return func(*this);
	}

	class Move
	{
		public:
			Move(const Vec &new_pos) : pos(new_pos)
		{

		}

			friend CursesStream& operator<< (CursesStream &out, const Move &m)
			{
				wmove(out, m.pos.y, m.pos.x);
				return out;
			}

			friend CursesStream& operator<< (CursesStream &out, Move &&m)
			{
				wmove(out, m.pos.y, m.pos.x);
				return out;
			}
		private:
			Vec pos;
	};

	inline CursesStream& Erase(CursesStream &out) {
		werase(out);
		return out;
	}

	inline CursesStream& Refresh(CursesStream &out)
	{
		wrefresh(out);
		return out;
	}

	inline CursesStream& ClrToEol(CursesStream &out)
	{
		int x, y;
		getyx(out, y, x);
		if (x + 1 < getmaxx(out)) // don't clear of blocked
			wclrtoeol(out);
		return out;
	}

	inline CursesStream& ClrToBot(CursesStream &out)
	{
		int x, y, maxx, maxy;
		getyx(out, y, x);
		getmaxyx(out, maxy, maxx);
		if (y + 1 < maxy || (y + 1 == maxy && x + 1 < maxx)) // don't clear of blocked
			wclrtobot(out);
		return out;
	}

	class AttrOn
	{
		public:
			AttrOn(const chtype &new_ch) : ch(new_ch)
		{

		}

			friend CursesStream& operator<< (CursesStream &out, const AttrOn &at)
			{
				wattron(out, at.ch);
				return out;
			}

			friend CursesStream& operator<< (CursesStream &out, AttrOn &&at)
			{
				wattron(out, at.ch);
				return out;
			}
		private:
			chtype ch;
	};

	class AttrOff
	{
		public:
			AttrOff(const chtype &new_ch) : ch(new_ch)
		{

		}

			friend CursesStream& operator<< (CursesStream &out, const AttrOff &at)
			{
				wattroff(out, at.ch);
				return out;
			}

			friend CursesStream& operator<< (CursesStream &out, AttrOff &&at)
			{
				wattroff(out, at.ch);
				return out;
			}
		private:
			chtype ch;
	};

	class AttrSet
	{
		public:
			AttrSet(const chtype &new_ch) : ch(new_ch)
		{

		}

			AttrOn operator+ () const
			{
				return AttrOn(ch);
			}

			AttrOff operator- () const
			{
				return AttrOff(ch);
			}

			friend CursesStream& operator<< (CursesStream &out, const AttrSet &at)
			{
				attrset(at.ch);
				return out;
			}

			friend CursesStream& operator<< (CursesStream &out, AttrSet &&at)
			{
				attrset(at.ch);
				return out;
			}
		private:
			chtype ch;
	};

	extern AttrSet Bold, Normal;
}


#endif // CURSESSTREAM_H
