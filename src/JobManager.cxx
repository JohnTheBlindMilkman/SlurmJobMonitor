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
                else if (jobCollection[path].State() == Job::AnalysisState::Finished)
                {
                    jobCollection.erase(path);
                }
                jobCollection[path].Evaluate();
            }
            else if (path.find(finishedJobPostfix) != std::string::npos)
            {
                ++logFile;
            }
        }

        runningCounter = outFile;
        finishedCounter = logFile;

        return (outFile > 0) ? true : false;
    }

    ftxui::Elements JobManager::CreateTable()
    {
        ftxui::Elements tableContent;
        std::vector<Job> jobVec;
        for(auto &[key,job] : jobCollection) // passing map to vector for sorting 
            jobVec.push_back(job);
        std::sort(jobVec.begin(),jobVec.end(),[](Job j1, Job j2){return (j1.GetPercentage() > j2.GetPercentage());});

        for(auto &job : jobVec)
        {
            tableContent.push_back(
                ftxui::window(
                    ftxui::text(job.GetJobId() + ":" + job.GetTaskId()) | ftxui::center,
                    ftxui::vbox(
                        ftxui::text(job.PrintPercentage() + " %\n"),
                        ftxui::text(job.PrintRemainingTime())
                    )
                )
            );
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
            if (tot < finishedCounter)
            {
                stateVec.push_back(Job::AnalysisState::Finished);
            }
            else if (tot < jobCollection.size() + finishedCounter)
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

    ftxui::Element JobManager::PrintStatus(bool minimal, bool full)
    {
        ftxui::Elements contents;
        contents.push_back(ftxui::vbox(
                ftxui::text("Total number of jobs: " + std::to_string(totalJobs)),
                ftxui::text("Finished jobs: " + std::to_string(finishedCounter)),
                ftxui::text("Currently running: " + std::to_string(runningCounter))
                ) | ftxui::border);

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
