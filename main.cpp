﻿// Visual Studio 2017
// ISO C++ 最新草案标准 (/std:c++latest)
// 编译时应当选择64位编译
// 编译时应当编译Release版本
// 运行时库选择MT
// 编译完成后应当在 Win 7 64位电脑上进行简单测试：建立一个空目录，将程序拷贝-
// 到空目录，双击运行，看是否有不兼容警告

// https://github.com/ngzqqb/simple_change_dir_name

#include <list>
#include <regex>
#include <array>
#include <string>
#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>
#include <filesystem>
#include <string_view>

#include <exception>
#include <stdexcept>

using namespace std::string_literals;
using namespace std::string_view_literals;

/* 单线程，获得std::wcout */
inline auto & wcout_() {
    if (std::wcout.good()) {
        return std::wcout;
    }
    std::wcout.clear();
    std::wcout << std::endl;
    return std::wcout;
}

/* 单线程，获得std::cout */
inline auto & cout_() {
    if (std::cout.good()) {
        return std::cout;
    }
    std::cout.clear();
    std::cout << std::endl;
    return std::cout;
}

/* 单线程，执行类 */
class Duty {
    using Path = std::filesystem::path;
    const Path thisRootPath;
public:

    template<typename T>
    inline Duty(T && arg) :
        thisRootPath(std::forward<T>(arg)) {
    }

private:

    class Dir {
    public:

        const Path dirName;
        std::wstring dirNameString;
        std::wstring replaceDirName;
        std::array<int, 4> dirNameInt{ 0,0,0,0 };
        int minFileIndex{ 0 };

        /* 构造执行目录 */
        template<typename T>
        inline Dir(T && arg) :
            dirName(std::forward<T>(arg)) {
            dirNameString = dirName.filename().wstring();
            replaceDirName = dirNameString;
        }

        /* 检查文件名格式是否符合要求 */
        inline bool isFormatDir() const {
            const auto & varDirName = dirNameString;
            {/* 符合要求 ... 根据文件名进一步处理 */
                const static std::wregex varMatchRegex{ LR"([A-Z][0-9]{3}-[0-9]{3}-[0-9]{3}-[0-9]{3})" };
                if (std::regex_match(varDirName, varMatchRegex)) {
                    return true;
                }
            }
            {/* 符合要求 ... 不处理 */
                const static std::wregex varMatchRegex{ LR"([A-Z][0-9]{3}-[0-9]{3}-[0-9]{3})" };
                if (std::regex_match(varDirName, varMatchRegex)) {
                    return false;
                }
            }
            {/* 轻微错误 ... 根据文件名进一步处理 */
                const static std::wregex varMatchRegex{ LR"([A-Z][0-9]{1,3}-[0-9]{1,3}-[0-9]{1,3}-[0-9]{1,3})" };
                if (std::regex_match(varDirName, varMatchRegex)) {
                    wcout_() << varDirName << LR"( 此目录名错误，将被替换 !!! )" << std::endl;
                    return true;
                }
            }
            /* 完全错误 ... 无法处理 */
            wcout_() << varDirName << LR"( 错误的目录名 ??? )" << std::endl;
            return false;
        }

        /* 重命名文件夹 */
        inline void rename() const try {
            if (replaceDirName != dirNameString) {
                auto varReplaceName = dirName;
                std::filesystem::rename(dirName,
                    varReplaceName.replace_filename(replaceDirName));
            }
        } catch (const std::exception & e) {
            cout_() << e.what() << std::endl;
        }

    private:

        /* wstring 转 int */
        inline static int toInt(const std::wstring & arg) try {
            return std::stoi(arg);
        } catch (...) {
            return 0;
        }

        /* 数字格式：[0-9][0-9][0-9] */
        template<typename T>
        inline static std::wstring formatWstring(T && arg) {
            if (arg.size() == 2) {
                return LR"(0)"s + std::forward<T>(arg);
            } else if (arg.size() == 1) {
                return LR"(00)"s + std::forward<T>(arg);
            }
            return std::forward<T>(arg);
        }

