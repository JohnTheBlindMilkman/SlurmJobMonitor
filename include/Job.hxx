#ifndef Job_hxx
    #define Job_hxx

    #include "FileHandler.hxx"
    #include "GlobalConstants.hxx"

    #include <chrono>
    #include <iomanip>
    #include <ostream>

    namespace SJM
    {
        class Job
        {
            public:
                enum class AnalysisState{Error,NotStarted,Started,Finished};

                Job() = default;
                explicit Job(const std::string &);
                ~Job() = default;
                Job(const Job&) = default;
                Job& operator=(const Job&) = default;
                Job(Job&&) = default;
                Job& operator=(Job&&) = default;

                bool Evaluate();
                std::string GetPercentage() const;
                std::string GetJobId() const;
                std::string GetTaskId() const;
                AnalysisState State() const;
                std::string_view PrintState() const;
                std::string PrintRemainingTime();
                std::string PrintElapsedTime();
                std::string PrintEtaTime();

            private:
                AnalysisState EvalState();
                std::stringstream MakeTime(std::chrono::seconds);
                void CalculateTime();

                bool hasStarted,hasFinished;
                int currentPercentage,lastPercentage;
                std::string fileContents;
                std::string percent,jobId,taskId,fileName;
                AnalysisState state;
                std::chrono::steady_clock::time_point currentTime,lastTime;
                std::chrono::seconds elapsedTime,ETA,remainigTime;
        };
        inline std::string Job::GetPercentage() const
        {
            return std::to_string(currentPercentage);
        }

        inline std::string Job::GetJobId() const
        {
            return jobId;
        }

        inline std::string Job::GetTaskId() const
        {
            return taskId;
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
        inline std::string Job::PrintRemainingTime()
        {
            return MakeTime(remainigTime).str();
        }
        inline std::string Job::PrintElapsedTime()
        {
            return MakeTime(elapsedTime).str();
        }
        inline std::string Job::PrintEtaTime()
        {
            return MakeTime(ETA).str();
        }
    } // namespace SJM
    

#endif