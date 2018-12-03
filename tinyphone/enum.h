#pragma once

#include <algorithm>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

#define STRING_REMOVE_CHAR(str, ch) str.erase(std::remove(str.begin(), str.end(), ch), str.end())

std::vector<std::string> splitString(std::string str, char sep = ',') {
	std::vector<std::string> vecString;
	std::string item;

	std::stringstream stringStream(str);

	while (std::getline(stringStream, item, sep))
	{
		vecString.push_back(item);
	}

	return vecString;
}

#define DECLARE_ENUM_WITH_TYPE(E, T, ...)                                                                     \
    enum class E : T                                                                                          \
    {                                                                                                         \
        __VA_ARGS__                                                                                           \
    };                                                                                                        \
    std::map<T, std::string> E##MapName(generateEnumMap<T>(#__VA_ARGS__));                                    \
    std::ostream &operator<<(std::ostream &os, E enumTmp)                                                     \
    {                                                                                                         \
        os << E##MapName[static_cast<T>(enumTmp)];                                                            \
        return os;                                                                                            \
    }                                                                                                         \
    size_t operator*(E enumTmp) { (void) enumTmp; return E##MapName.size(); }                                 \
    std::string operator~(E enumTmp) { return E##MapName[static_cast<T>(enumTmp)]; }                          \
    std::string operator+(std::string &&str, E enumTmp) { return str + E##MapName[static_cast<T>(enumTmp)]; } \
    std::string operator+(E enumTmp, std::string &&str) { return E##MapName[static_cast<T>(enumTmp)] + str; } \
    std::string &operator+=(std::string &str, E enumTmp)                                                      \
    {                                                                                                         \
        str += E##MapName[static_cast<T>(enumTmp)];                                                           \
        return str;                                                                                           \
    }                                                                                                         \
    E operator++(E &enumTmp)                                                                                  \
    {                                                                                                         \
        auto iter = E##MapName.find(static_cast<T>(enumTmp));                                                 \
        if (iter == E##MapName.end() || std::next(iter) == E##MapName.end())                                  \
            iter = E##MapName.begin();                                                                        \
        else                                                                                                  \
        {                                                                                                     \
            ++iter;                                                                                           \
        }                                                                                                     \
        enumTmp = static_cast<E>(iter->first);                                                                \
        return enumTmp;                                                                                       \
    }                                                                                                         \
    bool valid##E(T value) { return (E##MapName.find(value) != E##MapName.end()); }

#define DECLARE_ENUM(E, ...) DECLARE_ENUM_WITH_TYPE(E, int32_t, __VA_ARGS__)
template <typename T>
std::map<T, std::string> generateEnumMap(std::string strMap)
{
	STRING_REMOVE_CHAR(strMap, ' ');
	STRING_REMOVE_CHAR(strMap, '(');

	std::vector<std::string> enumTokens(splitString(strMap));
	std::map<T, std::string> retMap;
	T inxMap;

	inxMap = 0;
	for (auto iter = enumTokens.begin(); iter != enumTokens.end(); ++iter)
	{
		// Token: [EnumName | EnumName=EnumValue]
		std::string enumName;
		T enumValue;
		if (iter->find('=') == std::string::npos)
		{
			enumName = *iter;
		}
		else
		{
			std::vector<std::string> enumNameValue(splitString(*iter, '='));
			enumName = enumNameValue[0];
			//inxMap = static_cast<T>(enumNameValue[1]);
			if (std::is_unsigned<T>::value)
			{
				inxMap = static_cast<T>(std::stoull(enumNameValue[1], 0, 0));
			}
			else
			{
				inxMap = static_cast<T>(std::stoll(enumNameValue[1], 0, 0));
			}
		}
		retMap[inxMap++] = enumName;
	}

	return retMap;
}
