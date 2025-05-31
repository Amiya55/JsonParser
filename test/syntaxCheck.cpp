#include <cctype>
#include <exception>
#include <fstream>
#include <iostream>
#include <stack>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

enum Type { OBJ, ARR, STR, INT, FLT };

void check_str(string& str) {}

void check_num(string& num) {}

// 检查Json对象中的key的合法性
bool check_key(string&& key) {
    bool le = false;
    bool ri = false;
    for (int i = 0; i < key.size(); ++i) {
        if (le == false) {
            if (key[i] == '\"')
                le = true;
            else if (key[i] != ' ')
                return false;
        } else if (le == true && ri == false) {
            if (key[i] == '\"' && key[i - 1] != '\\')
                ri = true;
        } else if (le == true && ri == true) {
            if (key[i] != ' ')
                return false;
        }
    }
    if (le == false || ri == false)
        return false;

    return true;
}

// 检查Json对象中的value的合法性
bool check_value(const string& value) {
    if (value.find_first_not_of(" \t\n\r") == string::npos) {
        return false;
    }

    stack<Type> type;
    int countE = 0;
    for (int i = 0; i < value.size(); ++i) {
        if (type.empty()) {
            if (value[i] == '\"') {
                // 如果是"并且栈为空，则认为进入了字符串
                type.push(STR);
            } else if (value[i] == '{' || value[i] == '[') {
                // 还要判断一下会不会出现{"C++"}1234这种情况
                size_t last = value.find_last_not_of(" \t\n\r");
                if (value[last] == '}' || value[last] == ']')
                    return true;
                else
                    return false;
            } else if (isdigit(value[i])) {
                type.push(INT);
            } else if (value[i] != '\"' && value[i] != ' ') {
                // 如果在引号之外出现了字符，那么我们直接判断这个字符串是不是null，不是，则认为value值非法
                size_t end = value.find_last_not_of(" \t\n\r");
                if (value.substr(i, end + 1) == "null")
                    return true;
                else
                    return false;
            }
        } else if (type.top() == STR) {
            if (value[i] == '\"' && value[i - 1] != '\\') {
                // 字符为"并且栈中标记为STR，并且前一位不是\，就认为字符串结束，value值结束
                type.pop();
                if (value.substr(i + 1).find_first_not_of(" \t\n\r") !=
                    string::npos)
                    // 假设value值结束还存在非空格字符，则认为value值非法
                    return false;
            }
        } else if ((type.top() == INT || type.top() == FLT)) {
            if (value[i] == ' ') {
                // 如果当前的类型为float和int，如果当前字符为空格，则认为value值已经结束，可以收尾了
                type.pop();
                if (value.substr(i + 1).find_first_not_of(" \t\n\r") !=
                    string::npos)
                    // 如果value值结束了还有字符，那么肯定是因为原对象中缺了逗号，当前值非法
                    return false;
            } else if (value[i] == 'e') {
                // 如果当前的类型为float或者int，并且当前字符不是数字，不是字母e，或者e的数量大于一，都认为value值非法
                ++countE;
                if (countE > 1 || i == value.size() - 1 ||
                    !isdigit(value[i + 1]))
                    return false;
            } else if (value[i] == '.' && countE == 0) {
                // e的后面不能有小数点
                if (type.top() == INT) {
                    if (i == value.size() - 1 || !isdigit(value[i + 1])) {
                        // 如果小数点后面没有字符或者字符不是数字，直接返回false
                        return false;
                    } else {
                        type.pop();
                        type.push(FLT);
                    }
                } else {
                    // 如果已经是float类型，这时候再出现一次小数点，那么认为当前value非法
                    return false;
                }
            } else if (!isdigit(value[i])) {
                // 除了以上特殊字符情况，当前的字符不是数字，认为value值非法
                return false;
            }
        }
    }

    // 如果最后出循环，栈里面还有元素，并且元素还是STR，意味着字符串的引号未关闭，value值非法
    if (!type.empty() && type.top() == STR)
        return false;

    return true;
}

void check_syntax(string& obj, Type curType) {
    string value;
    string
        subContainer; // 当前字符串中存在的子容器: {}或者[]，用作递归调用参数

    stack<Type> types; // 当前字符所在的数据结构: "", {}, []
    vector<string> values; // obj字符串以逗号(,)分割开的子字符串

    size_t countComma = 0;
    bool inContainer = false;

    for (int i = 1; i < obj.size() - 1; ++i) {
        if (obj[i] == '\"') {
            if (types.empty() || types.top() != STR)
                types.push(STR);
            else if (types.top() == STR && obj[i - 1] != '\\')
                types.pop();
        }

        if (obj[i] == '{' || obj[i] == '[') {
            if (types.empty() || types.top() != STR) {
                obj[i] == '{' ? types.push(OBJ) : types.push(ARR);
                inContainer = true;
            }
        } else if (obj[i] == '}' || obj[i] == ']') {
            if (!types.empty() && types.top() != STR) {
                const Type lastType = types.top();
                types.pop();
                // 当栈为空，说明我们最初遇到的容器结束了，可以递归调用，传入这个容器字符串，进一步检查
                if (types.empty()) {
                    subContainer.push_back(obj[i]);
                    lastType == OBJ
                        ? check_syntax(subContainer, OBJ)
                        : check_syntax(subContainer, ARR);
                    inContainer = false;
                    subContainer.clear();
                }
            }
        }

        if (obj[i] == ',' && types.empty()) {
            ++countComma; // 只有当types栈为空时才加加这个值，否则可能是字符串或者容器内的逗号
            if (!value.empty() &&
                value.find_first_not_of(" \t\n\r") != string::npos)
                values.push_back(value);
            value.clear();
        } else {
            if (inContainer)
                subContainer.push_back(obj[i]);
            value.push_back(obj[i]);
        }

        if (i == obj.size() - 2) {
            // 遇到json字符串结尾，push最后的数据到数组中
            if (value.find_first_not_of(" \t\n\r") != string::npos)
                values.push_back(value);

            // 最后检查types数组是否为空，如果不为空，那么说明""，{}或者[]存在没有关闭的情况
            if (!types.empty())
                throw runtime_error(
                    "Syntax Error, string or container does not closed: " +
                    obj);
        }
    }

    if (!values.empty() && countComma >= values.size()) {
        throw runtime_error("unexpected comma: " + obj);
    }

    // 检查语法
    for (int i = 0; i < values.size(); ++i) {
        if (curType == OBJ) {
            size_t sep = values[i].find_first_of(":");
            if (sep == string::npos) {
                throw runtime_error("expected value: " + values[i]);
            }

            if (!check_key(values[i].substr(0, sep))) {
                throw runtime_error("key must be string: " + values[i]);
            }

            if (!check_value(values[i].substr(sep + 1))) {
                throw runtime_error("unexpected value: " + values[i]);
            }
        } else if (curType == ARR) {
            if (!check_value(values[i])) {
                throw runtime_error("unexpected value: " + values[i]);
            }
        }
    }
}

string load_json() {
    fstream fs;
    fs.open("data.json", ios::in);
    if (!fs.is_open()) {
        throw runtime_error("cannot find json file! check the path");
    }

    string str;
    string tmp;
    while (getline(fs, tmp))
        str += tmp;
    fs.close();

    // cout << str << endl;
    return str;
}

int main() {
    try {
        string json(load_json());
        check_syntax(json, OBJ);
    } catch (exception& e) {
        cout << e.what() << endl;
    }
    return 0;
}
