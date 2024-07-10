/**
 * @file JobManager.hxx
 * @author Jędrzej Kołaś (jedrzej.kolas.dokt@pw.edu.pl)
 * @brief Container-like class for storing and managing Job class objects, and estimating runtime of the whole submitted job batch
 * @version 2.0.0
 * @date 2024-06-02
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef JobManager_hxx
    #define JobManager_hxx

    #include "Graphics.hxx"

    #include <cstdlib>
    #include <iostream>
    #include <fstream>
    #include <chrono>
    #include <utility>

    namespace SJM
    {
        class JobManager
        {
            public:
                /**
                 * @brief Construct a new Job Manager object
                 * 
                 * @param njobs number of jobs which the user submitted
                 * @param username name of the user for whom the jobs should be monitored
                 * @param jobIds collection of SLURM job ids to be monitored
                 */
                JobManager(std::size_t njobs, const std::string &username, const std::vector<unsigned long> &jobIds) noexcept;
                /**
                 * @brief Called to read information about all the specified jobs
                 * 
                 * @return true If jobs are still running
                 * @return false In no jobs are left
                 */
                bool UpdateJobs();
                /**
                 * @brief Callaed to update the terminal GUI (so TUI I guess???)
                 * 
                 */
                void UpdateGui();

            private:

                [[nodiscard]] std::string ParseVector(const std::vector<unsigned long> &vec) const noexcept;
                std::string ExecuteCommand(const std::optional<std::string> &username,const std::optional<std::vector<unsigned long> > &jobIds);
                [[nodiscard]] nlohmann::json ReadJson(const std::string_view &strView);
                [[nodiscard]] std::vector<Job> FromJsonToJobVector(const nlohmann::json &j);
                [[nodiscard]] std::size_t CountJobsByState(const std::vector<Job> &vec, Job::State state) const;
                std::tuple<std::chrono::seconds,double> PopulateVariables(const std::vector<Job> &jobVec);
                [[nodiscard]] std::string PrintTime(std::chrono::seconds time) const;
                [[nodiscard]] std::string PrintTime(std::chrono::system_clock::time_point time) const;

                static constexpr std::string_view m_pathToJson{"./sacct.json"};
                static constexpr double m_toGiga = 1./1024/1024/1024;

                const std::size_t m_totalJobs;
                std::string m_resetPos;
                std::string m_userName;
                const std::vector<unsigned long> m_jobIdsVector;
                std::vector<Job> m_jobCollection;
                std::chrono::seconds m_averageRunTime, m_remainingTime;
                std::chrono::system_clock::time_point m_eta;
                std::size_t m_numberOfJobs, m_finishedCounter, m_runningCounter, m_pendingCounter, m_failedCounter, m_requeueCounter, m_resizeCounter, m_suspendedCounter;
                double m_totalMemAssigned, m_predictedTotalMemUsed, m_averagePastMemUsed;
                bool m_hasJobsWithFinishedState;
                Graphics m_gui;

        };
    } // namespace SJM
    

#endif