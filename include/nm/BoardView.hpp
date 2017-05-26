#ifndef _NM_BOARD_VIEW_
#define _NM_BOARD_VIEW_

#include <nm/View.hpp>

namespace nm
{
	/**
	 * The standard minesweeper board view.
	 *
	 * The board is divided into 'squares' that are three columns wide, one line high.
	 * Cursors are represented by "[ ]"-blocks around the square. This class provides
	 * input handling for this board and draws it.
	 */
	class BoardView : public View
	{
		public:

		/**
		 * The standard BoardView constructor.
		 *
		 * @param cursorData The client's own cursor data.
		 * @param main The main window.
		 * @param sidebar The sidebar window.
		 * @param open The event to fire when a square is opened.
		 * @param flag The event to fire when a square is flagged.
		 * @param move The event to fire when the cursor is moved.
		 */
		BoardView(CursorData& cursorData, Window& main, Window& sidebar, const SquareEvent& open, const SquareEvent& flag, const SquareEvent& move);

		const SquareEvent& ev_square_open;
		const SquareEvent& ev_square_flag;
		const SquareEvent& ev_cursor_move;

		/**
		 * Describes whether a chunk border is shown or not.
		 */
		bool border_enabled = false;

		/**
		 * Draws flagged squares.
		 * 
		 * @param x The window's x-coordinate of the square.
		 * @param y The window's y-coordinate of the square.
		 * @param square The square in question.
		 */
		void draw_flag_square(int x, int y,
				Square& square);

		/**
		 * Draws opened squares.
		 * 
		 * @param x The window's x-coordinate of the square.
		 * @param y The window's y-coordinate of the square.
		 * @param square The square in question.
		 */
		void draw_open_square(int x, int y,
				Square& square);

		/**
		 * Draws closed squares.
		 * 
		 * @param x The window's x-coordinate of the square.
		 * @param y The window's y-coordinate of the square.
		 * @param square The square in question.
		 */
		void draw_closed_square(int x, int y,
				Square& square);

		/**
		 * Draw other players' cursors.
		 *
		 * @param cursors The cursor map.
		 */
		void draw_cursors(CursorMap& cursors);

		/**
		 * Draws the sidebar.
		 *
		 * @param squareSource 
		 * @param cursors The other players' cursors.
		 */
		void draw_sidebar(ChunkSquareSource& squareSource, CursorMap& cursors);

		/**
		 * Draw the main window.
		 */
		void draw_main(ChunkSquareSource& squareSource, CursorMap& others);

		/**
		 * Handle this view's input.
		 *
		 * @param input_character The inputted character.
		 * @return An enum result describing the next action in the input chain.
		 */
		HandlerResult handle_input(int input_character);


		/**
		 * Draw the cursor.
		 *
		 * @param squareSource The squaresource.
		 * @param others The cursor map.
		 */
		void draw_cursor(ChunkSquareSource& squareSource, CursorMap& others);
		void center_cursor();
		void center_cursor(int global_x, int global_y);
	};
}

#endif //_NM_BOARD_VIEW_
