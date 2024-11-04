#ifndef DATA_STORAGE_HPP
#define DATA_STORAGE_HPP

#include <string>
#include <unordered_map>
#include <mutex>

#include "../libraries/json.hpp"
#include "../common/json_handler.hpp"
#include "../common/const.hpp"

using json = nlohmann::json;

struct User
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

    static User deserialize(std::string username, const json &j)
    {
        return User
        {
            username,
            j.at("elo").get<uint16_t>()
        };
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

    void dispose()
    {
        delete &getInstance();
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
        std::lock_guard<std::mutex> lock(mtx);

        if (users.find(username) != users.end())
        {
            return false; // Username đã tồn tại
        }

        users[username] = User
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
        std::lock_guard<std::mutex> lock(mtx);

        return users.find(username) != users.end();
    }

    uint16_t getUserELO(const std::string &username)
    {
        std::lock_guard<std::mutex> lock(mtx);

        auto it = users.find(username);
        if (it != users.end())
        {
            return it->second.elo;
        }
        return 0; // ELO mặc định nếu không tìm thấy
    }

    bool updateUserELO(const std::string &username, const uint16_t elo)
    {
        std::lock_guard<std::mutex> lock(mtx);

        auto it = users.find(username);
        if (it != users.end())
        {
            it->second.elo = elo;
            saveUsersData();
            return true;
        }
        return false;
    }

private:
    std::unordered_map<std::string, User> users; // mapping username -> User
    std::mutex mtx;

    DataStorage()
    {
        // Tải dữ liệu từ users.json
        json j = JSONHandler::readJSON("../data/users.json");
        for (auto it = j.begin(); it != j.end(); ++it)
        {
            std::string username = it.key();
            uint16_t elo = it.value()["elo"];
            users[username] = User::deserialize(username, it.value());
        }
    }

    ~DataStorage() = default;

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

    DataStorage(const DataStorage &) = delete;
    DataStorage &operator=(const DataStorage &) = delete;
};

#endif // DATA_STORAGE_HPP