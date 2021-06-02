#ifndef LAS_PARSER_VER_2
#define LAS_PARSER_VER_2

#include <string>

#include <las_parser\lasCore.hpp>

class CLAS2 : public CLASCore
{
public:

    static bool CheckVersionCompatibility(const std::string& lasFile, std::wstring* errorDescription = nullptr) noexcept;

    using CLASCore::ParseWithError;
    bool ParseWithError(const std::string& fileContents, std::wstring* errorDescription) override;

    static std::wstring m_msgRequiredSectionsAreAbsent;

    struct SGenerateData
    {
        using TValues = std::vector<SValue>;
        using TLogLine = std::vector<std::string>;
        using TLogData = std::vector<TLogLine>;

        TValues m_version;
        TValues m_well;
        TValues m_curve;
        TValues m_parameter;
        std::string m_other;
        TLogData m_log;
    };
    static CLAS2 Generate(const struct SGenerateData& data);    // throws std::wstring

protected:
    void Validate() override;
};

#endif // LAS_PARSER_VER_2