        /* int 转 wstring */
        inline static std::wstring toWstring(const int & arg) {
            return formatWstring(std::to_wstring(arg));
        }

        /* 将数字强制转为 [0-9][0-9][0-9] 格式 */
        inline void recheckDirName() {
            /*                                             1      2            3                 4            5*/
            const static std::wregex varMatchRegex{ LR"(([A-Z])([0-9]{1,3})-([0-9]{1,3})-([0-9]{1,3})-([0-9]{1,3}))" };
            std::wsmatch varMatch;
            std::regex_match(dirNameString, varMatch, varMatchRegex);
            auto varFormatString = varMatch[1].str()
                + formatWstring(varMatch[2].str())
                + LR"(-)"s
                + formatWstring(varMatch[3].str())
                + LR"(-)"s
                + formatWstring(varMatch[4].str())
                + LR"(-)"s
                + formatWstring(varMatch[5].str());
            dirNameInt[0] = toInt(varMatch[2]);
            dirNameInt[1] = toInt(varMatch[3]);
            dirNameInt[2] = toInt(varMatch[4]);
            dirNameInt[3] = toInt(varMatch[5]);
            if (varFormatString == dirNameString) {
                return;
            }
            replaceDirName = std::move(varFormatString);
        }
    public:
        /* 计算完整的重命名目录名称 */
        inline void fullConstruct() {
            recheckDirName();
            int varReplaceName = 0;
            {
                std::vector<int> varFileNames;
                {
                    std::filesystem::directory_iterator varPos{ dirName };
                    const std::filesystem::directory_iterator varEnd;
                    for (; varPos != varEnd; ++varPos) {
                        if (varPos->is_directory()) {
                            continue;
                        }
                        auto varPath = varPos->path();
                        const static std::wregex varCheckRegex{ LR"([0-9]{4}[.]jpg)" };
                        auto const varFileName = varPath.filename().wstring();
                        varFileNames.push_back(toInt(varFileName));
                        if (!std::regex_match(varFileName, varCheckRegex)) {
                            /*更改不规范的图片名称...*/
                            const static std::wregex varCheckRegex1{ LR"([0-9]{4}[.][jJ][pP][eE]?[gG])" };
                            if (std::regex_match(varFileName, varCheckRegex1)) {
                                auto varNewFilePath = varPath;
                                auto varNewFileName = varFileName;
                                varNewFileName.resize(1+varFileName.find_last_of(L'.'));
                                try {
                                    std::filesystem::rename(varPath,
                                        varNewFilePath.replace_filename(varNewFileName += LR"(jpg)"sv));
                                } catch (const std::exception & e) {
                                    cout_() << e.what() << std::endl;
                                }
                            } else {
                                wcout_() << varFileName << LR"( 错误的文件名 ??? )"sv << std::endl;
                            }
                        }
                    }
                    std::sort(varFileNames.begin(), varFileNames.end());
                }
                if (varFileNames.empty()) {
                    wcout_() << dirName << LR"( 空目录 ??? )"sv << std::endl;
                    return;
                }
                if (varFileNames[0] <= 0) {
                    wcout_() << dirName << LR"( 文件最小编号小于 1 ??? )"sv << std::endl;
                }
                for (const auto & varI : varFileNames) {
                    varReplaceName = varI;
                    if (varReplaceName != 0) {
                        break;
                    }
                }
            }
            if (varReplaceName < dirNameInt[3]) {
                wcout_()
                    << LR"(目录：)"sv << dirNameString
                    << LR"(文件最小编号：)"sv << varReplaceName
                    << LR"(小于目录编号：)"sv << dirNameInt[3]
                    << std::endl;
                throw std::runtime_error("严重错误，停止！"s);
            }
            minFileIndex = varReplaceName;
            replaceDirName.resize(replaceDirName.size() - 3);
            replaceDirName += toWstring(varReplaceName);
        }

    };

private:
    using Dirs = std::list< std::shared_ptr< Dir > >;
    Dirs dirs;

