#ifndef LOGIC_HANDLER_HPP
#define LOGIC_HANDLER_HPP

#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>

#include "../common/protocol.hpp"
#include "../common/message.hpp"

#include "session_data.hpp"
#include "network_client.hpp"
#include "ui.hpp"

class LogicHandler
{
public:
    /**
     * @brief Xử lý menu ban đầu của ứng dụng, cho phép người dùng đăng ký, đăng nhập hoặc thoát.
     *
     * Hàm này hiển thị logo ứng dụng và trình bày menu ban đầu cho người dùng.
     * Dựa trên lựa chọn của người dùng, nó thực hiện các hành động sau:
     *
     * - Đăng ký (Lựa chọn 1): Yêu cầu người dùng nhập tên đăng nhập, tạo một RegisterMessage,
     *   và gửi nó đến máy chủ. Nếu gửi thất bại, một thông báo lỗi được hiển thị.
     *
     * - Đăng nhập (Lựa chọn 2): Yêu cầu người dùng nhập tên đăng nhập, tạo một LoginMessage,
     *   và gửi nó đến máy chủ. Nếu gửi thất bại, một thông báo lỗi được hiển thị.
     *
     * - Thoát (Lựa chọn 3): Đặt cờ running thành false và đóng kết nối mạng.
     *
     * Nếu người dùng nhập một lựa chọn không hợp lệ, một thông báo thích hợp được hiển thị.
     *
     * @param running Một tham chiếu đến một biến boolean nguyên tử điều khiển trạng thái chạy của ứng dụng.
     */
    void handleInitialMenu()
    {
        UI::printLogo();

        NetworkClient &network_client = NetworkClient::getInstance();
        SessionData &session_data = SessionData::getInstance();

        while (true)
        {
            int choice = UI::displayInitialMenu();
            if (choice == 1)
            {
                std::string username = UI::displayRegister();

                RegisterMessage reg_msg = RegisterMessage{username};

                if (!network_client.sendPacket(reg_msg.getType(), reg_msg.serialize()))
                {
                    UI::printErrorMessage("Gửi yêu cầu đăng ký thất bại.");
                    break;
                }
                break;
            }
            else if (choice == 2)
            {
                std::string username = UI::displayLogin(network_client);

                LoginMessage login_msg = LoginMessage{username};

                if (!network_client.sendPacket(login_msg.getType(), login_msg.serialize()))
                {
                    UI::printErrorMessage("Gửi yêu cầu đăng nhập thất bại.");
                    break;
                }
                break;
            }
            else if (choice == 3)
            {
                session_data.setRunning(false);
                network_client.closeConnection();
                break;
            }
            else
            {
                UI::printErrorMessage("Lựa chọn không hợp lệ. Vui lòng chọn lại.");
            }
        }
    }

    /**
     * Xử lý menu trò chơi, bao gồm gửi yêu cầu ghép trận tự động, chơi với máy, hoặc trở về menu chính.
     */
    void handleGameMenu()
    {
        // UI::printLogo();

        NetworkClient &network_client = NetworkClient::getInstance();
        SessionData &session_data = SessionData::getInstance();

        while (true)
        {
            int choice = UI::displayGameMenu();
            if (choice == 1)
            {
                // Gửi yêu cầu tìm đối thủ
                AutoMatchRequestMessage auto_match_request_msg;
                auto_match_request_msg.username = session_data.getUsername();

                if (!network_client.sendPacket(auto_match_request_msg.getType(), auto_match_request_msg.serialize()))
                {
                    UI::printErrorMessage("Gửi yêu cầu ghép trận tự động thất bại.");
                    break;
                }

                UI::printInfoMessage("Đang tìm đối thủ...");
                break;
            }
            else if (choice == 2)
            {
                // Chơi với máy
                UI::printErrorMessage("Chức năng này đang nấu.");
            }
            else if (choice == 3)
            {
                handleInitialMenu();
                break;
            }
            else
            {
                UI::printErrorMessage("Lựa chọn không hợp lệ. Vui lòng chọn lại.");
                break;
            }
        }
    }

