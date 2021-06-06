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
    static std::wstring m_msgIncorrentCurvePosition;

    std::string Generate();

private:
    void Validate() override;
    void ValidateBeforeGenerating();
};

#endif // LAS_PARSER_VER_2
