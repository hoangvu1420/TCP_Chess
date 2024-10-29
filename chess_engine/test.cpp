#include <iostream>
#include <string>
#include <cctype>
#include <cstring>
#include "chess.hpp"

using namespace chess;

// ANSI color codes
const std::string RESET = "\033[0m";
const std::string WHITE_COLOR = "\033[96m"; // Cyan
const std::string BLACK_COLOR = "\033[31m"; // Red
const std::string LABEL_COLOR = "\033[34m"; // Orange

#define WHITE_SQUARE "\033[37m█\033[0m"
#define BLACK_SQUARE "\033[90m█\033[0m"
#define SPACE " "

// Function to determine if a piece is white
bool isWhitePiece(const Piece &piece)
{
    return piece.color() == Color::WHITE;
}

// Function to determine if a piece is black
bool isBlackPiece(const Piece &piece)
{
    return piece.color() == Color::BLACK;
}

void printLine(int iLine, const char *pchColor1, const char *pchColor2, const Board &board, bool flip)
{
    const int CELL = 6; // Number of characters per cell vertically

    for (int subLine = 0; subLine < CELL / 2; subLine++)
    {
        for (int iPair = 0; iPair < 4; iPair++)
        {
            // First square in the pair
            bool hasPiece1 = false; // Flag for the first square
            for (int subColumn = 0; subColumn < CELL; subColumn++)
            {
                if (subLine == 1 && subColumn == 2)
                {
                    // Determine the square index based on the board orientation (flipped or not)
                    Square sq1 = flip ? Square((7 - iLine) * 8 + (7 - iPair * 2)) : Square(iLine * 8 + iPair * 2);
                    Piece piece1 = board.at(sq1);

                    if (piece1 != Piece::NONE)
                    {
                        // Apply color based on piece color and print the Unicode symbol
                        if (isWhitePiece(piece1))
                            std::cout << WHITE_COLOR << static_cast<std::string>(piece1) << RESET;
                        else if (isBlackPiece(piece1))
                            std::cout << BLACK_COLOR << static_cast<std::string>(piece1) << RESET;
                        else
                            std::cout << pchColor1; // Fallback to square color if color is NONE
                        hasPiece1 = true;           // Set flag since a piece is present
                    }
                    else
                    {
                        std::cout << pchColor1; // Print square color if no piece
                    }
                }
                else if (subLine == 1 && subColumn == 3)
                {
                    if (hasPiece1)
                        std::cout << SPACE; // Print buffer only if a piece was present
                    else
                        std::cout << pchColor1; // Otherwise, print square color
                }
                else
                {
                    // Print the square color for other positions
                    std::cout << pchColor1;
                }
            }

            // Second square in the pair
            bool hasPiece2 = false; // Flag for the second square
            for (int subColumn = 0; subColumn < CELL; subColumn++)
            {
                if (subLine == 1 && subColumn == 2)
                {
                    // Determine the square index based on the board orientation (flipped or not)
                    Square sq2 = flip ? Square((7 - iLine) * 8 + (7 - iPair * 2 - 1)) : Square(iLine * 8 + iPair * 2 + 1);
                    Piece piece2 = board.at(sq2);

                    if (piece2 != Piece::NONE)
                    {
                        // Apply color based on piece color and print the Unicode symbol
                        if (isWhitePiece(piece2))
                            std::cout << WHITE_COLOR << static_cast<std::string>(piece2) << RESET;
                        else if (isBlackPiece(piece2))
                            std::cout << BLACK_COLOR << static_cast<std::string>(piece2) << RESET;
                        else
                            std::cout << pchColor2; // Fallback to square color if color is NONE
                        hasPiece2 = true;           // Set flag since a piece is present
                    }
                    else
                    {
                        std::cout << pchColor2; // Print square color if no piece
                    }
                }
                else if (subLine == 1 && subColumn == 3)
                {
                    if (hasPiece2)
                        std::cout << SPACE; // Print buffer only if a piece was present
                    else
                        std::cout << pchColor2; // Otherwise, print square color
                }
                else
                {
                    // Print the square color for other positions
                    std::cout << pchColor2;
                }
            }
        }

        if (subLine == 1)
        {
            // Print the row label (1-8) with blue color
            std::cout << "   " << LABEL_COLOR << (flip ? 8 - iLine : iLine + 1) << RESET;
        }
        std::cout << "\n";
    }
}

