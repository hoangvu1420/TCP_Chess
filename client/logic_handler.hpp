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

class MessageHandler;

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
            std::string result = UI::displayInitialMenu();

            // if the input was cancelled
            if (session_data.shouldStop())
            {
                std::cout << "shouldStop..." << std::endl;
                break;
            }

            int choice;
            try
            {
                choice = std::stoi(result);
            }
            catch (const std::exception &e)
            {
                UI::printErrorMessage("Lựa chọn không hợp lệ. Vui lòng chọn lại.");
                continue;
            }

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
        NetworkClient &network_client = NetworkClient::getInstance();
        SessionData &session_data = SessionData::getInstance();

        while (true)
        {
            std::string result = UI::displayGameMenu();

            // if result is empty, it means the input was cancelled
            if (session_data.shouldStop())
            {
                std::cout << "shouldStop..." << std::endl;
                break;
            }

            int choice;
            try
            {
                choice = std::stoi(result);
            }
            catch (const std::exception &e)
            {
                UI::printErrorMessage("Lựa chọn không hợp lệ. Vui lòng chọn lại.");
                continue;
            }

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
                // Xem danh sách người chơi trực tuyến
                RequestPlayerListMessage request_player_list_msg;
                if (!network_client.sendPacket(request_player_list_msg.getType(), request_player_list_msg.serialize()))
                {
                    UI::printErrorMessage("Tải danh sách người chơi trực tuyến thất bại.");
                    break;
                }
                UI::printInfoMessage("Danh sách người chơi trực tuyến:");
                break;
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
            std::string result = UI::displayAutoMatchOptions();

            // if the input was cancelled
            if (SessionData::getInstance().shouldStop())
            {
                std::cout << "shouldStop..." << std::endl;
                break;
            }

            int choice;
            try
            {
                choice = std::stoi(result);
            }
            catch (const std::exception &e)
            {
                UI::printErrorMessage("Lựa chọn không hợp lệ. Vui lòng chọn lại.");
                continue;
            }

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
        std::cout << "Game_id: " << message.game_id << std::endl
                  << "FEN: " << message.fen << std::endl
                  << "Current_turn_username: " << message.current_turn_username << std::endl
                  << "Is_game_over: " << static_cast<bool>(message.is_game_over) << std::endl;

        // Update session data
        SessionData &session_data = SessionData::getInstance();

        bool is_my_turn = message.current_turn_username == session_data.getUsername();

        session_data.setTurn(is_my_turn);
        session_data.setFen(message.fen);

        if (static_cast<bool>(message.is_game_over))
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
            std::string result = UI::getMove();

            // Nếu người dùng không nhập nước đi
            if (session_data.shouldStop())
            {
                std::cout << "shouldStop..." << std::endl;
                return;
            }

            std::string uci_move = result;

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

    void handleChallenge()
    {
        // Thách đấu người chơi khác
        std::string opponent = UI::displayChallengeMenu();

        // Nếu người dùng chọn quay lại
        if (opponent == "<NO_USERNAME_PROVIDED>")
            return;

        // Nếu người dùng không chọn quay lại và nhập username
        ChallengeRequestMessage challenge_request_msg;

        challenge_request_msg.to_username = opponent;

        NetworkClient &network_client = NetworkClient::getInstance();

        if (!network_client.sendPacket(challenge_request_msg.getType(), challenge_request_msg.serialize()))
        {
            UI::printErrorMessage("Gửi yêu cầu thách đấu thất bại.");
        }
        else
        {
            UI::printInfoMessage("Đã gửi yêu cầu thách đấu. Đang chờ phản hồi...");
            std::this_thread::sleep_for(std::chrono::seconds(10));
            UI::printInfoMessage("Hết thời gian chờ đợi.");
        }
    }

    void handleChallengeDecision()
    {
        NetworkClient &network_client = NetworkClient::getInstance();

        while (true)
        {
            std::string result = UI::displayChallengeDecision();

            // if the input was cancelled
            if (SessionData::getInstance().shouldStop())
            {
                std::cout << "shouldStop..." << std::endl;
                break;
            }

            int choice;

            if (result == "<TIMEOUT>")
            {
                UI::printErrorMessage("Hết thời gian chờ đợi. Tự động từ chối thách đấu.");
                choice = 2;
            }
            else
            {
                try
                {
                    choice = std::stoi(result);
                }
                catch (const std::exception &e)
                {
                    UI::printErrorMessage("Lựa chọn không hợp lệ. Vui lòng chọn lại.");
                    continue;
                }
            }

            if (choice == 1)
            {
                // Chấp nhận
                // ChallengeAcceptedMessage challenge_accepted_msg;

                // if (!network_client.sendPacket(challenge_accepted_msg.getType(), challenge_accepted_msg.serialize()))
                // {
                //     UI::printErrorMessage("Gửi yêu cầu chấp nhận thách đấu thất bại.");
                //     break;
                // }

                UI::printInfoMessage("Đã chấp nhận thách đấu.");
                break;
            }
            else if (choice == 2)
            {
                // Từ chối
                // ChallengeDeclinedMessage challenge_declined_msg;

                // if (!network_client.sendPacket(challenge_declined_msg.getType(), challenge_declined_msg.serialize()))
                // {
                //     UI::printErrorMessage("Gửi yêu cầu từ chối thách đấu thất bại.");
                //     break;
                // }

                UI::printInfoMessage("Đã từ chối thách đấu.");
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

private:

}; // class LogicHandler

#endif // LOGIC_HANDLER_HPP
