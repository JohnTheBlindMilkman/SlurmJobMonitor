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

    #include "ftxui/dom/elements.hpp"
    #include "ftxui/screen/screen.hpp"
    #include "ftxui/dom/table.hpp"

    #include "nlohmann/json.hpp"

    #include <cstdlib>
    #include <iostream>
    #include <fstream>
    #include <chrono>
    #include <utility>

    namespace SJM
    {
        /**
         * @brief Helper struct for holding all necessary information
         * 
         */
        struct Job
        {
            std::string exitCodeStatus,node,partition,qos,currentState,stateReason;
            long unsigned jobId,taskId,elapsedTime,maxTime,startTime,endTime,submissionTime,priority,usedMemory,maxMemory;
            std::vector<std::string> flags;
        };
        /**
         * @brief Override of nlohmann::json method for data serialisation
         * 
         * @param j input nlohmann::json object
         * @param job output Job struct
         */
        void from_json(const nlohmann::json &j,Job &job);

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
                JobManager(std::size_t njobs, const std::optional<std::string> &username, const std::optional<std::vector<unsigned long> > &jobIds) noexcept;
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
                enum class State
                {
                    Pending,
                    Running,
                    Completed,
                    Failed,
                    Timeout,
                    Resizing,
                    Deadline,
                    NodeFail
                };

                [[nodiscard]] std::string ParseVector(const std::vector<unsigned long> &vec) const noexcept;
                std::string ExecuteCommand(const std::optional<std::string> &username,const std::optional<std::vector<unsigned long> > &jobIds);
                [[nodiscard]] nlohmann::json ReadJson(const std::string_view &strView);
                [[nodiscard]] std::vector<Job> FromJsonToJobVector(const nlohmann::json &j);
                [[nodiscard]] std::size_t CountJobsByState(const std::vector<Job> &vec, State state) const;

                static constexpr std::string_view m_pathToJson{"./sacct.json"};

                std::optional<std::string> m_userName;
                std::optional<std::vector<unsigned long> > m_jobIdsVector;
                std::vector<Job> m_jobCollection;
                std::chrono::seconds m_averageRunTime, m_eta, m_remainingTime;
                std::size_t m_numberOfJobs, m_finishedCounter, m_runningCounter, m_pendingCounter, m_failedCounter, m_timeoutCounter, m_resizingCounter, m_deadlineCounter, m_nodeFailCounter;
                double m_totalMemAssigned, m_predictedTotalMemUsed, m_averagePastMemUsed;
                bool m_hasJobsWithFinishedState;
                const std::map<State,std::string> m_stateMap;
                const std::size_t m_totalJobs;

        };
    } // namespace SJM
    

#endif