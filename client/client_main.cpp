#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "../common/utils.hpp"
#include "board_display.hpp"

#define PORT 8088
#define SERVER_IP "127.0.0.1"
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

int main()
{
    int sock;
    struct sockaddr_in server_addr;
    uint8_t type;
    std::string payload;
    std::string player_id;

    // Tạo socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("Tạo socket thất bại");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Chuyển đổi địa chỉ IP
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0)
    {
        perror("Địa chỉ IP không hợp lệ hoặc không hỗ trợ");
        close(sock);
        exit(EXIT_FAILURE);
    }

    // Kết nối đến server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Kết nối thất bại");
        close(sock);
        exit(EXIT_FAILURE);
    }
    std::cout << "Đã kết nối đến server.\n";

    // Nhận PLAYER_ID từ server
    if (!receive_message(sock, type, payload) || type != PLAYER_ID)
    {
        std::cout << "Nhận PLAYER_ID không thành công.\n";
        close(sock);
        exit(EXIT_FAILURE);
    }
    player_id = payload;
    std::cout << "Bạn là Người chơi " << player_id << " (" << ((player_id == "1") ? "White" : "Black") << ").\n";

    while (true)
    {
        // Đọc loại thông điệp
        if (!receive_message(sock, type, payload))
        {
            std::cout << "Ngắt kết nối từ server.\n";
            break;
        }

        switch (type)
        {
        case TURN_NOTIFICATION:
        {
            if (payload == player_id)
            {
                std::cout << "Đến lượt bạn.\n";
                std::cout << "Nhập nước đi của bạn (UCI format, ví dụ: e2e4): ";
                std::string uci_move;
                std::cin >> uci_move;
                send_message(sock, MOVE, uci_move);
            }
            else
            {
                std::cout << "Đợi đối thủ ra nước đi...\n";
            }
            break;
        }

        case STATE_UPDATE:
        {
            bool flip = (player_id == "2");
            board_display::printBoard(payload, flip);
            break;
        }

        case RESULT:
        {
            if (payload == "1")
            {
                if (player_id == "1")
                    std::cout << "Chúc mừng! Bạn đã thắng!\n";
                else
                    std::cout << "Bạn đã thua.\n";
            }
            else if (payload == "2")
            {
                if (player_id == "2")
                    std::cout << "Chúc mừng! Bạn đã thắng!\n";
                else
                    std::cout << "Bạn đã thua.\n";
            }
            else if (payload == "0")
            {
                std::cout << "Trò chơi hòa.\n";
            }
            close(sock);
            exit(0);
            break;
        }

        default:
        {
            std::cout << "Thông điệp không xác định: " << static_cast<int>(type) << "\n";
            break;
        }
        }
    }

    close(sock);
    return 0;
}