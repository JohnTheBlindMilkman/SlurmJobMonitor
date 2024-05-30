#include "JobManager.hxx"

namespace SJM
{
    JobManager::JobManager(std::string dirPath,unsigned njobs) : directoryPath(dirPath),runningCounter(0),finishedCounter(0),totalJobs(njobs)
    {
        int counter = 0;
        if (! std::filesystem::is_directory(directoryPath))
        {
            std::cerr << "Provided path " << directoryPath << " is not a directory\n";
            std::exit(1);
        }
        for (const auto &entry : std::filesystem::directory_iterator(directoryPath))
            ++counter;

        if (counter == 0)
        {
            std::cerr << "Directory " << directoryPath << " is empty\n";
            std::exit(1);
        }
    }

    JobManager::~JobManager()
    {}

    bool JobManager::UpdateJobs()
    {
        std::string path;
        unsigned outFile = 0, logFile = 0;

        for (const auto &entry : std::filesystem::directory_iterator(directoryPath))
        {
            path = entry.path();
            if (path.find(runningJobPostfix) != std::string::npos)
            {
                ++outFile;
                if (jobCollection.find(path) == jobCollection.end())
                {
                    jobCollection.emplace(path,Job(path));
                }
                jobCollection[path].Evaluate();
            }
            else if (path.find(finishedJobPostfix) != std::string::npos)
            {
                ++logFile;
                if (jobCollection.find(path) == jobCollection.end())
                {
                    jobCollection.emplace(path,Job(path));
                }
            }
        }

        runningCounter = outFile;
        finishedCounter = logFile;

        std::tie(remainingTime,avgFinishTime,ETA) = CalculateTimeLeft();

        return (outFile > 0) ? true : false;
    }

    ftxui::Elements JobManager::CreateTable()
    {
        bool flip = true;
        ftxui::Color titleColor = ftxui::Color::Default;
        
        ftxui::Elements tableContent;
        std::vector<Job> jobVec;
        for(auto &[key,job] : jobCollection) // passing map to vector for sorting 
            if (job.State() == Job::AnalysisState::Started)
                jobVec.push_back(job);
        std::sort(jobVec.begin(),jobVec.end(),[](const Job &j1, const Job &j2){return (j1.GetPercentage() > j2.GetPercentage());});

        for(auto &job : jobVec)
        {
            tableContent.push_back(
                ftxui::window(
                    ftxui::text(job.GetJobId() + ":" + job.GetTaskId()) | ftxui::center | ftxui::color(titleColor),
                    ftxui::vbox(
                        ftxui::text(job.PrintPercentage() + " %"),
                        ftxui::text(job.PrintRemainingTime())
                    )
                )
            );
            flip ? titleColor = ftxui::Color::Cyan : titleColor = ftxui::Color::Default;
            flip = !flip;
        }

        return tableContent;
    }

    ftxui::Color JobManager::GetColorByStatus(const Job::AnalysisState &state) const
    {
        switch (state)
        {
            case Job::AnalysisState::NotStarted :
                return ftxui::Color::GrayDark;

            case Job::AnalysisState::Started :
                return ftxui::Color::Yellow;
                
            case Job::AnalysisState::Finished :
                return ftxui::Color::Green;
                
            case Job::AnalysisState::Error :
                return ftxui::Color::Red;
        }
    }

    ftxui::Elements JobManager::CreateStatusBox()
    {
        ftxui::Elements elems;
        
        for (const auto &jobState : CreateJobStateVector())
        {
            elems.push_back(ftxui::text("   ") | ftxui::bgcolor(GetColorByStatus(jobState)));
        }

        return elems;
    }
    
    std::vector<Job::AnalysisState> JobManager::CreateJobStateVector()
    {
        auto jobIterator = jobCollection.begin();
        std::vector<Job::AnalysisState> stateVec;

        for (unsigned tot = 0; tot < totalJobs; ++tot)
        {
            if (tot < jobCollection.size())
            {
                stateVec.push_back(jobIterator->second.State());
                jobIterator++;
            }
            else
            {
                stateVec.push_back(Job::AnalysisState::NotStarted);
            }
        }
        return stateVec;
    }

