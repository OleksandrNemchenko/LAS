
#include <array>
#include <las_parser/las2.hpp>

using namespace std::string_literals;

/* static */ std::wstring CLAS2::m_msgRequiredSectionsAreAbsent = L"Required sections are absent"s;

bool CLAS2::ParseWithError(const std::string& fileContents, std::wstring* errorDescription)
{
    if (!BaseParsing(fileContents, errorDescription))
        return false;

    return true;
}

void CLAS2::Validate()
{
    static const std::array requiredSections = {'V', 'W', 'C', 'A'};

    if (Sections().size() < requiredSections.size())
        throw m_msgRequiredSectionsAreAbsent;

    auto charToWCharT = [](char symbol) -> wchar_t
    {
        size_t resSize;
        const std::string srcStr = {symbol};
        std::array<wchar_t, 2> resStr;
        
        mbstowcs_s(&resSize, resStr.data(), resStr.size(), srcStr.c_str(), srcStr.size());

        return resStr.at(0);
    };

    std::wstring absentSections;
    auto checkSection = [&, this](char section)
    {
        if (!HasSection(section))
        {
            if (!absentSections.empty())
                absentSections += L", ";
            absentSections += L"~" + charToWCharT(section);
        }
    };

    for (char section : requiredSections)
        checkSection(section);

    if (!absentSections.empty())
        throw m_msgRequiredSectionsAreAbsent + L": "s + absentSections;
}
