#ifndef JSON_H
#define JSON_H

#include "config.h"
#include "json_type.h"
#include "lexer_parser.h"

#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <utility>

namespace simple_json
{

class Json
{
  public:
    ~Json() = default;

    /**
     * @brief Generate an empty json object.
     *
     * @return Json - return an empty json object object.
     */
    [[nodiscard]] static Json JsonObj() noexcept
    {
        return Json("{}");
    }

    /**
     * @brief Generate an empty json array.
     *
     * @return Json - return an empty json array object.
     */
    [[nodiscard]] static Json JsonArr() noexcept
    {
        return Json("[]");
    }

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
};

} // namespace simple_json

#endif // JSON_H