    ftxui::Elements JobManager::CreateJobErrorMsgVector()
    {
        ftxui::Elements vec;
        for (const auto &[key,job] : jobCollection)
        {
            if (job.State() == Job::AnalysisState::Error)
            {
                vec.push_back(ftxui::hbox(
                    ftxui::text(job.GetJobId() + job.GetTaskId() + " = ") | ftxui::color(ftxui::Color::Orange1),
                    ftxui::text(job.GetErrorMsg())
                ));
            }
        }

        return vec;
    }

    std::tuple<std::chrono::seconds,std::chrono::seconds,std::chrono::high_resolution_clock::time_point> JobManager::CalculateTimeLeft() const
    {
        std::chrono::seconds prevRunTime = std::chrono::seconds(0),
            futureRunTime = std::chrono::seconds(0),
            largestCurrentRemainingTime = std::chrono::seconds(0),
            totRemainingTime = std::chrono::seconds(0);
        for (const auto &[key,job] : jobCollection)
        {
            if (job.State() == Job::AnalysisState::Finished)
            {
                prevRunTime += job.GetRunTime();
            }
            else if (job.State() == Job::AnalysisState::Started) // find largest currenlty runinig remaining time
            {
                if (largestCurrentRemainingTime <= job.GetRemainingTime())
                    largestCurrentRemainingTime = job.GetRemainingTime();
            }
        }

        prevRunTime = prevRunTime/finishedCounter; // how long (on average) did finished jobs run
        futureRunTime = prevRunTime * ((totalJobs-finishedCounter-runningCounter)/runningCounter); // estimate how long the jobs which have not yet strted will take
        totRemainingTime = prevRunTime+largestCurrentRemainingTime+futureRunTime;

        return std::make_tuple(totRemainingTime,prevRunTime,std::chrono::system_clock::now() + totRemainingTime);
    }

    std::stringstream JobManager::MakeTime(std::chrono::seconds sec)
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

    ftxui::Element JobManager::PrintStatus(bool minimal, bool full)
    {
        ftxui::Elements contents,errors;
        const std::time_t ETAtime = std::chrono::system_clock::to_time_t(ETA);

        contents.push_back(ftxui::vbox(
                    ftxui::hbox(
                        ftxui::text("Total number of jobs: " + std::to_string(totalJobs)),
                        ftxui::filler(),
                        ftxui::text("Predicted remaining time: " + MakeTime(remainingTime).str())
                    ),
                    ftxui::hbox(
                        ftxui::text("Finished jobs: " + std::to_string(finishedCounter)),
                        ftxui::filler(),
                        ftxui::text("Predicted ETA: " + std::string(std::ctime(&ETAtime))) 
                    ),
                    ftxui::hbox(
                        ftxui::text("Currently running: " + std::to_string(runningCounter)),
                        ftxui::filler(),
                        ftxui::text("Previous average job runtime: " + MakeTime(avgFinishTime).str()) 
                    )
                ) | ftxui::border);

        errors = std::move(CreateJobErrorMsgVector());
        if (errors.size() > 0)
        {
            contents.push_back(ftxui::vbox(std::move(errors)) | ftxui::border);
        }

        if (full)
        {
            contents.push_back(ftxui::vbox(
                    ftxui::text("Currenlty running jobs") | ftxui::center,
                    ftxui::separator(),
                    ftxui::hflow(std::move(CreateTable()))) | ftxui::border
                );
        }
        else if (!minimal)
        {
            contents.push_back(
                ftxui::vbox(
                ftxui::hbox(
                    ftxui::text("Finished = "),
                    ftxui::text("   ") | ftxui::bgcolor(ftxui::Color::Green),
                    ftxui::text("   Running = "),
                    ftxui::text("   ") | ftxui::bgcolor(ftxui::Color::Yellow),
                    ftxui::text("   Pending = "),
                    ftxui::text("   ") | ftxui::bgcolor(ftxui::Color::GrayDark),
                    ftxui::text("   Error = "),
                    ftxui::text("   ") | ftxui::bgcolor(ftxui::Color::Red)) | ftxui::center,
                ftxui::separator(),
                ftxui::hflow(std::move(CreateStatusBox()))) | ftxui::border);
        }
        return ftxui::vbox(std::move(contents));
    }

} // namespace SJM
