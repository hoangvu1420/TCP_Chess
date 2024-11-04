// message.hpp
#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <vector>
#include <memory>

#include "utils.hpp"
#include "protocol.hpp"

// RegisterMessage ========================================================================================
/*
Send from client to server to register a new user.

Payload structure:
    - uint8_t username_length (1 byte)
    - char[username_length] username (username_length bytes)
*/

struct RegisterMessage
{
    std::string username;

    MessageType getType() const
    {
        return MessageType::REGISTER;
    }

    std::vector<uint8_t> serialize() const
    {
        std::vector<uint8_t> payload;
        payload.push_back(static_cast<uint8_t>(username.size()));
        payload.insert(payload.end(), username.begin(), username.end());
        return payload;
    }

    static RegisterMessage deserialize(const std::vector<uint8_t> &payload)
    {
        RegisterMessage message;
        size_t pos = 0;
        uint8_t username_length = payload[pos++];
        message.username = std::string(payload.begin() + pos, payload.begin() + pos + username_length);
        return message;
    }
};

// RegisterSuccessMessage =================================================================================
/*
Send from server to client to notify that the registration was successful.

Payload structure:
    - uint8_t username_length (1 byte)
    - char[username_length] username (username_length bytes)
    - uint16_t elo (2 bytes)
*/

struct RegisterSuccessMessage
{
    std::string username;
    uint16_t elo;

    MessageType getType() const
    {
        return MessageType::REGISTER_SUCCESS;
    }

    std::vector<uint8_t> serialize() const
    {
        std::vector<uint8_t> payload;
        payload.push_back(static_cast<uint8_t>(username.size()));
        payload.insert(payload.end(), username.begin(), username.end());
        std::vector<uint8_t> elo_bytes = to_big_endian_16(elo);
        payload.insert(payload.end(), elo_bytes.begin(), elo_bytes.end());
        return payload;
    }

    static RegisterSuccessMessage deserialize(const std::vector<uint8_t> &payload)
    {
        RegisterSuccessMessage message;
        size_t pos = 0;
        uint8_t username_length = payload[pos++];
        message.username = std::string(payload.begin() + pos, payload.begin() + pos + username_length);
        pos += username_length;
        message.elo = from_big_endian_16(payload, pos);
        return message;
    }
};

// RegisterFailureMessage =================================================================================
/*
Send from server to client to notify that the registration was unsuccessful.

Payload structure:
    - uint8_t error_message_length (1 byte)
    - char[error_message_length] error_message (error_message_length bytes)
*/

struct RegisterFailureMessage
{
    std::string error_message;

    MessageType getType() const
    {
        return MessageType::REGISTER_FAILURE;
    }

    std::vector<uint8_t> serialize() const
    {
        std::vector<uint8_t> payload;
        payload.push_back(static_cast<uint8_t>(error_message.size()));
        payload.insert(payload.end(), error_message.begin(), error_message.end());
        return payload;
    }

    static RegisterFailureMessage deserialize(const std::vector<uint8_t> &payload)
    {
        RegisterFailureMessage message;
        size_t pos = 0;
        uint8_t error_message_length = payload[pos++];
        message.error_message = std::string(payload.begin() + pos, payload.begin() + pos + error_message_length);
        return message;
    }
};

// LoginMessage ============================================================================================
/*
Send from client to server to login.

Payload structure:
    - uint8_t username_length (1 byte)
    - char[username_length] username (username_length bytes)
*/

struct LoginMessage
{
    std::string username;

    MessageType getType() const
    {
        return MessageType::LOGIN;
    }

    std::vector<uint8_t> serialize() const
    {
        std::vector<uint8_t> payload;
        payload.push_back(static_cast<uint8_t>(username.size()));
        payload.insert(payload.end(), username.begin(), username.end());
        return payload;
    }

    static LoginMessage deserialize(const std::vector<uint8_t> &payload)
    {
        LoginMessage message;
        size_t pos = 0;
        uint8_t username_length = payload[pos++];
        message.username = std::string(payload.begin() + pos, payload.begin() + pos + username_length);
        return message;
    }
};

// LoginSuccessMessage ======================================================================================
/*
Send from server to client to notify that the login was successful.

Payload structure:
    - uint8_t username_length (1 byte)
    - char[username_length] username (username_length bytes)
    - uint16_t elo (2 bytes)
*/

struct LoginSuccessMessage
{
    std::string username;
    uint16_t elo;

    MessageType getType() const
    {
        return MessageType::LOGIN_SUCCESS;
    }

