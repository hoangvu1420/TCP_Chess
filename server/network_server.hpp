#ifndef NETWORK_SERVER_HPP
#define NETWORK_SERVER_HPP

#include <unordered_map>
#include <vector>
#include <mutex>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <iostream>

#include "../common/protocol.hpp"
#include "../common/message.hpp"
#include "../common/const.hpp"

struct ClientInfo
{
    std::vector<uint8_t> buffer;
    std::mutex mutex;
    std::string username = "";
};

class NetworkServer
{
private:
    int server_fd;
    std::unordered_map<int, ClientInfo> clients;
    std::mutex clients_mutex;

    /**
     * @brief Khởi tạo máy chủ với cổng được chỉ định.
     *
     * Tạo socket, thiết lập địa chỉ và cấu hình server để lắng nghe kết nối trên cổng đã cho.
     * Nếu xảy ra lỗi trong quá trình thiết lập, chương trình sẽ kết thúc.
     *
     * @param port Cổng mà server sẽ lắng nghe kết nối.
     */
    void initialize(uint16_t port)
    {
        server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd == -1)
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        sockaddr_in address;
        std::memset(&address, 0, sizeof(address));
        address.sin_family = AF_INET;
        address.sin_addr.s_addr = INADDR_ANY;
        address.sin_port = htons(port);

        if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
        {
            perror("bind failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        if (listen(server_fd, Const::BACKLOG) < 0)
        {
            perror("listen failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }

        std::cout << "Server đang lắng nghe trên: " << inet_ntoa(address.sin_addr) << ":" << ntohs(address.sin_port) << " ..." << std::endl;
    }

    // Private constructor for Singleton
    NetworkServer() : server_fd(-1)
    {
        initialize(Const::SERVER_PORT);
    }

public:
    // Delete copy constructor and assignment operator
    NetworkServer(const NetworkServer &) = delete;
    NetworkServer &operator=(const NetworkServer &) = delete;

    // Destructor
    ~NetworkServer()
    {
        if (server_fd != -1)
        {
            close(server_fd);
        }
    }

    // Static method to get the singleton instance
    static NetworkServer &getInstance()
    {
        static NetworkServer instance;
        return instance;
    }

    /**
     * @brief Chấp nhận kết nối mới từ khách hàng.
     *
     * @return fd của client nếu thành công, -1 nếu thất bại.
     */
    int acceptConnection()
    {
        sockaddr_in client_address;
        socklen_t client_len = sizeof(client_address);

        int client_fd = accept(server_fd, (struct sockaddr *)&client_address, &client_len);
        if (client_fd < 0)
        {
            perror("accept failed");
            return -1;
        }

        std::cout << "Client kết nối từ: " << inet_ntoa(client_address.sin_addr) << ":"
                  << ntohs(client_address.sin_port) << " (fd = " << client_fd << ")" << std::endl;
        return client_fd;
    }

    /**
     * Gửi một gói tin đến client.
     *
     * @param client_fd Định danh của client.
     * @param messageType Loại thông điệp.
     * @param payload Dữ liệu payload của gói tin.
     * @return true nếu gửi thành công, false nếu thất bại.
     */
    bool sendPacket(int client_fd, MessageType messageType, const std::vector<uint8_t> &payload)
    {
        uint8_t type = static_cast<uint8_t>(messageType);
        uint16_t length = htons(static_cast<uint16_t>(payload.size())); // Chuyển đổi một lần

        std::vector<uint8_t> packet;
        packet.push_back(type);
        packet.push_back(static_cast<uint8_t>((length >> 8) & 0xFF)); // Byte cao
        packet.push_back(static_cast<uint8_t>(length & 0xFF));        // Byte thấp
        packet.insert(packet.end(), payload.begin(), payload.end());

        ssize_t sent = send(client_fd, packet.data(), packet.size(), 0);
        if (sent != static_cast<ssize_t>(packet.size()))
        {
            perror("send failed");
            return false;
        }
        return true;
    }

    /**
     * Gửi một gói tin đến người dùng bằng tên đăng nhập.
     *
     * @param username Tên đăng nhập của người dùng nhận gói tin.
     * @param messageType Loại thông điệp cần gửi.
     * @param payload Dữ liệu gói tin cần gửi.
     * @return Trả về true nếu gửi thành công, ngược lại trả về false.
     */
    bool sendPacketToUsername(const std::string &username, MessageType messageType, const std::vector<uint8_t> &payload)
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (const auto &pair : clients)
        {
            if (pair.second.username == username)
            {
                return sendPacket(pair.first, messageType, payload);
            }
        }
        std::cerr << "Username " << username << " không được tìm thấy." << std::endl;
        return false;
    }

    /**
     * @brief Nhận một gói tin từ client.
     *
     * @param client_fd Mô tả socket của client.
     * @param packet Tham chiếu để chứa gói tin nhận được.
     * @return true nếu nhận thành công, false nếu thất bại hoặc kết nối bị đóng.
     */
    bool receivePacket(int client_fd, Packet &packet)
    {
        // Đọc dữ liệu từ client_fd mà không giữ khóa
        uint8_t buffer_temp[Const::BUFFER_SIZE];
        ssize_t bytes_received = recv(client_fd, buffer_temp, sizeof(buffer_temp), 0);
        if (bytes_received <= 0)
        {
            return false;
        }

        // Bảo vệ map clients khi thêm dữ liệu vào buffer
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients[client_fd].buffer.insert(clients[client_fd].buffer.end(), buffer_temp, buffer_temp + bytes_received);
        }

        // Xử lý gói tin
        {
            // Bảo vệ buffer của client_fd
            std::lock_guard<std::mutex> lock(clients[client_fd].mutex);
            auto &buffer = clients[client_fd].buffer;
            while (buffer.size() >= 3)
            {
                MessageType type = static_cast<MessageType>(buffer[0]);
                uint16_t length = (static_cast<uint16_t>(buffer[1]) << 8) |
                                  static_cast<uint16_t>(buffer[2]);

                length = ntohs(length);

                if (buffer.size() < 3 + length)
                {
                    break; // Chưa đủ dữ liệu
                }

                std::vector<uint8_t> payload(buffer.begin() + 3, buffer.begin() + 3 + length);
                packet = Packet{type, length, payload};

                buffer.erase(buffer.begin(), buffer.begin() + 3 + length);
                return true;
            }
        }

        return false; // Chưa nhận đủ dữ liệu
    }

    void setUsername(int client_fd, const std::string &username)
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients[client_fd].username = username;
    }

    std::string getUsername(int client_fd)
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        return clients[client_fd].username;
    }

    int getClientFD(const std::string &username)
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (const auto &pair : clients)
        {
            if (pair.second.username == username)
            {
                return pair.first;
            }
        }
        return -1;
    }

    bool isUserLoggedIn(const std::string &username)
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        for (const auto &pair : clients)
        {
            if (pair.second.username == username)
            {
                return true;
            }
        }
        return false;
    }

    void closeConnection(int client_fd)
    {
        close(client_fd);
        // Xóa thông tin client khỏi clients map
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            clients.erase(client_fd);
        }
    }

    void closeAllConnections()
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        close(server_fd);
        for (auto &pair : clients)
        {
            close(pair.first);
        }
        clients.clear();
    }

    void dispose()
    {
        closeAllConnections();
        delete &getInstance();
    }
};

#endif // NETWORK_SERVER_HPP