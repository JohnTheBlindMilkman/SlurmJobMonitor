/**
 * @file Config.hxx
 * @author Jędrzej Kołaś (jedrzej.kolas.dokt@pw.edu.pl)
 * @brief Global config class containing project and version information. Created using CMake configure file macro
 * @version 1.2.2
 * @date 2024-06-02
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef Config_hxx
    #define Config_hxx

    #include <string_view>

    namespace SJM::Config
    {
        inline constexpr std::string_view projectName{ "SlurmJobMonitor" };
        inline constexpr std::string_view projectVersion{ "1.2.2" };
        inline constexpr int projectVersionMajor { 1 };
        inline constexpr int projectVersionMinor { 2 };
        inline constexpr int projectVersionPatch { 2 };
    }

#endif
