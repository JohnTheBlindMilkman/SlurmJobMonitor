#ifndef Job_hxx
    #define Job_hxx

    #include "nlohmann/json.hpp"

    #include <chrono>

    namespace SJM
    {
        /**
         * @brief Helper struct for holding all necessary information
         * 
         */
        struct JobStruct
        {
            std::string exitCodeStatus,node,partition,currentState,stateReason,name;
            long unsigned jobId,taskId,elapsedTime,maxTime,startTime,endTime,submissionTime,priority,usedMemory,maxMemory;
            std::vector<std::string> flags;
        };
        /**
         * @brief Helper struct for holding number of tasks in job array
         * 
         */
        struct JobArrayStruct
        {
            std::string nTasks;
        };
        /**
         * @brief Override of nlohmann::json method for data serialisation
         * 
         * @param j input nlohmann::json object
         * @param job output Job struct
         */
        void from_json(const nlohmann::json &j,JobStruct &job);
        /**
         * @brief Override of nlohmann::json method for data serialisation
         * 
         * @param j input nlohmann::json object
         * @param job output Job Array struct
         */
        void from_json(const nlohmann::json &j,JobArrayStruct &job);

        class Job
        {
            public:
                enum class State
                {
                    Requeued,
                    Resizing,
                    Pending,
                    Running,
                    Completed,
                    Failed,
                    NodeFail,
                    OutOfMemory,
                    Revoked,
                    Preempted,
                    Suspended,
                    Timeout,
                    Deadline,
                    Cancelled,
                    BootFail
                };
                enum class Partition
                {
                    Main,
                    Long,
                    Grid,
                    HighMem,
                    Gpu,
                    Debug,
                    New
                };

                Job(const JobStruct &j);
                [[nodiscard]] State GetState() const noexcept;
                [[nodiscard]] Partition GetPartition() const noexcept;
                [[nodiscard]] std::string GetNode() const noexcept;
                [[nodiscard]] std::string GetName() const noexcept;
                [[nodiscard]] unsigned long GetJobId() const noexcept;
                [[nodiscard]] unsigned long GetTaskId() const noexcept;
                [[nodiscard]] unsigned long GetUsedMem() const noexcept;
                [[nodiscard]] unsigned long GetRequestedMem() const noexcept;
                [[nodiscard]] std::chrono::seconds GetElapsedTime() const noexcept;
                [[nodiscard]] std::chrono::seconds GetMaxTime() const noexcept;
                [[nodiscard]] std::chrono::system_clock::time_point GetStartTime() const noexcept;
                [[nodiscard]] std::chrono::system_clock::time_point GetEndTime() const noexcept;
                [[nodiscard]] std::chrono::system_clock::time_point GetSubTime() const noexcept;
                [[nodiscard]] std::vector<std::string> GetListOfFlags() const noexcept;

            private:
                [[nodiscard]] std::chrono::seconds MakeDuration(unsigned long) const noexcept;
                [[nodiscard]] std::chrono::system_clock::time_point MakeTimePoint(unsigned long) const noexcept;

                State m_currentState;
                Partition m_partition;
                const std::map<std::string,State> m_stateMap;
                const std::map<std::string,Partition> m_partitionMap;
                std::string m_exitCodeStatus,m_node,m_stateReason,m_name;
                long unsigned m_jobId,m_taskId,m_priority,m_usedMemory,m_maxMemory;
                std::chrono::seconds m_elapsedTime,m_maxTime;
                std::chrono::system_clock::time_point m_startTime,m_endTime,m_submissionTime;
                std::vector<std::string> m_flags;
        };

        inline std::chrono::seconds Job::MakeDuration(unsigned long time) const noexcept {return std::chrono::seconds(time);}

        inline std::chrono::system_clock::time_point Job::MakeTimePoint(unsigned long time) const noexcept 
        {
            return std::chrono::system_clock::time_point(std::chrono::seconds(time));
        }

        inline Job::State Job::GetState() const noexcept {return m_currentState;}
        inline Job::Partition Job::GetPartition() const noexcept {return m_partition;}
        inline std::string Job::GetNode() const noexcept {return m_node;}
        inline std::string Job::GetName() const noexcept {return m_name;}
        inline unsigned long Job::GetJobId() const noexcept {return m_jobId;}
        inline unsigned long Job::GetTaskId() const noexcept {return m_taskId;}
        inline unsigned long Job::GetUsedMem() const noexcept {return m_usedMemory;}
        inline unsigned long Job::GetRequestedMem() const noexcept {return m_maxMemory;}
        inline std::chrono::seconds Job::GetElapsedTime() const noexcept {return m_elapsedTime;}
        inline std::chrono::seconds Job::GetMaxTime() const noexcept {return m_maxTime;}
        inline std::chrono::system_clock::time_point Job::GetStartTime() const noexcept {return m_startTime;}
        inline std::chrono::system_clock::time_point Job::GetEndTime() const noexcept {return m_endTime;}
        inline std::chrono::system_clock::time_point Job::GetSubTime() const noexcept {return m_submissionTime;}
        inline std::vector<std::string> Job::GetListOfFlags() const noexcept {return m_flags;}

    } // namespace SJM
    

#endif