#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "../chess_engine/chess.hpp"
#include "../common/utils.hpp"

#define PORT 8088
#define BUFFER_SIZE 1024

// Loại Thông Điệp
#define PLAYER_ID 0x01
#define MOVE 0x02
#define STATE_UPDATE 0x03
#define RESULT 0x04
#define TURN_NOTIFICATION 0x05

// Hàm gửi thông điệp
int send_message(int sock, uint8_t type, const std::string &payload)
{
    std::vector<uint8_t> packet;
    
    packet.push_back(type);
    uint16_t length = payload.size();

    std::vector<uint8_t> length_bytes = to_big_endian_16(length);
    packet.insert(packet.end(), length_bytes.begin(), length_bytes.end());
    packet.insert(packet.end(), payload.begin(), payload.end());

    return send(sock, packet.data(), packet.size(), 0);
}

// Hàm nhận thông điệp
bool receive_message(int sock, uint8_t &type, std::string &payload)
{
    std::vector<uint8_t> header(3);

    int bytes = recv(sock, header.data(), 3, MSG_WAITALL);
    if (bytes <= 0)
        return false;
    type = header[0];

    uint16_t length = from_big_endian_16(header, 1);
    if (length > 0)
    {
        std::vector<char> data(length);
        bytes = recv(sock, data.data(), length, MSG_WAITALL);
        if (bytes <= 0)
            return false;
        payload.assign(data.begin(), data.end());
    }
    else
    {
        payload = "";
    }
    return true;
}

bool isValidMove(const chess::Board &board, const chess::Move &move)
{
    if (move == chess::Move::NO_MOVE)
    {
        return false;
    }

    chess::Movelist moves;
    chess::movegen::legalmoves(moves, board);
    if (std::find(moves.begin(), moves.end(), move) == moves.end())
    {
        return false;
    }

    return true;
}

int main()
{
    int server_sock, client1_sock, client2_sock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_len = sizeof(struct sockaddr_in);
    uint8_t type;
    std::string payload;

    // Tạo socket
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // Thiết lập địa chỉ server
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_sock, 2) < 0)
    {
        perror("Listen failed");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    std::cout << "Server đang lắng nghe trên cổng " << PORT << "...\n";

    // Chấp nhận kết nối người chơi 1
    if ((client1_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len)) < 0)
    {
        perror("Failed to accept connection for player 1");
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    std::cout << "Người chơi 1 đã kết nối.\n";

    // Gửi PLAYER_ID cho người chơi 1
    send_message(client1_sock, PLAYER_ID, "1");

    // Chấp nhận kết nối người chơi 2
    if ((client2_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_len)) < 0)
    {
        perror("Failed to accept connection for player 2");
        close(client1_sock);
        close(server_sock);
        exit(EXIT_FAILURE);
    }
    std::cout << "Người chơi 2 đã kết nối.\n";

    // Gửi PLAYER_ID cho người chơi 2
    send_message(client2_sock, PLAYER_ID, "2");

    // Khởi tạo bàn cờ
    const char *initFen = chess::constants::STARTPOS;
    chess::Board board(initFen);
    chess::GameResult result = chess::GameResult::NONE;
    chess::GameResultReason reason = chess::GameResultReason::NONE;

    std::string current_turn = "1"; // Người chơi 1 (White) đi trước
    bool game_over = false;

    // Gửi cập nhật bàn cờ cho cả hai người chơi
    send_message(client1_sock, STATE_UPDATE, board.getFen());
    send_message(client2_sock, STATE_UPDATE, board.getFen());

    while (!game_over)
    {
        int current_sock = (current_turn == "1") ? client1_sock : client2_sock;
        int other_sock = (current_turn == "1") ? client2_sock : client1_sock;

        // Thông báo lượt cho người chơi hiện tại
        send_message(client1_sock, TURN_NOTIFICATION, current_turn);
        send_message(client2_sock, TURN_NOTIFICATION, current_turn);

        // Nhận nước đi từ người chơi hiện tại
        if (!receive_message(current_sock, type, payload) || type != MOVE)
        {
            std::cout << "Người chơi " << current_turn << " đã ngắt kết nối.\n";
            // Gửi kết quả cho người chơi còn lại
            send_message(other_sock, RESULT, (current_turn == "1") ? "2" : "1");
            break;
        }

        // Giải mã nước đi (UCI format)
        std::string uci_move = payload;
        chess::Move move = chess::uci::uciToMove(board, uci_move);
        if (!isValidMove(board, move))
        {
            std::cout << "Người chơi " << current_turn << " đã gửi nước đi không hợp lệ: " << uci_move << "\n";
            continue; // Yêu cầu người chơi gửi lại
        }

        // Thực hiện nước đi
        board.makeMove(move);

        // Gửi cập nhật bàn cờ cho cả hai người chơi
        send_message(client1_sock, STATE_UPDATE, board.getFen());
        send_message(client2_sock, STATE_UPDATE, board.getFen());

        // Kiểm tra kết quả trò chơi
        std::tie(reason, result) = board.isGameOver();

        if (result != chess::GameResult::NONE)
        {
            switch (result)
            {
            case chess::GameResult::LOSE:
                if (current_turn == "1")
                {
                    send_message(client1_sock, RESULT, "1");
                    send_message(client2_sock, RESULT, "1");
                    std::cout << "Người chơi 1 (White) thắng!\n";
                }
                else
                {
                    send_message(client1_sock, RESULT, "2");
                    send_message(client2_sock, RESULT, "2");
                    std::cout << "Người chơi 2 (Black) thắng!\n";
                }
                game_over = true;
                break;
            case chess::GameResult::DRAW:
                send_message(client1_sock, RESULT, "0");
                send_message(client2_sock, RESULT, "0");
                std::cout << "Trò chơi hòa.\n";
                game_over = true;
                break;
            default:
                break;
            }
        }
        else
        {
            // Chuyển lượt chơi
            current_turn = (current_turn == "1") ? "2" : "1";
        }
    }

    // Đóng kết nối
    close(client1_sock);
    close(client2_sock);
    close(server_sock);
    return 0;
}