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
                FileHandler(/* args */) = default;
                explicit FileHandler(const std::string &);
                ~FileHandler();
                FileHandler(const FileHandler&) = delete;
                FileHandler& operator=(const FileHandler&) = delete;
                FileHandler(FileHandler&&) = default;
                FileHandler& operator=(FileHandler&&) = default;

                std::string ReadFile();
        };
    } // namespace SJM
    
#endif