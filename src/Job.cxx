#include "Job.hxx"

namespace SJM
{
    Job::Job(const std::string &name) :
    hasStarted(false),
    hasFinished(false),
    currentPercentage(0),
    lastPercentage(0),
    avgPercentage(0),
    errorCounter(0),
    fileContents(""),
    percent(""),
    fileName(name),
    errorMessage(""),
    state(AnalysisState::NotStarted),
    currentTime(std::chrono::system_clock::now()),
    lastTime(std::chrono::system_clock::now()),
    avgElapsedTime(0),
    ETA(0),
    remainigTime(0)
    {
        std::string prefix = "slurm-", delim = "_",postfix = ".out";
        std::size_t pos1 = name.find(prefix);
        if (pos1 != std::string::npos)
        {
            std::size_t pos2 = name.rfind(delim);
            std::size_t pos3 = name.find(postfix);
            jobId = name.substr(pos1 + prefix.size(),pos2 - pos1 - prefix.size());
            taskId = name.substr(pos2 + delim.size(),pos3 - pos2 - delim.size());
        }
        else
        {
            prefix = "apr12ana_all_",postfix = ".log";
            pos1 = name.find(prefix);
            std::size_t pos2 = name.find(postfix);
            jobId = "N/A";
            taskId = name.substr(pos1 + prefix.size(),pos2 - pos1 - prefix.size());
            state = Job::AnalysisState::Finished;

            FileHandler file(name);
            realRunTime = ReadTime(file.ReadFile());

            if (errorCounter >= maxErrorCount) // I'm starting to produce spaghetti code
                state = Job::AnalysisState::Error;
        }
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
                    ++errorCounter;
                    errorMessage += std::to_string(errorCounter) + " unable to parse current percentage, ";
                }

                currentTime = std::chrono::system_clock::now();
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

        if (fileContents.find(beginingExpression) != std::string::npos)
            hasStarted = true;

        if (fileContents.find(endingExpression) != std::string::npos && hasStarted)
            hasFinished = true;

        if (errorCounter >= maxErrorCount)
            return Job::AnalysisState::Error;

        if (hasStarted && hasFinished)
        {
            return Job::AnalysisState::Finished;
        }
        else if(hasStarted && !hasFinished)
        {
                std::size_t percentPos = fileContents.rfind(percentExpression);
                if (percentPos != std::string::npos)
                    percent = fileContents.substr(percentPos - substrOffset,substrLength);

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

    std::chrono::seconds Job::ReadTime(const std::string &&str)
    {
        using namespace std::chrono;

        seconds time(0);
        const std::size_t charSize(2);
        std::size_t pos1 = str.find(realTimePrefix);
        if (pos1 != std::string::npos)
        {
            std::string sstr = str.substr(pos1 + realTimePrefix.size(),20);

            std::size_t posM = sstr.find("m");
            std::size_t posS = sstr.find("s");
            if (posM != std::string::npos)
            {
                float minutes;
                try
                {
                    minutes = std::stof(sstr.substr(0,posM));
                }
                catch(const std::exception& e)
                {
                    minutes = 0.;
                    ++errorCounter;
                    errorMessage += std::to_string(errorCounter) + " unable to parse minutes, ";
                }
                time += seconds(static_cast<int>(minutes)*60);
            }
            if (posS != std::string::npos)
            {
                float sec;
                try
                {
                    sec = std::stof(sstr.substr(posM + charSize,posS - posM - charSize));
                }
                catch(const std::exception& e)
                {
                    sec = 0.;
                    ++errorCounter;
                    errorMessage += errorCounter + "unable to parse seconds, ";
                }
                
                time += seconds(static_cast<int>(sec));
            }
        }
        return time;
    }

    void Job::CalculateTime()
    {
        elapsedQueue.push_back(std::chrono::duration_cast<std::chrono::seconds>(currentTime - lastTime));
        if (elapsedQueue.size() > maxQueueSize)
            elapsedQueue.pop_front();

        progressQueue.push_back(abs(currentPercentage - lastPercentage));
        if (progressQueue.size() > maxQueueSize)
            progressQueue.pop_front();

        avgElapsedTime = CalcAverage(elapsedQueue);
        avgPercentage = CalcAverage(progressQueue);

        (avgPercentage > 0) ? ETA = avgElapsedTime * maxPercentage / avgPercentage : ETA = std::chrono::seconds(0);
        remainigTime = abs(ETA - avgElapsedTime);
    }

} // namespace SJM
