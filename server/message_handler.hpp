#ifndef MESSAGE_HANDLER_HPP
#define MESSAGE_HANDLER_HPP

#include <string>
#include <vector>
#include <memory>

#include "../common/protocol.hpp"
#include "../common/message.hpp"
#include "../common/const.hpp"

#include "data_storage.hpp"
#include "network_server.hpp"
#include "game_manager.hpp"

class MessageHandler
{
public:
    // Handle incoming messages ================================================================================

    void handleMessage(int client_fd, const Packet &packet)
    {
        switch (packet.type)
        {
        case MessageType::REGISTER:
            // Handle register
            handleRegister(client_fd, packet.payload);
            break;
        case MessageType::LOGIN:
            // Handle login
            handleLogin(client_fd, packet.payload);
            break;
        case MessageType::MOVE:
            // Handle move
            handleMove(client_fd, packet.payload);
            break;

        case MessageType::AUTO_MATCH_REQUEST:
            // Handle auto match request
            handleAutoMatchRequest(client_fd, packet.payload);
            break;
        case MessageType::AUTO_MATCH_ACCEPTED:
            // Handle auto match accepted
            handleAutoMatchAccepted(client_fd, packet.payload);
            break;
        case MessageType::AUTO_MATCH_DECLINED:
            // Handle auto match declined
            handleAutoMatchDeclined(client_fd, packet.payload);
            break;

            // Add additional cases for other MessageTypes

        case MessageType::REQUEST_PLAYER_LIST:
            // Handle request player list
            handleRequestPlayerList(client_fd, packet.payload);
            break;

        case MessageType::CHALLENGE_REQUEST:
            // Handle incoming challenge request
            handleChallengeRequest(client_fd, packet.payload);
            break;

        default:
            // Handle unknown message type
            handleUnknown(client_fd, packet.payload);
            break;
        }
    }

private:
    // Handle specific message types ============================================================================

    void handleUnknown(int client_fd, const std::vector<uint8_t> &payload)
    {
        std::cout << "[UNKNOWN]" << std::endl;
    }

    void handleRegister(int client_fd, const std::vector<uint8_t> &payload)
    {
        RegisterMessage message = RegisterMessage::deserialize(payload);

        std::cout << "[REGISTER] username: " << message.username << std::endl;

        DataStorage &storage = DataStorage::getInstance();
        bool isUserValid = storage.registerUser(message.username);

        NetworkServer &server = NetworkServer::getInstance();

        if (isUserValid)
        {
            RegisterSuccessMessage successMessage;
            successMessage.username = message.username;
            successMessage.elo = Const::DEFAULT_ELO;
            std::vector<uint8_t> serialized = successMessage.serialize();
            server.sendPacket(client_fd, successMessage.getType(), serialized);
            server.setUsername(client_fd, message.username);
        }
        else
        {
            RegisterFailureMessage failureMessage;
            failureMessage.error_message = "Username already exists.";
            std::vector<uint8_t> serialized = failureMessage.serialize();
            server.sendPacket(client_fd, failureMessage.getType(), serialized);
        }
    }

    void handleLogin(int client_fd, const std::vector<uint8_t> &payload)
    {
        LoginMessage message = LoginMessage::deserialize(payload);

        std::cout << "[LOGIN] username: " << message.username << ", client_fd: " << client_fd << std::endl;

        DataStorage &storage = DataStorage::getInstance();
        bool isUserValid = storage.validateUser(message.username);

        NetworkServer &server = NetworkServer::getInstance();

        bool isLoggedIn = server.isUserLoggedIn(message.username);

        if (isUserValid && !isLoggedIn)
        {
            uint16_t elo = storage.getUserELO(message.username);

            LoginSuccessMessage successMessage;
            successMessage.username = message.username;
            successMessage.elo = elo;
            std::vector<uint8_t> serialized = successMessage.serialize();
            server.sendPacket(client_fd, successMessage.getType(), serialized);
            server.setUsername(client_fd, message.username);
        }
        else if (!isUserValid)
        {
            LoginFailureMessage failureMessage;
            failureMessage.error_message = "Invalid username.";
            std::vector<uint8_t> serialized = failureMessage.serialize();
            server.sendPacket(client_fd, failureMessage.getType(), serialized);
        }
        else
        {
            LoginFailureMessage failureMessage;
            failureMessage.error_message = "User already logged in.";
            std::vector<uint8_t> serialized = failureMessage.serialize();
            server.sendPacket(client_fd, failureMessage.getType(), serialized);
        }
    }

