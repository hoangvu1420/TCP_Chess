// ui.hpp
#ifndef UI_HPP
#define UI_HPP

#include <iostream>
#include <string>
#include <cstring>

#include "../libraries/tabulate.hpp"

#include "message_handler.hpp"
#include "board_display.hpp"

#define RESET "\033[0m"
#define CYAN "\033[96m" // Cyan
#define RED "\033[31m" // Red
#define GREEN "\033[32m" // Green
#define YELLOW "\033[33m" // Yellow
#define BLUE "\033[34m" // Blue

/**
 * @namespace UI
 * @brief Chứa các chức năng giao diện người dùng như hiển thị menu, thông báo và tương tác với người dùng.
 */
namespace UI
{
    // Clear the console
    void clearConsole()
    {
        std::cout << "\033[2J\033[H" << std::flush;
    }

    // Print the logo
    void printLogo(void)
    {
        std::cout << std::endl;
        std::cout << BLUE << "=============================================================\n" << RESET;
        std::cout << "   _______   _____  _____     _____  _                       \n";
        std::cout << "  |__   __| / ____||  __ \\   / ____|| |                     \n";
        std::cout << "     | |   | |     | |__) | | |     | |__    ___  ___  ___   \n";
        std::cout << "     | |   | |     |  ___/  | |     | '_ \\  / _ \\/ __|/ __|\n";
        std::cout << "     | |   | |____ | |      | |____ | | | ||  __/\\__ \\\\__ \\\n";
        std::cout << "     |_|    \\_____||_|       \\_____||_| |_| \\___||___/|___/\n";
        std::cout << BLUE << "=============================================================\n" << RESET;
    }

    // Print error message in red
    void printErrorMessage(const std::string &message)
    {
        std::cout << std::endl;
        std::cerr << RED << message << RESET << std::endl;
    }

    // Print success message in green
    void printSuccessMessage(const std::string &message)
    {
        std::cout << std::endl;
        std::cout << GREEN << message << RESET << std::endl;
    }

    // Print info message in cyan
    void printInfoMessage(const std::string &message)
    {
        std::cout << std::endl;
        std::cout << CYAN << message << RESET << std::endl;
    }

    // Display the main menu
    int displayInitialMenu()
    {
        std::cout << "\n=========Main menu=========" << std::endl;
        std::cout << "Chọn hành động: " << std::endl;
        std::cout << "  1. Đăng ký" << std::endl;
        std::cout << "  2. Đăng nhập" << std::endl;
        std::cout << "  3. Thoát" << std::endl;

        int choice;
        std::cout << "> " << std::flush;
        std::cin >> choice;
        return choice;
    }

    // Handle user input for registration
    std::string displayRegister()
    {
        std::string username;
        std::cout << "\n=========Register=========" << std::endl;
        std::cout << "Username: ";
        std::cin >> username;
        return username;
    }

    // Handle user input for login
    std::string displayLogin(NetworkClient &network_client)
    {
        std::string username, password;
        std::cout << "\n=========Login=========" << std::endl;
        std::cout << "Username: ";
        std::cin >> username;
        
        return username;
    }

    // Display the game menu
    int displayGameMenu()
    {
        std::cout << "\n=========Game menu=========" << std::endl;
        std::cout << "Chọn hành động: " << std::endl;
        std::cout << "  1. Ghép trận tự động" << std::endl;
        std::cout << "  2. Danh sách người chơi trực tuyến" << std::endl;
        std::cout << "  3. Trở về" << std::endl;

        int choice;
        std::cout << "> " << std::flush;
        std::cin >> choice;
        return choice;
    }

    // Display the auto match options
    int displayAutoMatchOptions()
    {
        std::cout << "\n=========Auto match=========" << std::endl;
        std::cout << "Chọn hành động: " << std::endl;
        std::cout << "  1. Chấp nhận" << std::endl;
        std::cout << "  2. Từ chối" << std::endl;

        int choice;
        std::cout << "> " << std::flush;
        std::cin >> choice;
        return choice;
    }

    // Display the chess board
    void showBoard(const std::string &fen, bool flip = false)
    {
        board_display::printBoard(fen, flip);
    }

    std::string getMove()
    {
        std::string move;
        std::cout << "Nhập nước đi (VD: e2e4): " << std::flush;
        std::cin >> move;
        return move;
    }

    // Display "challenge other players" menu
    std::string displayChallengeMenu() {
        std::cout << "\n===== Thách đấu người chơi khác =====" << std::endl;
        std::cout << "1. Thách đấu người chơi khác" << std::endl;
        std::cout << "2. Quay lại" << std::endl;

        int choice;
        std::cout << "> " << std::flush;
        std::cin >> choice;

        if (choice == 1) {
            std::string username;
            std::cout << "Nhập username: " << std::flush;
            std::cin >> username;
            return username;
        } else {
            return "NO_USERNAME_PROVIDED";
        }
    }


} // namespace UI

#endif // UI_HPP