    std::vector<uint8_t> serialize() const
    {
        std::vector<uint8_t> payload;
        payload.push_back(static_cast<uint8_t>(username.size()));
        payload.insert(payload.end(), username.begin(), username.end());
        std::vector<uint8_t> elo_bytes = to_big_endian_16(elo);
        payload.insert(payload.end(), elo_bytes.begin(), elo_bytes.end());
        return payload;
    }

    static LoginSuccessMessage deserialize(const std::vector<uint8_t> &payload)
    {
        LoginSuccessMessage message;
        size_t pos = 0;
        uint8_t username_length = payload[pos++];
        message.username = std::string(payload.begin() + pos, payload.begin() + pos + username_length);
        pos += username_length;
        message.elo = from_big_endian_16(payload, pos);
        return message;
    }
};

// LoginFailureMessage ========================================================================================
/*
Send from server to client to notify that the login was unsuccessful.

Payload structure:
    - uint8_t error_message_length (1 byte)
    - char[error_message_length] error_message (error_message_length bytes)
*/

struct LoginFailureMessage
{
    std::string error_message;

    MessageType getType() const
    {
        return MessageType::LOGIN_FAILURE;
    }

    std::vector<uint8_t> serialize() const
    {
        std::vector<uint8_t> payload;
        payload.push_back(static_cast<uint8_t>(error_message.size()));
        payload.insert(payload.end(), error_message.begin(), error_message.end());
        return payload;
    }

    static LoginFailureMessage deserialize(const std::vector<uint8_t> &payload)
    {
        LoginFailureMessage message;
        size_t pos = 0;
        uint8_t error_message_length = payload[pos++];
        message.error_message = std::string(payload.begin() + pos, payload.begin() + pos + error_message_length);
        return message;
    }
};

// GameStartMessage ========================================================================================
/*
Send from server to clients to notify that a new game has started.

Payload structure:
    - uint8_t game_id_length (1 byte)
    - char[game_id_length] game_id (game_id_length bytes)
    - uint8_t player1_username_length (1 byte)
    - char[player1_username_length] player1_username (player1_username_length bytes)
    - uint8_t player2_username_length (1 byte)
    - char[player2_username_length] player2_username (player2_username_length bytes)
    - uint8_t starting_player_username_length (1 byte)
    - char[starting_player_username_length] starting_player_username (starting_player_username_length bytes)
    - uint8_t fen_length (1 byte)
    - char[fen_length] fen (fen_length bytes)
*/

struct GameStartMessage
{
    std::string game_id;
    std::string player1_username;
    std::string player2_username;
    std::string starting_player_username;
    std::string fen;

    MessageType getType() const
    {
        return MessageType::GAME_START;
    }

    std::vector<uint8_t> serialize() const
    {
        std::vector<uint8_t> payload;
        payload.push_back(static_cast<uint8_t>(game_id.size()));
        payload.insert(payload.end(), game_id.begin(), game_id.end());
        payload.push_back(static_cast<uint8_t>(player1_username.size()));
        payload.insert(payload.end(), player1_username.begin(), player1_username.end());
        payload.push_back(static_cast<uint8_t>(player2_username.size()));
        payload.insert(payload.end(), player2_username.begin(), player2_username.end());
        payload.push_back(static_cast<uint8_t>(starting_player_username.size()));
        payload.insert(payload.end(), starting_player_username.begin(), starting_player_username.end());
        payload.push_back(static_cast<uint8_t>(fen.size()));
        payload.insert(payload.end(), fen.begin(), fen.end());
        return payload;
    }

    static GameStartMessage deserialize(const std::vector<uint8_t> &payload)
    {
        GameStartMessage message;
        size_t pos = 0;
        uint8_t game_id_length = payload[pos++];
        message.game_id = std::string(payload.begin() + pos, payload.begin() + pos + game_id_length);
        pos += game_id_length;
        uint8_t player1_username_length = payload[pos++];
        message.player1_username = std::string(payload.begin() + pos, payload.begin() + pos + player1_username_length);
        pos += player1_username_length;
        uint8_t player2_username_length = payload[pos++];
        message.player2_username = std::string(payload.begin() + pos, payload.begin() + pos + player2_username_length);
        pos += player2_username_length;
        uint8_t starting_player_username_length = payload[pos++];
        message.starting_player_username = std::string(payload.begin() + pos, payload.begin() + pos + starting_player_username_length);
        pos += starting_player_username_length;
        uint8_t fen_length = payload[pos++];
        message.fen = std::string(payload.begin() + pos, payload.begin() + pos + fen_length);
        return message;
    }
};

