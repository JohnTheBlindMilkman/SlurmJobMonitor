#include "FileHandler.hxx"

namespace SJM
{
    FileHandler::FileHandler(const std::string &name)
    {
        inputStream.open(name);
        inputStream.is_open() ? isOk = true : isOk = false;
    }

    FileHandler::~FileHandler()
    {
        if (isOk)
            inputStream.close();
    }

    std::string FileHandler::ReadFile()
    {
        std::stringstream sstr;
        sstr << inputStream.rdbuf();
        return sstr.str();
    }
} // namespace SJM
