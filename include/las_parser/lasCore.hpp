#ifndef LAS_PARSER_CORE
#define LAS_PARSER_CORE

#include <cassert>
#include <filesystem>
#include <string>
#include <unordered_map>

class CLASCore
{
public:

    struct SValue
    {
        std::string m_units;
        std::string m_data;
        std::string m_description;
        int m_positionId;
    };

    using TLogValues = std::vector<std::string>;
    struct SLogData
    {
        std::string m_index;
        TLogValues m_values;
    };

    using TLog = std::vector<SLogData>;
    using TSection = std::unordered_map<std::string, SValue>;

    struct SSection
    {
        std::string m_fullName;
        TSection m_values;
    };

    using TSections = std::unordered_map<char, SSection>;
    using TLog = std::vector<SLogData>;

    virtual ~CLASCore() = default;

    virtual bool ParseWithError(const std::string& fileContents, std::wstring* errorDescription) = 0;
    bool ParseWithError(const std::filesystem::path& filePath, std::wstring* errorDescription);
    bool Parse(const std::string& fileContents)            { return ParseWithError(fileContents, nullptr); }
    bool Parse(const std::filesystem::path& filePath)      { return ParseWithError(filePath, nullptr); }

    const SSection& Section(char section) const noexcept   { assert(HasSection(section)); return m_section.at(section);}
    SSection& Section(char section) noexcept               { return m_section[section]; }
    bool HasSection(char section) const noexcept;

    const SValue& Field(char section, const std::string& name) const noexcept   { assert(HasField(section, name)); return m_section.at(section).m_values.at(name);}
    SValue& Field(char section, const std::string& name) noexcept               { return m_section[section].m_values[name]; }
    bool HasField(char section, const std::string& name) const noexcept;

    TSections& Sections() noexcept                      { return m_section; }
    std::string Others() noexcept                       { return m_other; }
    TLog& Logs() noexcept                               { return m_log; }

    static std::wstring m_msgFileIsEmpty;
    static std::wstring m_msgFileDoesNotExist;
    static std::wstring m_msgUnableToOpenFile;
    static std::wstring m_msgErrorOpeningFile;
    static std::wstring m_msgErrorParsingFile;
    static std::wstring m_msgCurrentDir;
    static std::wstring m_msgUnsupportedFileFormat;

protected:
    static const std::string m_stringDelimiter;
    static const size_t m_stringDelimiterSize;

    virtual void Validate()                             { }

    bool BaseParsing(const std::string& fileContents, std::wstring* errorDescription);

private:
    TSections m_section;
    std::string m_other;
    TLog m_log;

    using TLines = std::vector<std::string>;
    TLines GenerateLines(const std::string& fileContents) const;
    void DetectSections(const TLines& lines);
    void ProcessASCII(std::string line);
    void ProcessValue(TSection& section, int positionId, std::string line);

    template<typename TSect, typename TSects>
    static TSect* SectionImpl(TSects sections, char name) noexcept;
};

#endif // LAS_PARSER_CORE
