/**
 * @file FileHandler.hxx
 * @author Jędrzej Kołaś (jedrzej.kolas.dokt@pw.edu.pl)
 * @brief Helper class for managing the std::ifstream object, making use of RAII
 * @version 1.2.2
 * @date 2024-06-02
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef FileHandler_hxx
    #define FileHandler_hxx

    #include <string_view>
    #include <sstream>
    #include <fstream>

    namespace SJM
    {
        class FileHandler
        {
            private:
                bool isOk;
                std::ifstream inputStream;

            public:
                /**
                 * @brief Construct a new File Handler object
                 * 
                 */
                FileHandler() = default;
                /**
                 * @brief Construct a new File Handler object
                 * 
                 */
                explicit FileHandler(const std::string &);
                /**
                 * @brief Destroy the File Handler object
                 * 
                 */
                ~FileHandler();
                /**
                 * @brief Construct a new File Handler object
                 * 
                 */
                FileHandler(const FileHandler&) = delete;
                /**
                 * @brief Standard copy assignment operator
                 * 
                 * @return FileHandler& 
                 */
                FileHandler& operator=(const FileHandler&) = delete;
                /**
                 * @brief Construct a new File Handler object
                 * 
                 */
                FileHandler(FileHandler&&) = default;
                /**
                 * @brief Standard move assignment operator
                 * 
                 * @return FileHandler& 
                 */
                FileHandler& operator=(FileHandler&&) = default;
                /**
                 * @brief Try to open and retrieve contents of the file
                 * 
                 * @return std::string Contents of the file
                 */
                std::string ReadFile();
        };
    } // namespace SJM
    
#endif