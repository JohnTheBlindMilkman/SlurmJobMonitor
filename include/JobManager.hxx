#ifndef JobManager_hxx
    #define JobManager_hxx

    #include "Job.hxx"
    #include "ftxui/dom/elements.hpp"
    #include "ftxui/screen/screen.hpp"
    #include "ftxui/dom/table.hpp"

    #include <filesystem>
    #include <map>
    #include <iostream>
    #include <cmath>

    namespace SJM
    {
        class JobManager
        {
            private:
                static constexpr std::string_view finishedJobPostfix{".log"};
                static constexpr std::string_view runningJobPostfix{".out"};
                static constexpr unsigned maxRunningJobs{400};

                ftxui::Elements CreateTable();
                ftxui::Elements CreateStatusBox();
                ftxui::Color GetColorByStatus(const Job::AnalysisState &) const;
                std::vector<Job::AnalysisState> CreateJobStateVector();
                std::tuple<std::chrono::seconds,std::chrono::seconds,std::chrono::high_resolution_clock::time_point> CalculateTimeLeft() const;
                std::stringstream MakeTime(std::chrono::seconds);

                std::map<std::string,Job> jobCollection;
                std::chrono::seconds avgFinishTime,remainingTime;
                std::chrono::high_resolution_clock::time_point ETA;
                std::string directoryPath;
                unsigned runningCounter,finishedCounter,totalJobs;

            public:
                JobManager(std::string dirPath,unsigned njobs);
                ~JobManager();

                bool UpdateJobs();
                ftxui::Element PrintStatus(bool minimal, bool full);
        };
    } // namespace SJM
    

#endif