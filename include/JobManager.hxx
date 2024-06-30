/**
 * @file JobManager.hxx
 * @author Jędrzej Kołaś (jedrzej.kolas.dokt@pw.edu.pl)
 * @brief Container-like class for storing and managing Job class objects, and estimating runtime of the whole submitted job batch
 * @version 1.2.2
 * @date 2024-06-02
 * 
 * @copyright Copyright (c) 2024
 * 
 */
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

                /**
                 * @brief Create a Table object 
                 * 
                 * @return ftxui::Elements 
                 */
                ftxui::Elements CreateTable();
                /**
                 * @brief Create a Status Box object
                 * 
                 * @return ftxui::Elements 
                 */
                ftxui::Elements CreateStatusBox();
                /**
                 * @brief Get a color corresponding to Job AnalysisStatus
                 * 
                 * @return ftxui::Color 
                 */
                ftxui::Color GetColorByStatus(const Job::AnalysisState &) const;
                /**
                 * @brief Create a Job State Vector object
                 * 
                 * @return std::vector<Job::AnalysisState> 
                 */
                std::vector<Job::AnalysisState> CreateJobStateVector();
                /**
                 * @brief Create a Job Error Message Vector object
                 * 
                 * @return ftxui::Elements 
                 */
                ftxui::Elements CreateJobErrorMsgVector();
                /**
                 * @brief Calculate the time variables related to the whole batch. Based on past runtime (.log files) the future runtime is calculated and added to the current longest running job. This gives the information ablut when the jobs are expected to finish.
                 * 
                 * @return std::tuple<std::chrono::seconds,std::chrono::seconds,std::chrono::high_resolution_clock::time_point> 
                 */
                std::tuple<std::chrono::seconds,std::chrono::seconds,std::chrono::high_resolution_clock::time_point> CalculateTimeLeft() const;
                /**
                 * @brief Create user friendly time format from std::chrono::duration type
                 * 
                 * @return std::stringstream 
                 */
                std::stringstream MakeTime(std::chrono::seconds);
                std::pair<unsigned,unsigned> DirecotryContents(const std::string &dirPath) const;

                std::map<std::string,Job> jobCollection;
                std::chrono::seconds avgFinishTime,remainingTime;
                std::chrono::high_resolution_clock::time_point ETA;
                std::string directoryPath;
                unsigned runningCounter,finishedCounter,totalJobs;

            public:
                /**
                 * @brief Construct a new Job Manager object
                 * 
                 * @param dirPath Path to the output directory of SLURM
                 * @param njobs Total number of jobs submited to the batchfarm
                 */
                JobManager(std::string dirPath,unsigned njobs);
                /**
                 * @brief Destroy the Job Manager object
                 * 
                 */
                ~JobManager();

                /**
                 * @brief Called to read all the currently running jobs, update their status, time, and progress
                 * 
                 * @return true If jobs are still running
                 * @return false In no jobs are left
                 */
                bool UpdateJobs();
                /**
                 * @brief Get the whole GUI to be printed in the terminal 
                 * 
                 * @param minimal program flag
                 * @param full program flag
                 * @return ftxui::Element 
                 */
                ftxui::Element PrintStatus(bool minimal, bool full);
        };
    } // namespace SJM
    

#endif