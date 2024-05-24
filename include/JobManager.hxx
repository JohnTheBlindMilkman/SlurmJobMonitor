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
                static constexpr unsigned statusBoxWidth{32};

                ftxui::Elements CreateTable();
                ftxui::Elements CreateStatusBox();
                ftxui::Color GetColorByStatus(const Job::AnalysisState &) const;
                std::vector<Job::AnalysisState> CreateJobStateVector();

                std::map<std::string,Job> jobCollection;
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