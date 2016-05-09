#include "Board.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
    nm::Board board;
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 32; j++) {
            nm::Square& square = board.get({i, j});
            if (square.is_mine)
                std::cout << " * ";
            else
                std::cout << "   ";
        }
        std::cout << std::endl;
    }
}