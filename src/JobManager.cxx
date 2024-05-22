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

    ftxui::Table JobManager::CreateTable()
    {
        std::vector<std::vector<std::string> > tableContent{{"Job Id","Task Id","Current %","Remaining Time"}};

        for(auto &[key,job] : jobCollection)
        {
            tableContent.push_back(std::vector<std::string>({job.GetJobId(),job.GetTaskId(),job.GetPercentage(),job.PrintRemainingTime()}));
        }

        auto table = ftxui::Table(tableContent);

        table.SelectAll().Border(ftxui::LIGHT);

        table.SelectRow(0).Decorate(ftxui::bold);
        table.SelectRow(0).SeparatorVertical(ftxui::LIGHT);
        table.SelectRow(0).Border(ftxui::DASHED);

        table.SelectColumn(2).DecorateCells(ftxui::center);
        table.SelectColumn(3).DecorateCells(ftxui::center);

        auto content = table.SelectRows(1, -1);
        content.DecorateCellsAlternateRow(ftxui::color(ftxui::Color::Cyan), 2, 0);
        content.DecorateCellsAlternateRow(ftxui::color(ftxui::Color::White), 2, 1);

        return table;
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
        ftxui::Elements columnsAndRows;
        
        for (const auto &inner : CreateJobStateMatrix())
        {
            ftxui::Elements row;
            for (const auto &elem : inner)
            {
                row.push_back(ftxui::text("   ") | ftxui::bgcolor(GetColorByStatus(elem)));
            }
            columnsAndRows.push_back(ftxui::hbox(std::move(row)));
        }

        return columnsAndRows;
    }

    std::vector<std::vector<Job::AnalysisState> > JobManager::CreateJobStateMatrix()
    {
        auto jobIterator = jobCollection.begin();
        std::vector<std::vector<Job::AnalysisState> > matrix;
        std::vector<Job::AnalysisState> rowVec;

        for (unsigned tot = 0; tot < totalJobs; ++tot)
        {
            if (tot < finishedCounter)
            {
                rowVec.push_back(Job::AnalysisState::Finished);
            }
            else if (tot < jobCollection.size() + finishedCounter)
            {
                rowVec.push_back(jobIterator->second.State());
                jobIterator++;
            }
            else
            {
                rowVec.push_back(Job::AnalysisState::NotStarted);
            }

            if (tot % statusBoxWidth == statusBoxWidth - 1)
            {
                matrix.push_back(rowVec);
                rowVec = std::vector<Job::AnalysisState>();
            }
        }
        matrix.push_back(rowVec);

        return matrix;
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
            contents.push_back(CreateTable().Render());
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
                CreateStatusBox()) | ftxui::border);
        }
        return ftxui::vbox(std::move(contents));
    }

} // namespace SJM