// MoveMessage ====================================================================================================
/*
Send from client to server to make a move.

Payload structure:
    - uint8_t game_id_length (1 byte)
    - char[game_id_length] game_id (game_id_length bytes)
    - uint8_t uci_move_length (1 byte)
    - char[uci_move_length] uci_move (uci_move_length bytes)
*/

struct MoveMessage
{
    std::string game_id;
    std::string uci_move;

    MessageType getType() const
    {
        return MessageType::MOVE;
    }

    std::vector<uint8_t> serialize() const
    {
        std::vector<uint8_t> payload;
        payload.push_back(static_cast<uint8_t>(game_id.size()));
        payload.insert(payload.end(), game_id.begin(), game_id.end());
        payload.push_back(static_cast<uint8_t>(uci_move.size()));
        payload.insert(payload.end(), uci_move.begin(), uci_move.end());
        return payload;
    }

    static MoveMessage deserialize(const std::vector<uint8_t> &payload)
    {
        MoveMessage message;
        size_t pos = 0;
        uint8_t game_id_length = payload[pos++];
        message.game_id = std::string(payload.begin() + pos, payload.begin() + pos + game_id_length);
        pos += game_id_length;
        uint8_t uci_move_length = payload[pos++];
        message.uci_move = std::string(payload.begin() + pos, payload.begin() + pos + uci_move_length);
        return message;
    }
};

// MoveErrorMessage ========================================================================================
/*
Send from server to client to notify that the move was invalid.

Payload structure:
    - uint8_t game_id_length (1 byte)
    - char[game_id_length] game_id (game_id_length bytes)
    - uint8_t error_message_length (1 byte)
    - char[error_message_length] error_message (error_message_length bytes)
*/

struct MoveErrorMessage
{
    std::string game_id;
    std::string error_message;

    MessageType getType() const
    {
        return MessageType::MOVE_ERROR;
    }

    std::vector<uint8_t> serialize() const
    {
        std::vector<uint8_t> payload;
        payload.push_back(static_cast<uint8_t>(game_id.size()));
        payload.insert(payload.end(), game_id.begin(), game_id.end());
        payload.push_back(static_cast<uint8_t>(error_message.size()));
        payload.insert(payload.end(), error_message.begin(), error_message.end());
        return payload;
    }

    static MoveErrorMessage deserialize(const std::vector<uint8_t> &payload)
    {
        MoveErrorMessage message;
        size_t pos = 0;
        uint8_t game_id_length = payload[pos++];
        message.game_id = std::string(payload.begin() + pos, payload.begin() + pos + game_id_length);
        pos += game_id_length;
        uint8_t error_message_length = payload[pos++];
        message.error_message = std::string(payload.begin() + pos, payload.begin() + pos + error_message_length);
        return message;
    }
};

// GameStatusUpdateMessage ========================================================================================
/*
Send from server to clients to notify that the game status has been updated.

Payload structure:
    - uint8_t game_id_length (1 byte)
    - char[game_id_length] game_id (game_id_length bytes)

    - uint8_t fen_length (1 byte)
    - char[fen_length] fen (fen_length bytes)

    - uint8_t current_turn_username_length (1 byte)
    - char[current_turn_username_length] current_turn_username (current_turn_username_length bytes)

    - uint8_t is_game_over (1 byte) (0: false, 1: true)

    - uint8_t message_length (1 byte)
    - char[message_length] message (message_length bytes)
*/

struct GameStatusUpdateMessage
{
    std::string game_id;
    std::string fen;
    std::string current_turn_username;
    uint8_t is_game_over;
    std::string message;

    MessageType getType() const
    {
        return MessageType::GAME_STATUS_UPDATE;
    }

    std::vector<uint8_t> serialize() const
    {
        std::vector<uint8_t> payload;

        payload.push_back(static_cast<uint8_t>(game_id.size()));
        payload.insert(payload.end(), game_id.begin(), game_id.end());

        payload.push_back(static_cast<uint8_t>(fen.size()));
        payload.insert(payload.end(), fen.begin(), fen.end());

        payload.push_back(static_cast<uint8_t>(current_turn_username.size()));
        payload.insert(payload.end(), current_turn_username.begin(), current_turn_username.end());

        payload.push_back(is_game_over);

        payload.push_back(static_cast<uint8_t>(message.size()));
        payload.insert(payload.end(), message.begin(), message.end());
        
        return payload;
    }