    void handleMove(int client_fd, const std::vector<uint8_t> &payload)
    {
        MoveMessage message = MoveMessage::deserialize(payload);

        std::cout << "[MOVE] game_id: " << message.game_id
                  << ", uci_move: " << message.uci_move << std::endl;

        GameManager::getInstance().handleMove(client_fd, message.game_id, message.uci_move);
    }

    void handleAutoMatchRequest(int client_fd, const std::vector<uint8_t> &payload)
    {
        AutoMatchRequestMessage message = AutoMatchRequestMessage::deserialize(payload);

        std::cout << "[AUTO_MATCH_REQUEST] username: " << message.username << std::endl;

        NetworkServer::getInstance().setUsername(client_fd, message.username);

        GameManager::getInstance().addPlayerToQueue(client_fd);
    }

    void handleAutoMatchAccepted(int client_fd, const std::vector<uint8_t> &payload)
    {
        AutoMatchAcceptedMessage message = AutoMatchAcceptedMessage::deserialize(payload);

        std::cout << "[AUTO_MATCH_ACCEPTED] game_id: " << message.game_id << std::endl;

        GameManager::getInstance().handleAutoMatchAccepted(client_fd, message.game_id);
    }

    void handleAutoMatchDeclined(int client_fd, const std::vector<uint8_t> &payload)
    {
        AutoMatchDeclinedMessage message = AutoMatchDeclinedMessage::deserialize(payload);

        std::cout << "[AUTO_MATCH_DECLINED] game_id: " << message.game_id << std::endl;

        GameManager::getInstance().handleAutoMatchDeclined(client_fd, message.game_id);
    }

    void handleRequestPlayerList(int client_fd, const std::vector<uint8_t> &payload)
    {
        RequestPlayerListMessage message = RequestPlayerListMessage::deserialize(payload);

        std::cout << "[REQUEST_PLAYER_LIST]" << std::endl;

        DataStorage &storage = DataStorage::getInstance();
        std::unordered_map<std::string, UserModel> players = storage.getPlayerList();

        PlayerListMessage response;

        NetworkServer &server = NetworkServer::getInstance();

        for (const auto &pair : players)
        {
            PlayerListMessage::Player player;
            player.username = pair.second.username;
            player.elo = pair.second.elo;

            if (server.isUserLoggedIn(player.username)) {
                response.players.push_back(player);
                std::cout << "Logged in user: " << player.username << ", ELO: " << player.elo << std::endl;
            }
        }

        server.sendPacket(client_fd, response.getType(), response.serialize());
    }

    void handleChallengeRequest(int client_fd, const std::vector<uint8_t> &payload)
    {
        ChallengeRequestMessage message = ChallengeRequestMessage::deserialize(payload);
        NetworkServer &server = NetworkServer::getInstance();
        DataStorage &storage = DataStorage::getInstance();

        std::cout << "[CHALLENGE_REQUEST] from: " << server.getUsername(client_fd)
                  << ", to: " << message.to_username << std::endl;

        // Generated by GPT and currently working on this
        // if (server.isUserLoggedIn(message.to_username))
        // {
        //     int to_client_fd = server.getClientFD(message.to_username);

        //     ChallengeRequestMessage forwardMessage;
        //     forwardMessage.from_username = message.from_username;
        //     forwardMessage.to_username = message.to_username;
        //     std::vector<uint8_t> serialized = forwardMessage.serialize();
        //     server.sendPacket(to_client_fd, forwardMessage.getType(), serialized);
        // }
        // else
        // {
        //     ChallengeFailureMessage failureMessage;
        //     failureMessage.error_message = "User not logged in.";
        //     std::vector<uint8_t> serialized = failureMessage.serialize();
        //     server.sendPacket(client_fd, failureMessage.getType(), serialized);
        // }
    }
};

#endif // MESSAGE_HANDLER_HPP