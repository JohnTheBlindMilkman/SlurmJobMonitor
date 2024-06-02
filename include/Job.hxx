/**
 * @file Job.hxx
 * @author Jędrzej Kołaś (jedrzej.kolas.dokt@pw.edu.pl)
 * @brief Class describing the Job object
 * @version 1.2.2
 * @date 2024-06-02
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef Job_hxx
    #define Job_hxx

    #include "FileHandler.hxx"

    #include <numeric>
    #include <deque>
    #include <chrono>
    #include <iomanip>
    #include <iostream>

    namespace SJM
    {
        class Job
        {
            public:
                enum class AnalysisState{Error,NotStarted,Started,Finished};

                /**
                 * @brief Construct a new Job object
                 * 
                 */
                Job() = default;
                /**
                 * @brief Construct a new Job object
                 * 
                 */
                explicit Job(const std::string &);
                /**
                 * @brief Destroy the Job object
                 * 
                 */
                ~Job() = default;
                /**
                 * @brief Construct a new Job object
                 * 
                 */
                Job(const Job&) = default;
                /**
                 * @brief Standard copy assignment operator
                 * 
                 * @return Job& 
                 */
                Job& operator=(const Job&) = default;
                /**
                 * @brief Construct a new Job object
                 * 
                 */
                Job(Job&&) = default;
                /**
                 * @brief Standard move assignment operator
                 * 
                 * @return Job& 
                 */
                Job& operator=(Job&&) = default;

                /**
                 * @brief Main Method for populating and updating all the fileds in Job class. It internally calls to open and read the file, updates state and all progress informations.
                 * 
                 * @return true 
                 * @return false 
                 */
                bool Evaluate();
                /**
                 * @brief Get the Percentage value
                 * 
                 * @return int 
                 */
                int GetPercentage() const;
                /**
                 * @brief Get the Run Time object
                 * 
                 * @return std::chrono::seconds 
                 */
                std::chrono::seconds GetRunTime() const;
                /**
                 * @brief Get the Remaining Time object
                 * 
                 * @return std::chrono::seconds 
                 */
                std::chrono::seconds GetRemainingTime() const;
                /**
                 * @brief Get the Job Id object
                 * 
                 * @return std::string 
                 */
                std::string GetJobId() const;
                /**
                 * @brief Get the Task Id object
                 * 
                 * @return std::string 
                 */
                std::string GetTaskId() const;
                /**
                 * @brief Get the Error Msg object
                 * 
                 * @return std::string 
                 */
                std::string GetErrorMsg() const;
                /**
                 * @brief Get the State of the job
                 * 
                 * @return AnalysisState 
                 */
                AnalysisState State() const;
                /**
                 * @brief Get the State of the job in string format
                 * 
                 * @return std::string_view 
                 */
                std::string_view PrintState() const;
                /**
                 * @brief Get percentage in stirng format
                 * 
                 * @return std::string 
                 */
                std::string PrintPercentage() const;
                /**
                 * @brief Get remaining time in stirng format
                 * 
                 * @return std::string 
                 */
                std::string PrintRemainingTime();
                /**
                 * @brief Get elapsed time in stirng format
                 * 
                 * @return std::string 
                 */
                std::string PrintElapsedTime();
                /**
                 * @brief Get Estimated Time of Arrival (ETA) in stirng format
                 * 
                 * @return std::string 
                 */
                std::string PrintEtaTime();
                /**
                 * @brief Get the run time in stirng format
                 * 
                 * @return std::string 
                 */
                std::string PrintRunTime();

            private:
                /**
                 * @brief Calcuate average value of std::deque elements
                 * 
                 * @return std::chrono::seconds 
                 */
                std::chrono::seconds CalcAverage(const std::deque<std::chrono::seconds> &);
                /**
                 * @brief Calcuate average value of std::deque elements
                 * 
                 * @return unsigned 
                 */
                unsigned CalcAverage(const std::deque<unsigned> &);
                /**
                 * @brief Try to read the file and upon its contents evaluate at what state the analysis is
                 * 
                 * @return AnalysisState 
                 */
                AnalysisState EvalState();
                /**
                 * @brief Create user friendly time format from std::chrono::duration type
                 * 
                 * @return std::stringstream 
                 */
                std::stringstream MakeTime(std::chrono::seconds);
                /**
                 * @brief Try to read the real run time from a finished analysis file
                 * 
                 * @return std::chrono::seconds 
                 */
                std::chrono::seconds ReadTime(const std::string &&);
                /**
                 * @brief Calculate all fields realted to the time (elapsed,remaining,eta) and the analysis percentile progress
                 * 
                 */
                void CalculateTime();

                static constexpr std::size_t maxQueueSize{5};
                static constexpr std::string_view realTimePrefix{"real	"};
                static constexpr std::string_view beginingExpression{"with entries ="};
                static constexpr std::string_view endingExpression{"Finished DST processing"};
                static constexpr std::string_view percentExpression{" %"};
                static constexpr int substrOffset{3};
                static constexpr int substrLength{3};
                static constexpr int maxPercentage{99};
                static constexpr int maxErrorCount{5};

                bool hasStarted,hasFinished;
                int currentPercentage,lastPercentage,avgPercentage,errorCounter;
                std::string fileContents,percent,jobId,taskId,fileName,errorMessage;
                AnalysisState state;
                std::chrono::system_clock::time_point currentTime,lastTime;
                std::chrono::seconds avgElapsedTime,ETA,remainigTime,realRunTime;
                std::deque<std::chrono::seconds> elapsedQueue;
                std::deque<unsigned> progressQueue;
        };
        inline std::chrono::seconds Job::CalcAverage(const std::deque<std::chrono::seconds> &queue)
        {
            return std::accumulate(queue.begin(),queue.end(),std::chrono::seconds(0)) / queue.size();
        }

        inline unsigned Job::CalcAverage(const std::deque<unsigned> &queue)
        {
            return std::accumulate(queue.begin(),queue.end(),0) / queue.size();
        }

        inline int Job::GetPercentage() const
        {
            return currentPercentage;
        }

        inline std::chrono::seconds Job::GetRunTime() const
        {
            return realRunTime;
        }

        inline std::chrono::seconds Job::GetRemainingTime() const
        {
            return remainigTime;
        }

        inline std::string Job::GetJobId() const
        {
            return jobId;
        }

        inline std::string Job::GetTaskId() const
        {
            return taskId;
        }

        inline std::string Job::GetErrorMsg() const
        {
            return errorMessage;
        }

        inline Job::AnalysisState Job::State() const
        {
            return state;
        }

        inline std::string_view Job::PrintState() const
        {
            switch (state)
            {
                case Job::AnalysisState::NotStarted:
                    return "Not Started";

                case Job::AnalysisState::Started:
                    return "Started";

                case Job::AnalysisState::Finished:
                    return "Finished";

                case Job::AnalysisState::Error:
                    return "Error";
            }
            return "";
        }
        inline std::string Job::PrintPercentage() const
        {
            return std::to_string(currentPercentage);
        }
        inline std::string Job::PrintRemainingTime()
        {
            return MakeTime(remainigTime).str();
        }
        inline std::string Job::PrintElapsedTime()
        {
            return MakeTime(avgElapsedTime).str();
        }
        inline std::string Job::PrintEtaTime()
        {
            return MakeTime(ETA).str();
        }
        inline std::string Job::PrintRunTime()
        {
            return MakeTime(realRunTime).str();
        }
    } // namespace SJM
    

#endif