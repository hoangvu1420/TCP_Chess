#include <iostream>
#include <thread>
#include <atomic>
#include <chrono>

#include "network_client.hpp"
#include "message_handler.hpp"
#include "logic_handler.hpp"

int main()
{
    NetworkClient& network_client = NetworkClient::getInstance();
    SessionData& session_data = SessionData::getInstance();

    session_data.setRunning(true);

    // Thread để nhận thông điệp từ server
    std::thread receiverThread([&]()
    {
        MessageHandler handler;

        while (session_data.getRunning()) {
            Packet packet;
            bool received = network_client.receivePacket(packet);
            if (received)
            {
                bool isSuccess = handler.handleMessage(packet);

                if (!isSuccess) 
                {
                    std::cout << "Xử lý thông điệp thất bại." << std::endl;
                    session_data.setRunning(false);
                }
            }
            else
            {
                if (!session_data.getRunning())
                {
                    // Nếu running = false thì thoát
                    break;
                }
                // Ngược lại, có thể tiếp tục đợi dữ liệu
            }
        } 
    });

    // Thread để nhập liệu từ người dùng
    std::thread senderThread([&]()
    {
        LogicHandler logic_handler;
        logic_handler.handleInitialMenu();
    });

    senderThread.join();
    receiverThread.join();
    std::cout << "Client đã đóng kết nối." << std::endl;

    return 0;
}