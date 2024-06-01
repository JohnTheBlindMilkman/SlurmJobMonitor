#ifndef Config_hxx
    #define Config_hxx

    #include <string_view>

    namespace SJM::Config
    {
        inline constexpr std::string_view projectName{ "SlurmJobMonitor" };
        inline constexpr std::string_view projectVersion{ "1.2.1" };
        inline constexpr int projectVersionMajor { 1 };
        inline constexpr int projectVersionMinor { 2 };
        inline constexpr int projectVersionPatch { 1 };
    }

#endif