    static GameStatusUpdateMessage deserialize(const std::vector<uint8_t> &payload)
    {
        GameStatusUpdateMessage message;

        size_t pos = 0;
        uint8_t game_id_length = payload[pos++];
        message.game_id = std::string(payload.begin() + pos, payload.begin() + pos + game_id_length);

        pos += game_id_length;
        uint8_t fen_length = payload[pos++];
        message.fen = std::string(payload.begin() + pos, payload.begin() + pos + fen_length);

        pos += fen_length;
        uint8_t current_turn_username_length = payload[pos++];
        message.current_turn_username = std::string(payload.begin() + pos, payload.begin() + pos + current_turn_username_length);

        pos += current_turn_username_length;
        message.is_game_over = payload[pos++];

        uint8_t message_length = payload[pos++];
        message.message = std::string(payload.begin() + pos, payload.begin() + pos + message_length);

        return message;
    }
};

// GameEndMessage ========================================================================================
/*
Send from server to clients to notify that the game has ended.

Payload structure:
    - uint8_t game_id_length (1 byte)
    - char[game_id_length] game_id (game_id_length bytes)

    - uint8_t winner_username_length (1 byte)
    - char[winner_username_length] winner_username (winner_username_length bytes)

    - uint8_t reason_length (1 byte)
    - char[reason_length] reason (reason_length bytes)

    - uint16_t half_moves_count (2 bytes)
*/

struct GameEndMessage
{
    std::string game_id;
    std::string winner_username;
    std::string reason;
    uint16_t half_moves_count;

    MessageType getType() const
    {
        return MessageType::GAME_END;
    }

    std::vector<uint8_t> serialize() const
    {
        std::vector<uint8_t> payload;

        payload.push_back(static_cast<uint8_t>(game_id.size()));
        payload.insert(payload.end(), game_id.begin(), game_id.end());

        payload.push_back(static_cast<uint8_t>(winner_username.size()));
        payload.insert(payload.end(), winner_username.begin(), winner_username.end());

        payload.push_back(static_cast<uint8_t>(reason.size()));
        payload.insert(payload.end(), reason.begin(), reason.end());

        std::vector<uint8_t> full_moves_count_bytes = to_big_endian_16(half_moves_count);
        payload.insert(payload.end(), full_moves_count_bytes.begin(), full_moves_count_bytes.end());

        return payload;
    }

    static GameEndMessage deserialize(const std::vector<uint8_t> &payload)
    {
        GameEndMessage message;

        size_t pos = 0;
        uint8_t game_id_length = payload[pos++];
        message.game_id = std::string(payload.begin() + pos, payload.begin() + pos + game_id_length);

        pos += game_id_length;
        uint8_t winner_username_length = payload[pos++];
        message.winner_username = std::string(payload.begin() + pos, payload.begin() + pos + winner_username_length);
        
        pos += winner_username_length;
        uint8_t reason_length = payload[pos++];
        message.reason = std::string(payload.begin() + pos, payload.begin() + pos + reason_length);

        pos += reason_length;
        message.half_moves_count = from_big_endian_16(payload, pos);
        
        return message;
    }
};

// AutoMatchRequestMessage ========================================================================================
/*
Send from client to server to request an auto match.

Payload structure:
    - uint8_t username_length (1 byte)
    - char[username_length] username (username_length bytes)
*/

struct AutoMatchRequestMessage
{
    std::string username;

    MessageType getType() const
    {
        return MessageType::AUTO_MATCH_REQUEST;
    }

    std::vector<uint8_t> serialize() const
    {
        std::vector<uint8_t> payload;
        payload.push_back(static_cast<uint8_t>(username.size()));
        payload.insert(payload.end(), username.begin(), username.end());
        return payload;
    }

    static AutoMatchRequestMessage deserialize(const std::vector<uint8_t> &payload)
    {
        AutoMatchRequestMessage message;
        size_t pos = 0;
        uint8_t username_length = payload[pos++];
        message.username = std::string(payload.begin() + pos, payload.begin() + pos + username_length);
        return message;
    }
};

// AutoMatchFoundMessage ========================================================================================
/*
Send from server to clients to notify that an auto match has been found.

Payload structure:
    - uint8_t opponent_username_length (1 byte)
    - char[opponent_username_length] opponent_username (opponent_username_length bytes)
    - uint16_t opponent_elo (2 bytes)
    - uint8_t game_id_length (1 byte)
    - char[game_id_length] game_id (game_id_length bytes)
*/

