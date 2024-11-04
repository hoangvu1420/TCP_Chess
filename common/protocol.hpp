#ifndef PROTOCOL_HPP
#define PROTOCOL_HPP

#include <cstdint>
#include <vector>
#include <string>

#include "utils.hpp"

// Enum cho các loại thông điệp
enum class MessageType : uint8_t
{
    // Test
    TEST = 0x00,
    RESPONSE = 0x01,

    // Register
    REGISTER = 0x10,
    REGISTER_SUCCESS = 0x11,
    REGISTER_FAILURE = 0x12,

    // Login
    LOGIN = 0x20,
    LOGIN_SUCCESS = 0x21,
    LOGIN_FAILURE = 0x22,

    // Game
    GAME_START = 0x30,
    TURN_NOTIFICATION = 0x31,
    MOVE = 0x32,
    MOVE_ERROR = 0x33,
    GAME_STATUS_UPDATE = 0x34,
    GAME_END = 0x35,

    // Player list
    REQUEST_PLAYER_LIST = 0x40,
    PLAYER_LIST = 0x41,

    // Challenge
    CHALLENGE_REQUEST = 0x50,
    CHALLENGE_NOTIFICATION = 0x51,
    CHALLENGE_RESPONSE = 0x52,
    CHALLENGE_ACCEPTED = 0x53,
    CHALLENGE_DECLINED = 0x54,
    // Auto match
    AUTO_MATCH_REQUEST = 0x55,
    AUTO_MATCH_FOUND = 0x56,
    AUTO_MATCH_ACCEPTED = 0x57,
    AUTO_MATCH_DECLINED = 0x58,
    MATCH_DECLINED_NOTIFICATION = 0x59,
};

// Cấu trúc gói tin cơ bản
struct Packet
{
    MessageType type;
    uint16_t length;
    std::vector<uint8_t> payload;

    std::vector<uint8_t> serialize() const
    {
        std::vector<uint8_t> packet;
        packet.push_back(static_cast<uint8_t>(type));
        std::vector<uint8_t> length_bytes = to_big_endian_16(length);
        packet.insert(packet.end(), length_bytes.begin(), length_bytes.end());
        packet.insert(packet.end(), payload.begin(), payload.end());
        return packet;
    }
};

#endif // PROTOCOL_HPP