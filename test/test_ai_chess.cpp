#include <iostream>
#include <algorithm>
#include <limits>
#include <memory>
#include <string>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <random>
#include <cctype>

#include "../chess_engine/chess.hpp"
#include "../chess_engine/chess_bot.hpp"
#include "../client/board_display.hpp"

// Constants
const int MAX_DEPTH = 4;

int main()
{
    std::srand(static_cast<unsigned>(std::time(0)));
    const char *initFen = chess::constants::STARTPOS;
    chess::Board board(initFen);
    chess::GameResult result = chess::GameResult::NONE;
    chess::GameResultReason reason = chess::GameResultReason::NONE;
    chess::Color current_turn = board.sideToMove();
    bool isWhiteTurn = (current_turn == chess::Color::WHITE);

    // AI màu đen
    chess::Color aiColor = chess::Color::BLACK;

    while (result == chess::GameResult::NONE)
    {
        std::cout << std::endl;
        board_display::printBoard(board.getFen(), isWhiteTurn);

        chess::Move move;
        if (isWhiteTurn)
        {
            chess::Movelist moves;
            chess::movegen::legalmoves(moves, board);
            if (moves.empty())
                break;
            move = moves[std::rand() % moves.size()];
            std::cout << "White plays: " << chess::uci::moveToUci(move) << std::endl;
        }
        else
        {
            move = ChessBot::getInstance().findBestMove(board.getFen(), aiColor);
            if (move == chess::Move::NO_MOVE)
            {
                std::cout << "Black (AI) has no legal moves." << std::endl;
                break;
            }
            std::cout << "Black (AI) plays: " << chess::uci::moveToUci(move) << std::endl;
        }

        board.makeMove(move);
        std::tie(reason, result) = board.isGameOver();
        isWhiteTurn = !isWhiteTurn;
    }

    board_display::printBoard(board.getFen(), isWhiteTurn);

    std::cout << "Game over! Result: ";
    switch (result)
    {
    case chess::GameResult::LOSE:
        std::cout << (isWhiteTurn ? "Black" : "White") << " wins! Due to " << static_cast<int>(reason) << std::endl;
        break;
    case chess::GameResult::DRAW:
        std::cout << "Draw! Due to " << static_cast<int>(reason) << std::endl;
        break;
    default:
        std::cout << "None" << std::endl;
        break;
    }

    return 0;
}
