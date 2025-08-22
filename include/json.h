#ifndef JSON_H
#define JSON_H

#include "config.h"
#include "json_type.h"
#include "lexer_parser.h"

#include <filesystem>
#include <fstream>
#include <ostream>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <utility>
namespace simple_json
{

class Json
{
  public:
    ~Json() = default;

    Json(const Json &) = default;
    Json(Json &&) = default;
    Json &operator=(const Json &) = default;
    Json &operator=(Json &&) = default;

    /**
     * @brief Generate an empty json object.
     *
     * @return Json - return an empty json object object.
     */
    [[nodiscard]] static Json JsonObj() noexcept;

    /**
     * @brief Generate an empty json array.
     *
     * @return Json - return an empty json array object.
     */
    [[nodiscard]] static Json JsonArr() noexcept;

    /**
     * @brief construct a json data struct by reading a json file
     *
     * @param file_path - path of json file
     * @return Json - return specific json data struct
     */
    template <typename T, typename = enableIfString<T>> [[nodiscard]] static Json FromFile(T &&file_path)
    {
        const std::filesystem::path json_path(std::forward<T>(file_path));
        if (json_path.empty())
        {
            // 文件路径不存在，抛异常
            throw std::invalid_argument(INVALID_PATH);
        }

        if (!json_path.has_extension() || json_path.extension() != ".json")
        {
            // 文件不是json文件，抛异常
            throw std::invalid_argument(INVALID_FILE);
        }

        std::string json_str;

        std::fstream fs;
        fs.open(json_path, std::ios::in);
        if (!fs.is_open())
        {
            throw std::runtime_error(FAILED_OPEN_FILE);
        }

        std::string buffer;
        while (std::getline(fs, buffer))
        {
            json_str += buffer + '\n';
        }

        return Json(std::move(json_str));
    }

    /**
     * @brief construct a json data struct from specific string.
     *
     * @param json_str - specific string that contains a json data struct
     * @return Json - return specific json data struct object
     */
    template <typename T, typename = enableIfString<T>> static Json FromString(T &&json_str)
    {
        return Json(std::forward<T>(json_str));
    }

    template <typename T, typename = std::enable_if_t<std::is_integral_v<T> || std::is_constructible_v<std::string, T>>>
    JsonValue &operator[](T &&index)
    {
        if constexpr (std::is_integral_v<T>)
        {
            if (data_.GetType() != JsonType::Array)
            {
                throw std::invalid_argument(ERR_ARRAY_INTEGRAL);
            }

            std::vector<JsonValue> array = data_.GetVal<JsonType::Array>();
            if (index < 0 || index >= array.size())
            {
                throw std::out_of_range(ERR_OUT_OF_RANGE);
            }

            return data_.GetVal<JsonType::Array>()[std::forward<T>(index)];
        }
        else
        {
            if (data_.GetType() != JsonType::Object)
            {
                throw std::invalid_argument(ERR_OBJECT_STRING);
            }

            std::string key(std::forward<T>(index));
            std::unordered_map<std::string, JsonValue> object = data_.GetVal<JsonType::Object>();
            if (object.find(key) == object.end())
            {
                throw std::invalid_argument(ERR_INVALID_KEY);
            }

            return data_.GetVal<JsonType::Object>()[std::move(key)];
        }
    }

  private:
    JsonValue data_;

    /**
     * @brief Construct a new Json data struct
     *
     * @param json_str - a string that contains json data struct
     */
    template <typename T, typename = enableIfString<T>> explicit Json(T &&json_str)
    {
        Lexer lexer(std::forward<T>(json_str));
        Parser parser(std::move(lexer.GetToken()));

        data_ = std::move(parser.GetJsonAst());
    }

    /**
     * @brief easily print json data struct on console by using std::cout.
     *
     * @param os - ostream object.
     * @param json - json data struct object you want to print on console.
     * @return std::ostream& - reference to ostream object.
     */
    friend std::ostream &operator<<(std::ostream &os, const Json &json);
};

} // namespace simple_json

#endif // JSON_H