    /*重命名所有目录*/
    inline void makeDirs() {
        dirs.clear();
        std::filesystem::directory_iterator varPos{ thisRootPath };
        const std::filesystem::directory_iterator varEnd;
        for (; varPos != varEnd; ++varPos) {
            if (varPos->is_directory()) {
                auto varDirPath = std::make_shared<Dir>(varPos->path());
                if (varDirPath->isFormatDir()) {
                    varDirPath->fullConstruct();
                    dirs.push_back(std::move(varDirPath));
                }
            }
        }
    }

public:

    inline bool apply() try {

        /* 创建所有执行目录对象 */
        makeDirs();

        /* 检查目录数 */
        if (dirs.size() < 2) {
            wcout_() << LR"(目录小于2)"sv;
            throw std::runtime_error("严重错误，停止！"s);
        }

        /* 将所有目录对象按照由大到小排序 */
        dirs.sort([](const auto & l, const auto & r) {
            return l->dirNameInt[3] > r->dirNameInt[3];
        });

        {/* 检查卷宗号是否一致，目录号是否重复，文件编号有无逆序 */
            auto varNextPos = dirs.begin();
            const auto varEnd = dirs.end();
            for (auto varPos = varNextPos++; varNextPos != varEnd; varPos = varNextPos++) {
                if (((*varPos)->minFileIndex <= (*varNextPos)->minFileIndex) ||
                    ((*varPos)->dirNameInt[3] == (*varNextPos)->dirNameInt[3])) {
                    wcout_()
                        << LR"(目录：)"sv << (*varPos)->dirNameString
                        << LR"(最小文件号：)"sv
                        << (*varPos)->minFileIndex
                        << std::endl
                        << LR"(目录：)"sv << (*varNextPos)->dirNameString
                        << LR"(最小文件号：)"sv
                        << (*varNextPos)->minFileIndex
                        << std::endl
                        << LR"(最小文件编号逆序或目录重复)"sv
                        << std::endl;
                    throw std::runtime_error("严重错误，停止！"s);
                }
                if (((*varPos)->dirNameInt[0] != (*varNextPos)->dirNameInt[0]) ||
                    ((*varPos)->dirNameInt[1] != (*varNextPos)->dirNameInt[1]) ||
                    ((*varPos)->dirNameInt[2] != (*varNextPos)->dirNameInt[2])) {
                    wcout_()
                        << LR"(目录：)"sv << (*varPos)->dirNameString << std::endl
                        << LR"(目录：)"sv << (*varNextPos)->dirNameString << std::endl
                        << LR"(卷宗编号不一致)"sv
                        << std::endl;
                    throw std::runtime_error("严重错误，停止！"s);
                }
            }
        }

        /* 执行重命名 */
        for (auto & varI : dirs) {
            try {
                varI->rename();
            } catch (const std::exception & e) {
                cout_() << e.what() << std::endl;
            }
        }

        return true;
    } catch (const std::exception & e) {
        throw e;
    } catch (...) {
        return false;
    }

};

#include <locale>
bool globalNeedPause = false;

int main(int argc, char ** argv) try {

    {
        std::locale varLocal{ "chs" };
        std::wcout.imbue(varLocal);
    }

    std::unique_ptr<Duty> varDuty;

    if (argc < 2) {
        std::filesystem::path varPath{ argv[0] };
        globalNeedPause = true;
        varDuty = std::make_unique<Duty>(varPath.parent_path());
    } else {
        varDuty = std::make_unique<Duty>(argv[1]);
    }

    auto varAns = !varDuty->apply();
    if (globalNeedPause) {
        system("pause");
    }
    return varAns;

} catch (const std::exception & e) {
    cout_() << e.what() << std::endl;
    if (globalNeedPause) {
        system("pause");
    }
    return -1;
}

