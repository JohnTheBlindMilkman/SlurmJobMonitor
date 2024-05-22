#include "Job.hxx"

namespace SJM
{
    Job::Job(const std::string &name) :
    hasStarted(false),
    hasFinished(false),
    currentPercentage(0),
    lastPercentage(0),
    fileContents(""),
    percent(""),
    fileName(name),
    state(AnalysisState::NotStarted),
    currentTime(std::chrono::steady_clock::now()),
    lastTime(std::chrono::steady_clock::now()),
    elapsedTime(0),
    ETA(0),
    remainigTime(0)
    {
        std::string prefix = "slurm-", delim = "_",postfix = ".out";
        std::size_t pos1 = name.find(prefix);
        std::size_t pos2 = name.rfind(delim);
        std::size_t pos3 = name.find(postfix);
        jobId = name.substr(pos1 + prefix.size(),pos2 - pos1 - prefix.size());
        taskId = name.substr(pos2 + delim.size(),pos3 - pos2 - delim.size());
    }

    bool Job::Evaluate()
    {
        state = EvalState();

        switch (state)
        {
            case Job::AnalysisState::NotStarted:
                lastPercentage = 0;
                currentPercentage = 0;
                break;
            case Job::AnalysisState::Started:
                lastPercentage = currentPercentage;
                lastTime = currentTime;

                try
                {
                    currentPercentage = std::stoi(percent);
                }
                catch (const std::exception &e)
                {
                    currentPercentage = lastPercentage;
                }

                currentTime = std::chrono::steady_clock::now();
                CalculateTime();
                break;
            case Job::AnalysisState::Finished:
                currentPercentage = 100;
                break;
            case Job::AnalysisState::Error:
                return false;
            default:
                break;
        }

        return true;
    }

    Job::AnalysisState Job::EvalState()
    {
        FileHandler file(fileName);
        fileContents = file.ReadFile();

        if (fileContents.find(GlobalConstants::beginingExpression) != std::string::npos)
            hasStarted = true;

        if (fileContents.find(GlobalConstants::endingExpression) != std::string::npos && hasStarted)
            hasFinished = true;

        if (hasStarted && hasFinished)
        {
            return Job::AnalysisState::Finished;
        }
        else if(hasStarted && !hasFinished)
        {
                std::size_t percentPos = fileContents.rfind(GlobalConstants::percentExpression);
                if (percentPos != std::string::npos)
                    percent = fileContents.substr(percentPos - GlobalConstants::substrOffset,GlobalConstants::substrLength);

                return Job::AnalysisState::Started;
        }
        else if (!hasStarted && !hasFinished)
        {
            return Job::AnalysisState::NotStarted;
        }
        else
        {
            return Job::AnalysisState::Error;
        }
    }

    std::stringstream Job::MakeTime(std::chrono::seconds sec)
    {
        std::stringstream sstr;
        using namespace std::chrono;
        using days = duration<int, std::ratio<86400>>;
        char fill = sstr.fill();
        sstr.fill('0');
        auto d = duration_cast<days>(sec);
        sec -= d;
        auto h = duration_cast<hours>(sec);
        sec -= h;
        auto m = duration_cast<minutes>(sec);
        sec -= m;
        auto s = duration_cast<seconds>(sec);
        if (d.count() > 0)
            sstr << std::setw(2) << d.count() << "d:";
        if (h.count() > 0)
            sstr << std::setw(2) << h.count() << "h:";
        sstr << std::setw(2) << m.count() << "m:" << std::setw(2) << s.count() << 's';
        sstr.fill(fill);
        return sstr;
    }

    void Job::CalculateTime()
    {
        elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastTime);
        int percentProgress = currentPercentage - lastPercentage;
        (percentProgress > 0) ? ETA = elapsedTime * GlobalConstants::maxPercentage / percentProgress : ETA = std::chrono::seconds(0);
        (ETA > elapsedTime) ? remainigTime = ETA - elapsedTime : remainigTime = elapsedTime - ETA;
    }

} // namespace SJM
