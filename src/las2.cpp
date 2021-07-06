
#include <array>
#include <set>

#include <las_parser/las2.hpp>

using namespace std::string_literals;

/* static */ std::wstring CLAS2::m_msgRequiredSectionsAreAbsent = L"Required sections are absent"s;
/* static */ std::wstring CLAS2::m_msgIncorrentCurvePosition = L"Incorrect curve position"s;

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

std::string CLAS2::Generate()
{
    ValidateBeforeGenerating();

    std::string rawFile;

    static const size_t MIN_SIZE = 5;
    size_t maxMnemonic = MIN_SIZE;
    size_t maxUnits = MIN_SIZE;
    size_t maxData = MIN_SIZE;

    for (const auto& sectionIt : Sections())
    {
        for (const auto& valueIt : sectionIt.second.m_values)
        {
            const SValue& value = valueIt.second;

            auto calculateMax = [](size_t& prevMaxValue, const std::string& currentValue)
            {
                size_t curSize = currentValue.size();
                prevMaxValue = std::max(prevMaxValue, curSize);
            };

            calculateMax(maxMnemonic, valueIt.first);
            calculateMax(maxUnits, value.m_units);
            calculateMax(maxData, value.m_data);
        }
    }

    ++maxUnits;
    ++maxData;

    auto outputSection = [&](const SSection& section)
    {
        rawFile += '~' + section.m_fullName + m_stringDelimiter;

        bool noPosId = true;

        for (const auto& field : section.m_values)
        {
            if (field.second.m_positionId != 0)
            {
                noPosId = false;
                break;
            }
        }

        std::vector<std::pair<const std::string*, const SValue*>> values;

        int valuesAmount = section.m_values.size();

        values.reserve(valuesAmount);

        if (noPosId)
        {
            for (const auto& field : section.m_values)
                values.emplace_back(std::make_pair(&field.first, &field.second));
        }
        else
        {
            for (int curPosId = 0; curPosId < valuesAmount; ++curPosId)
            {
                for (const auto& field : section.m_values)
                    if (field.second.m_positionId == curPosId)
                    {
                        values.emplace_back(std::make_pair(&field.first, &field.second));
                        break;
                    }
            }
        }

        for (const auto& field : values)
        {
            const SValue& value = *field.second;

            std::string mnemonic = *field.first;
            std::string units = value.m_units;
            std::string data = value.m_data;
            std::string description = value.m_description;

            auto appendSpace = [](size_t maxValueSize, std::string& value)
            {
                value.append(maxValueSize - value.size(), ' ');
            };

            appendSpace(maxMnemonic, mnemonic);
            appendSpace(maxUnits, units);
            appendSpace(maxData, data);

            rawFile += mnemonic + "."s + units + " "s + data + ":"s + value.m_description + m_stringDelimiter;
        }
    };

    outputSection(Section('V'));

    outputSection(Section('W'));

    outputSection(Section('C'));

    if (HasSection('P'))
    {
        outputSection(Section('P'));
    }

    if (HasSection('O'))
    {
        rawFile += Others();
    }

    static const std::set sectionToSkip = {'V', 'W', 'C', 'P', 'O', 'A'};
    for (const auto& section : Sections())
    {
        if (sectionToSkip.contains(section.first))
            continue;

        outputSection(Section(section.first));
    }

    rawFile += '~' + Section('A').m_fullName + m_stringDelimiter;
    bool firstLogLine = true;
    for (const SLogData& line : Logs())
    {
        if (!firstLogLine)
            rawFile += m_stringDelimiter;
        else
            firstLogLine = false;

        rawFile += " "s + line.m_index;
        for (const std::string& value: line.m_values)
            rawFile += " "s + value;
    }

    return rawFile;
}

void CLAS2::ValidateBeforeGenerating()
{
    static const std::array requiredFields = 
    {
        std::pair{'V', "VERS"s},
        std::pair{'V', "WRAP"s},

        std::pair{'W', "STRT"s},
        std::pair{'W', "STOP"s},
        std::pair{'W', "STEP"s},
        std::pair{'W', "NULL"s},
        std::pair{'W', "COMP"s},
        std::pair{'W', "WELL"s},
        std::pair{'W', "FLD"s},
        std::pair{'W', "LOC"s},
        std::pair{'W', "PROV"s},
        std::pair{'W', "CNTY"s},
        std::pair{'W', "STAT"s},
        std::pair{'W', "CTRY"s},
        std::pair{'W', "SRVC"s},
        std::pair{'W', "DATE"s},
    };
    for (const auto& field : requiredFields)
        if (!HasField(field.first, field.second))
            throw m_msgRequiredSectionsAreAbsent;

    const TSection& units = Section('C').m_values;
    size_t unitsAmount = units.size();
    std::vector<std::size_t> positions(unitsAmount, 0);

    for (const auto& unit : units)
    {
        if (unit.second.m_positionId >= static_cast<int>(unitsAmount))
            throw m_msgIncorrentCurvePosition;

        ++positions[unit.second.m_positionId];
    }

    for (const auto& position : positions)
    {
        if (position != 1)
            throw m_msgIncorrentCurvePosition;
    }
}