    void handleMatchDecision(AutoMatchFoundMessage &message)
    {
        UI::printSuccessMessage("Tìm thấy trận!");
        std::cout << "Opponent_username: " << message.opponent_username << "\n"
                  << "Opponent_elo: " << message.opponent_elo << "\n"
                  << "Game_id: " << message.game_id << std::endl;

        NetworkClient &network_client = NetworkClient::getInstance();

        while (true)
        {
            int choice = UI::displayAutoMatchOptions();
            if (choice == 1)
            {
                // Chấp nhận
                AutoMatchAcceptedMessage auto_match_accepted_msg;
                auto_match_accepted_msg.game_id = message.game_id;

                if (!network_client.sendPacket(auto_match_accepted_msg.getType(), auto_match_accepted_msg.serialize()))
                {
                    UI::printErrorMessage("Gửi yêu cầu chấp nhận ghép trận tự động thất bại.");
                    break;
                }

                UI::printInfoMessage("Đã chấp nhận ghép trận tự động.");
                break;
            }
            else if (choice == 2)
            {
                // Từ chối
                AutoMatchDeclinedMessage auto_match_declined_msg;
                auto_match_declined_msg.game_id = message.game_id;

                if (!network_client.sendPacket(auto_match_declined_msg.getType(), auto_match_declined_msg.serialize()))
                {
                    UI::printErrorMessage("Gửi yêu cầu từ chối ghép trận tự động thất bại.");
                    break;
                }

                UI::printInfoMessage("Đã từ chối ghép trận tự động.");

                handleGameMenu();
                break;
            }
            else
            {
                UI::printErrorMessage("Lựa chọn không hợp lệ. Vui lòng chọn lại.");
                break;
            }
        }
    }

    void handleGameStart(const GameStartMessage &message)
    {
        UI::printInfoMessage("Trò chơi đã bắt đầu.");
        std::cout << "Game_id: " << message.game_id << "\n"
                  << "Player_1: " << message.player1_username << "\n"
                  << "Player_2: " << message.player2_username << "\n"
                  << "Starting player: " << message.starting_player_username << "\n"
                  << "FEN: " << message.fen << std::endl;

        // Set game status
        SessionData &session_data = SessionData::getInstance();

        bool is_white = message.starting_player_username == session_data.getUsername();
        session_data.setGameStatus(message.game_id, is_white, message.fen);

        // Bắt đầu trò chơi
        handleMove();
    }

    void handleGameStatusUpdate(const GameStatusUpdateMessage &message)
    {
        UI::printInfoMessage("Trò chơi đã cập nhật.");
        std::cout << "Game_id: " << message.game_id << "\n"
                  << "FEN: " << message.fen << "\n"
                  << "Current_turn_username: " << message.current_turn_username << std::endl;

        // Update session data
        SessionData &session_data = SessionData::getInstance();

        bool is_my_turn = message.current_turn_username == session_data.getUsername();

        session_data.setTurn(is_my_turn);
        session_data.setFen(message.fen);

        if (message.is_game_over)
        {
            UI::showBoard(session_data.getFen(), !session_data.isWhite());
            return;
        }

        handleMove();
    }

    void handleMove()
    {
        SessionData &session_data = SessionData::getInstance();
        NetworkClient &network_client = NetworkClient::getInstance();

        UI::showBoard(session_data.getFen(), !session_data.isWhite());

        if (session_data.isMyTurn())
        {
            // Lượt của người chơi
            std::string uci_move = UI::getMove();

            // Gửi nước đi
            MoveMessage move_msg;
            move_msg.game_id = session_data.getGameId();
            move_msg.uci_move = uci_move;

            if (!network_client.sendPacket(move_msg.getType(), move_msg.serialize()))
            {
                UI::printErrorMessage("Gửi nước đi thất bại.");
                return;
            }
        }
        else
        {
            // Lượt của đối thủ
            UI::printInfoMessage("Đang chờ đối thủ ra nước đi...");
        }
    }

private:

}; // class LogicHandler

#endif // LOGIC_HANDLER_HPP
