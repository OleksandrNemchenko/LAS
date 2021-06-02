
#include <cassert>
#include <fstream>
#include <regex>
#include <string>
#include <system_error>

#include <las_parser/lasCore.hpp>

using namespace std::string_literals;

/* static */ const std::string CLASCore::m_stringDelimiter = "\r\n";
/* static */ const size_t CLASCore::m_stringDelimiterSize = m_stringDelimiter.size();

/* static */ std::wstring CLASCore::m_msgFileIsEmpty = L"File is empty"s;
/* static */ std::wstring CLASCore::m_msgFileDoesNotExist = L"File does not exist"s;
/* static */ std::wstring CLASCore::m_msgUnableToOpenFile = L"Unable to open file"s;
/* static */ std::wstring CLASCore::m_msgErrorOpeningFile = L"Error has happened while opening file"s;
/* static */ std::wstring CLASCore::m_msgErrorParsingFile = L"Error has happened while parsing file"s;
/* static */ std::wstring CLASCore::m_msgCurrentDir = L"current directory"s;
/* static */ std::wstring CLASCore::m_msgUnsupportedFileFormat = L"Unsupported file format"s;

CLASCore::TLines CLASCore::GenerateLines(const std::string& fileContents) const
{
    if (fileContents.empty())
        throw m_msgFileIsEmpty;

    size_t linesAmount = 0;
    for (size_t curPos = 0; curPos != std::string::npos; ++linesAmount)
        curPos = fileContents.find_first_of(m_stringDelimiter, curPos + m_stringDelimiterSize);

    TLines rawLines;
    rawLines.reserve(linesAmount);
    size_t curPos = 0;
    size_t nextPos = 0;
    size_t fileSize = fileContents.size();
    do
    {
        nextPos = fileContents.find_first_of(m_stringDelimiter, curPos);
    
        if (nextPos == std::string::npos && curPos >= fileSize)
            break;

        if (nextPos == std::string::npos)
            nextPos = fileSize;

        rawLines.emplace_back(fileContents.substr(curPos, nextPos - curPos));

        if (nextPos == fileSize)
            break;

        curPos = nextPos + m_stringDelimiterSize;
    }
    while(true);

    return rawLines;
}

void CLASCore::DetectSections(const TLines& lines)
{
    m_section.clear();
    m_other.clear();
    m_log.clear();

    char currentSection = 0;
    SSection* section = nullptr;
    int positionId = 0;
    
    for (const auto& line : lines)
    {
        enum
        {
            COMMENTS = '#',
            SECTION_START = '~',
            FIRST_SYMBOL_POS = 0,
            SECTION_NAME_POS = 1
        };
        
        if (line.empty())
            continue;

        const char firstSymbol = line.at(FIRST_SYMBOL_POS);

        if (firstSymbol == COMMENTS)
            continue;

        if (firstSymbol == SECTION_START)
        {
            currentSection = line.at(SECTION_NAME_POS);
            section = &m_section[currentSection];
            section->m_fullName = line.substr(SECTION_NAME_POS);
            positionId = -1;

            continue;
        }

        switch (currentSection)
        {
        case 'O':
            m_other += line + m_stringDelimiter;
            break;

        case 'A':
            ProcessASCII(line);
            break;

        default:
            ProcessValue(section->m_values, positionId++, line);
            break;
        }
    }
}

void CLASCore::ProcessASCII(std::string line)
{
    SLogData logData;
    size_t pos;

    pos = line.find_first_not_of(' ');
    if (pos == std::string::npos)
        throw m_msgUnsupportedFileFormat;
    line = line.substr(pos);

    pos = line.find_first_of(' ');
     if (pos == std::string::npos)
        throw m_msgUnsupportedFileFormat;
    logData.m_index = line.substr(0, pos);
    line = line.substr(pos + 1);

    do
    {
        pos = line.find_first_not_of(' ');
        if (pos == std::string::npos)
            break;
        line = line.substr(pos);

        pos = line.find_first_of(' ');
        if (pos == std::string::npos)
            pos = line.size();
        logData.m_values.emplace_back(line.substr(0, pos));
        if (pos == line.size())
            break;

        line = line.substr(pos + 1);
    }
    while(true);

    m_log.emplace_back(std::move(logData));
}

void CLASCore::ProcessValue(TSection& section, int positionId, std::string line)
{
    SValue value;
    size_t pos;
    std::string mnemonic;

    value.m_positionId = positionId;

    auto cutSpaces = [](std::string value)
    {
        if (value.empty())
            return value;

        size_t firstNot = value.find_first_not_of(' ');
        size_t lastNot = value.find_last_not_of(' ');

        if (firstNot == std::string::npos || lastNot == std::string::npos )
            return std::string();
            
        return value = value.substr(firstNot, lastNot - firstNot + 1);
    };

    pos = line.find_first_of('.');
    if (pos == std::string::npos)
        throw m_msgErrorParsingFile;
    mnemonic = cutSpaces(line.substr(0, pos));
    line = line.substr(pos + 1);

    pos = line.find_first_of(' ');
    if (pos == std::string::npos)
        throw m_msgErrorParsingFile;
    value.m_units = cutSpaces(line.substr(0, pos));
    line = line.substr(pos + 1);

    pos = line.find_first_of(':');
    if (pos == std::string::npos)
        throw m_msgErrorParsingFile;
    value.m_data = cutSpaces(line.substr(0, pos));
    line = line.substr(pos + 1);
    
    value.m_description = cutSpaces(line);

    section[mnemonic] = std::move(value);
}

bool CLASCore::BaseParsing(const std::string& fileContents, std::wstring* errorDescription)
{
    try
    {
        TLines lines = GenerateLines(fileContents);
        DetectSections(lines);
        Validate();

        return true;
    }
    catch (const std::wstring& errorMessage)
    {
        if (errorDescription)
            *errorDescription = errorMessage;
        return false;
    }
}

bool CLASCore::ParseWithError(const std::filesystem::path& filePath, std::wstring* errorDescription)
{
    using namespace std::string_literals;

    try
    {
        std::string fileStr;

        {
            std::error_code errorCode;
            if (!std::filesystem::exists(filePath, errorCode))
                throw m_msgFileDoesNotExist;
    
            std::ifstream file;
            file.open(filePath, std::ifstream::binary);
            if (!file.is_open())
                throw m_msgUnableToOpenFile;
    
            std::stringstream fileBuf;
            fileBuf << file.rdbuf();
            if (!fileBuf.good())
                throw m_msgErrorOpeningFile;

            fileStr = fileBuf.str();
        }

        std::wstring parseErrorDescr;
        bool res = ParseWithError(fileStr, &parseErrorDescr);
        if (!res)
            throw m_msgErrorParsingFile + L": "s + parseErrorDescr;

        return true;
    }
    catch (const std::wstring& errorMessage)
    {
        if (errorDescription)
        {
            if (filePath.is_absolute())
                *errorDescription = filePath.wstring();
            else
                *errorDescription = filePath.wstring() + L", "s + m_msgCurrentDir + L" "s + std::filesystem::current_path().wstring();
            *errorDescription += L": "s + errorMessage;
        }
        return false;
    }
}

bool CLASCore::HasSection(char section) const noexcept
{
    return m_section.find(section) != m_section.cend();
}

bool CLASCore::HasField(char section, const std::string& name) const noexcept
{
    if (!HasSection(section))
        return false;

    const TSection& values = m_section.at(section).m_values;

    return values.find(name) != values.cend();
}
