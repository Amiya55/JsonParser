#include <algorithm>
#include <cctype>
#include <csignal>
#include <exception>
#include <fstream>
#include <iostream>
#include <stack>
#include <stdexcept>
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
            if (value[i] == ' ' || i == value.size() - 1) {
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
            } else if (value[i] == '.' && countE == 0) {  // e的后面不能有小数点
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
    string tmp;
    vector<string> vec;

    string subStr;
    stack<Type> curTy;

    size_t commaCount = 0;
    size_t index = 1;
    while (index < obj.size() - 1) {
        // 遇到对象或者数组的情况
        if ((obj[index] == '{' || obj[index] == '[')) {
            obj[index] == '{' ? curTy.push(OBJ) : curTy.push(ARR);
        } else if ((obj[index] == '}' || obj[index] == ']') && !curTy.empty()) {
            Type lastType = curTy.top();
            curTy.pop();

            if (curTy.empty()) {
                subStr.push_back(obj[index]);
                lastType == OBJ ? check_syntax(subStr, OBJ)
                                : check_syntax(subStr, ARR);
                subStr.clear();
            }
        }

        if (!curTy.empty()) {
            subStr.push_back(obj[index]);
            tmp.push_back(obj[index]);
        } else {
            if (obj[index] == ',') {
                // 为了配合逗号检查，如果一个字符串全是空格，我们认为它是非法逗号引起的无效字符串，不添加进入vec
                if (!tmp.empty() &&
                    tmp.find_first_not_of(" \t\n\r") != string::npos)
                    vec.push_back(tmp);
                ++commaCount;
                tmp.clear();
            } else {
                tmp.push_back(obj[index]);
            }
        }

        ++index;
        // 遇到最后一个字符，自动添加到数组，值得注意的是，如果最后一个字符串全是空格，则不添加
        if (index == obj.size() - 1 &&
            tmp.find_first_not_of(" \t\n\r") != string::npos)
            vec.push_back(tmp);
    }

    // 检查逗号是否合法
    if (!vec.empty() && commaCount >= vec.size()) {
        throw runtime_error("unexpected comma: " + obj);
    }

    // 检查语法
    for (int i = 0; i < vec.size(); ++i) {
        if (curType == OBJ) {
            size_t sep = vec[i].find_first_of(":");
            if (sep == string::npos) {
                throw runtime_error("expected value: " + vec[i]);
            }

            if (!check_key(vec[i].substr(0, sep))) {
                throw runtime_error("key must be string: " + vec[i]);
            }

            if (!check_value(vec[i].substr(sep + 1))) {
                throw runtime_error("unexpected value: " + vec[i]);
            }
        } else if (curType == ARR) {
            if (!check_value(vec[i])) {
                throw runtime_error("unexpected value: " + vec[i]);
            }
        }
    }

    // cout << obj << " " << curType << endl;

    // for (auto& e : vec) {
    //     cout << e << "|";
    // }
    // cout << endl;
}

string load_json() {
    fstream fs;
    fs.open("data.json", ios::in);
    // fs << "hello world" << endl;

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
        check_syntax(json, ARR);
    } catch (exception& e) {
        cout << e.what() << endl;
    }
    return 0;
}