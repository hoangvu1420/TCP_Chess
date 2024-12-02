#ifndef DATA_STORAGE_HPP
#define DATA_STORAGE_HPP

#include <string>
#include <unordered_map>
#include <mutex>
#include <chrono>

#include "../libraries/json.hpp"
#include "../common/json_handler.hpp"
#include "../common/const.hpp"

using json = nlohmann::json;

struct UserModel
{
    std::string username;
    uint16_t elo;

    json serialize() const
    {
        return 
        {
            {"elo", elo}
        };
    }

    static UserModel deserialize(const std::string &username, const json &j)
    {
        return UserModel
        {
            username,
            j.at("elo").get<uint16_t>()
        };
    }
};

struct MatchModel
{
    std::string game_id;
    std::string white_username;
    std::string black_username;
    std::string start_fen;
    std::chrono::time_point<std::chrono::steady_clock> start_time;

    struct Move
    {
        std::string uci_move;
        std::string fen;
        std::chrono::time_point<std::chrono::steady_clock> move_time;
    };

    std::vector<Move> moves;
    std::string result;
    std::string reason;

    json serialize() const
    {
        json j;
        j["white_username"] = white_username;
        j["black_username"] = black_username;
        j["start_fen"] = start_fen;
        j["start_time"] = start_time.time_since_epoch().count();

        json moves_json;
        for (const auto &move : moves)
        {
            json move_json;
            move_json["uci_move"] = move.uci_move;
            move_json["fen"] = move.fen;
            move_json["move_time"] = move.move_time.time_since_epoch().count();
            moves_json.push_back(move_json);
        }
        j["moves"] = moves_json;

        j["result"] = result;
        j["reason"] = reason;

        return j;
    }

    static MatchModel deserialize(const std::string &game_id, const json &j)
    {
        MatchModel game;
        game.game_id = game_id;
        game.white_username = j.at("white_username").get<std::string>();
        game.black_username = j.at("black_username").get<std::string>();
        game.start_fen = j.at("start_fen").get<std::string>();
        game.start_time = std::chrono::time_point<std::chrono::steady_clock>(std::chrono::nanoseconds(j.at("start_time").get<int64_t>()));

        for (const auto &move_json : j.at("moves"))
        {
            MatchModel::Move move;
            move.uci_move = move_json.at("uci_move").get<std::string>();
            move.fen = move_json.at("fen").get<std::string>();
            move.move_time = std::chrono::time_point<std::chrono::steady_clock>(std::chrono::nanoseconds(move_json.at("move_time").get<int64_t>()));
            game.moves.push_back(move);
        }

        game.result = j.at("result").get<std::string>();
        game.reason = j.at("reason").get<std::string>();

        return game;
    }
};

/**
 * @brief Lớp DataStorage là một Singleton quản lý dữ liệu người dùng trong ứng dụng TCP_Chess.
 *
 * Các chức năng chính:
 *
 * - Đăng ký người dùng mới với tên và điểm ELO mặc định.
 *
 * - Xác thực sự tồn tại của người dùng.
 *
 * - Lấy và cập nhật điểm ELO của người dùng.
 *
 * @note Lớp này không thể sao chép hoặc gán để đảm bảo chỉ có một instance được tồn tại.
 */
class DataStorage
{
public:
    static DataStorage &getInstance()
    {
        static DataStorage instance;
        return instance;
    }

    /**
     * Đăng ký một người dùng mới.
     *
     * @param username Tên người dùng cần đăng ký.
     * @param elo Điểm Elo khởi tạo (mặc định: Const::DEFAULT_ELO).
     * @return true nếu đăng ký thành công, false nếu tên người dùng đã tồn tại.
     */
    bool registerUser(const std::string &username, const uint16_t elo = Const::DEFAULT_ELO)
    {
        std::lock_guard<std::mutex> lock(users_mutex);

        if (users.find(username) != users.end())
        {
            return false; // Username đã tồn tại
        }

        users[username] = UserModel
        {
            username, 
            elo 
        }; 

        saveUsersData();

        return true;
    }

    /**
     * Kiểm tra tính hợp lệ của người dùng.
     *
     * @param username Tên người dùng cần xác thực.
     * @return Trả về true nếu người dùng tồn tại, ngược lại trả về false.
     */
    bool validateUser(const std::string &username)
    {
        std::lock_guard<std::mutex> lock(users_mutex);

        return users.find(username) != users.end();
    }

    uint16_t getUserELO(const std::string &username)
    {
        std::lock_guard<std::mutex> lock(users_mutex);

        auto it = users.find(username);
        if (it != users.end())
        {
            return it->second.elo;
        }
        return 0; // ELO mặc định nếu không tìm thấy
    }

    bool updateUserELO(const std::string &username, const uint16_t elo)
    {
        std::lock_guard<std::mutex> lock(users_mutex);

        auto it = users.find(username);
        if (it != users.end())
        {
            it->second.elo = elo;
            saveUsersData();
            return true;
        }
        return false;
    }

    std::unordered_map<std::string, UserModel> getPlayerList()
    {
        std::lock_guard<std::mutex> lock(users_mutex);
        return users;
    }

private:
    std::unordered_map<std::string, UserModel> users; // mapping username -> User
    std::mutex users_mutex;

    std::unordered_map<std::string, MatchModel> matches; // mapping game_id -> Match
    std::mutex matches_mutex;

    ~DataStorage() = default;
    DataStorage(const DataStorage &) = delete;
    DataStorage &operator=(const DataStorage &) = delete;

    DataStorage()
    {
        // Tải dữ liệu từ users.json
        json users_j = JSONHandler::readJSON("../data/users.json");
        for (auto it = users_j.begin(); it != users_j.end(); ++it)
        {
            std::string username = it.key();
            users[username] = UserModel::deserialize(username, it.value());
        }

        // Tải dữ liệu từ matches.json
        json matches_j = JSONHandler::readJSON("../data/matches.json");
        for (auto it = matches_j.begin(); it != matches_j.end(); ++it)
        {
            std::string game_id = it.key();
            matches[game_id] = MatchModel::deserialize(game_id, it.value());
        }
    }

    bool saveUsersData()
    {
        json j;
        for (const auto &[username, user] : users)
        {
            j[username] = user.serialize();
        }
        JSONHandler::writeJSON("../data/users.json", j);
        return true;
    }

    bool saveMatchesData()
    {
        json j;
        for (const auto &[game_id, match] : matches)
        {
            j[game_id] = match.serialize();
        }
        JSONHandler::writeJSON("../data/matches.json", j);
        return true;
    }
};

#endif // DATA_STORAGE_HPP