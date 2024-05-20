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

    std::string_view FileHandler::ReadFile()
    {
        std::stringstream sstr;
        if (isOk)
            sstr << inputStream.rdbuf();
        return static_cast<std::string_view>(sstr.str());
    }
} // namespace SJM