struct AutoMatchFoundMessage
{
    std::string opponent_username;
    uint16_t opponent_elo;
    std::string game_id;

    MessageType getType() const
    {
        return MessageType::AUTO_MATCH_FOUND;
    }

    std::vector<uint8_t> serialize() const
    {
        std::vector<uint8_t> payload;
        payload.push_back(static_cast<uint8_t>(opponent_username.size()));
        payload.insert(payload.end(), opponent_username.begin(), opponent_username.end());
        std::vector<uint8_t> elo_bytes = to_big_endian_16(opponent_elo);
        payload.insert(payload.end(), elo_bytes.begin(), elo_bytes.end());
        payload.push_back(static_cast<uint8_t>(game_id.size()));
        payload.insert(payload.end(), game_id.begin(), game_id.end());
        return payload;
    }

    static AutoMatchFoundMessage deserialize(const std::vector<uint8_t> &payload)
    {
        AutoMatchFoundMessage message;
        size_t pos = 0;
        uint8_t opponent_username_length = payload[pos++];
        message.opponent_username = std::string(payload.begin() + pos, payload.begin() + pos + opponent_username_length);
        pos += opponent_username_length;
        message.opponent_elo = from_big_endian_16(payload, pos);
        pos += 2;
        uint8_t game_id_length = payload[pos++];
        message.game_id = std::string(payload.begin() + pos, payload.begin() + pos + game_id_length);
        return message;
    }
};

// AutoMatchAcceptedMessage ========================================================================================
/*
Send from client to server to accept an auto match.

Payload structure:
    - uint8_t game_id_length (1 byte)
    - char[game_id_length] game_id (game_id_length bytes)
*/

struct AutoMatchAcceptedMessage
{
    std::string game_id;

    MessageType getType() const
    {
        return MessageType::AUTO_MATCH_ACCEPTED;
    }

    std::vector<uint8_t> serialize() const
    {
        std::vector<uint8_t> payload;
        payload.push_back(static_cast<uint8_t>(game_id.size()));
        payload.insert(payload.end(), game_id.begin(), game_id.end());
        return payload;
    }

    static AutoMatchAcceptedMessage deserialize(const std::vector<uint8_t> &payload)
    {
        AutoMatchAcceptedMessage message;
        size_t pos = 0;
        uint8_t game_id_length = payload[pos++];
        message.game_id = std::string(payload.begin() + pos, payload.begin() + pos + game_id_length);
        return message;
    }
};

// AutoMatchDeclinedMessage ========================================================================================
/*
Send from client to server to decline an auto match.

Payload structure:
    - uint8_t game_id_length (1 byte)
    - char[game_id_length] game_id (game_id_length bytes)
*/

struct AutoMatchDeclinedMessage
{
    std::string game_id;

    MessageType getType() const
    {
        return MessageType::AUTO_MATCH_DECLINED;
    }

    std::vector<uint8_t> serialize() const
    {
        std::vector<uint8_t> payload;
        payload.push_back(static_cast<uint8_t>(game_id.size()));
        payload.insert(payload.end(), game_id.begin(), game_id.end());
        return payload;
    }

    static AutoMatchDeclinedMessage deserialize(const std::vector<uint8_t> &payload)
    {
        AutoMatchDeclinedMessage message;
        size_t pos = 0;
        uint8_t game_id_length = payload[pos++];
        message.game_id = std::string(payload.begin() + pos, payload.begin() + pos + game_id_length);
        return message;
    }
};

// MatchDeclinedNotificationMessage ========================================================================================
/*
Send from server to client to notify that the opponent has declined the match.

Payload structure:
    - uint8_t game_id_length (1 byte)
    - char[game_id_length] game_id (game_id_length bytes)
*/

struct MatchDeclinedNotificationMessage
{
    std::string game_id;

    MessageType getType() const
    {
        return MessageType::MATCH_DECLINED_NOTIFICATION;
    }

    std::vector<uint8_t> serialize() const
    {
        std::vector<uint8_t> payload;
        payload.push_back(static_cast<uint8_t>(game_id.size()));
        payload.insert(payload.end(), game_id.begin(), game_id.end());
        return payload;
    }

    static MatchDeclinedNotificationMessage deserialize(const std::vector<uint8_t> &payload)
    {
        MatchDeclinedNotificationMessage message;
        size_t pos = 0;
        uint8_t game_id_length = payload[pos++];
        message.game_id = std::string(payload.begin() + pos, payload.begin() + pos + game_id_length);
        return message;
    }
};

#endif // MESSAGE_HPP