void printBoard(const Board &board, bool flip)
{
    // Colored labels at the top
    if (flip)
    {
        std::cout << "  "
                  << LABEL_COLOR << "H" << RESET << "     "
                  << LABEL_COLOR << "G" << RESET << "     "
                  << LABEL_COLOR << "F" << RESET << "     "
                  << LABEL_COLOR << "E" << RESET << "     "
                  << LABEL_COLOR << "D" << RESET << "     "
                  << LABEL_COLOR << "C" << RESET << "     "
                  << LABEL_COLOR << "B" << RESET << "     "
                  << LABEL_COLOR << "A" << RESET << "\n\n";
    }
    else
    {
        std::cout << "  "
                  << LABEL_COLOR << "A" << RESET << "     "
                  << LABEL_COLOR << "B" << RESET << "     "
                  << LABEL_COLOR << "C" << RESET << "     "
                  << LABEL_COLOR << "D" << RESET << "     "
                  << LABEL_COLOR << "E" << RESET << "     "
                  << LABEL_COLOR << "F" << RESET << "     "
                  << LABEL_COLOR << "G" << RESET << "     "
                  << LABEL_COLOR << "H" << RESET << "\n\n";
    }

    for (int iLine = 7; iLine >= 0; iLine--)
    {
        if (iLine % 2 == 0)
        {
            printLine(iLine, BLACK_SQUARE, WHITE_SQUARE, board, flip);
        }
        else
        {
            printLine(iLine, WHITE_SQUARE, BLACK_SQUARE, board, flip);
        }
    }

    // Colored labels at the bottom
    if (flip)
    {
        std::cout << "\n  "
                  << LABEL_COLOR << "H" << RESET << "     "
                  << LABEL_COLOR << "G" << RESET << "     "
                  << LABEL_COLOR << "F" << RESET << "     "
                  << LABEL_COLOR << "E" << RESET << "     "
                  << LABEL_COLOR << "D" << RESET << "     "
                  << LABEL_COLOR << "C" << RESET << "     "
                  << LABEL_COLOR << "B" << RESET << "     "
                  << LABEL_COLOR << "A" << RESET << "\n";
    }
    else
    {
        std::cout << "\n  "
                  << LABEL_COLOR << "A" << RESET << "     "
                  << LABEL_COLOR << "B" << RESET << "     "
                  << LABEL_COLOR << "C" << RESET << "     "
                  << LABEL_COLOR << "D" << RESET << "     "
                  << LABEL_COLOR << "E" << RESET << "     "
                  << LABEL_COLOR << "F" << RESET << "     "
                  << LABEL_COLOR << "G" << RESET << "     "
                  << LABEL_COLOR << "H" << RESET << "\n";
    }
}

int main()
{
    Board board(chess::constants::STARTPOS);
    std::string uciMove;
    GameResult result = GameResult::NONE;
    GameResultReason reason = GameResultReason::NONE;
    bool isWhiteTurn = true;

    while (result == GameResult::NONE)
    {
        printBoard(board, !isWhiteTurn);
        std::cout << "It is " << (isWhiteTurn ? "White" : "Black") << "'s turn." << std::endl;
        std::cout << "Enter your move (UCI format): ";
        std::cin >> uciMove;

        Move move = uci::uciToMove(board, uciMove);
        if (move == Move::NO_MOVE)
        {
            std::cout << "Invalid move. Try again." << std::endl;
            continue;
        }

        Movelist moves;
        movegen::legalmoves(moves, board);
        if (std::find(moves.begin(), moves.end(), move) == moves.end())
        {
            std::cout << "Illegal move. Try again." << std::endl;
            continue;
        }

        board.makeMove(move);
        std::tie(reason, result) = board.isGameOver();
        isWhiteTurn = !isWhiteTurn;
    }

    std::cout << "Game over! Result: ";
    switch (result)
    {
    case GameResult::WIN:
        std::cout << "Win";
        break;
    case GameResult::LOSE:
        std::cout << "Lose";
        break;
    case GameResult::DRAW:
        std::cout << "Draw";
        break;
    default:
        std::cout << "None";
        break;
    }
    std::cout << " due to " << static_cast<int>(reason) << std::endl;

    return 0;
}