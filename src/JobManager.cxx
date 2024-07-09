#include "JobManager.hxx"

namespace SJM
{
    JobManager::JobManager(std::size_t njobs,const std::optional<std::string> &username,const std::optional<std::vector<unsigned long> > &jobIds) noexcept : 
    m_totalJobs(njobs), m_userName(username), m_jobIdsVector(jobIds), m_jobCollection({}), m_averageRunTime(std::chrono::seconds(0)),
    m_eta(std::chrono::seconds(0)), m_remainingTime(std::chrono::seconds(0)), m_numberOfJobs(0), m_finishedCounter(0), m_runningCounter(0), 
    m_pendingCounter(0), m_failedCounter(0), m_completingCounter(0), m_preemptedCounter(0), m_suspendedCounter(0), m_stoppedCounter(0),
    m_totalMemAssigned(0.), m_predictedTotalMemUsed(0.), m_averagePastMemUsed(0.), m_hasJobsWithFinishedState(false), m_gui()
    {
    }

    bool JobManager::UpdateJobs()
    {
        //std::cout << ExecuteCommand(m_userName,m_jobIdsVector) << "\n";
        ExecuteCommand(m_userName,m_jobIdsVector);
        m_jobCollection = FromJsonToJobVector(ReadJson(m_pathToJson));
        std::tie(m_averageRunTime,m_averagePastMemUsed) = PopulateVariables(m_jobCollection);
        /* std::cout << "Running: " << m_runningCounter << "\n";
        std::cout << "Finished: " << m_finishedCounter << "\n";
        std::cout << "All: " << m_totalJobs << "\n";
        std::cout << "Avg run time: " << m_averageRunTime.count() << "\n";
        std::cout << "Avg mem used: " << m_averagePastMemUsed << "\n";
        std::cout << "Name: " << m_userName.value_or("unknown") << "\n"; */

        if (m_totalJobs < m_runningCounter + m_finishedCounter)
            throw std::runtime_error("njobs is smaller than running jobs + finished jobs");

        m_pendingCounter = m_totalJobs - m_runningCounter - m_finishedCounter;

        (m_finishedCounter > 0) ? m_hasJobsWithFinishedState = true : m_hasJobsWithFinishedState = false;

        return ((m_runningCounter + m_pendingCounter) > 0) ? true : false;
    }

    void JobManager::UpdateGui()
    {
        std::string resetPos;
        auto document = m_gui.PrintStatus(m_jobCollection,{m_userName.value_or("unknown"),m_totalJobs,m_finishedCounter,m_runningCounter,m_averagePastMemUsed,m_jobCollection.at(0).GetRequestedMem()/1000,m_hasJobsWithFinishedState});
        auto screen = ftxui::Screen::Create(ftxui::Dimension::Full(),ftxui::Dimension::Fit(document));
        ftxui::Render(screen, document);
        std::cout << resetPos;
        screen.Print();
        resetPos = screen.ResetPosition();
    }

    std::string JobManager::ParseVector(const std::vector<unsigned long> &vec) const noexcept
    {
        std::string outputCommand = " ";
        for (const auto &elem : vec)
            outputCommand += std::to_string(elem) + ",";

        return outputCommand;
    }

    std::string JobManager::ExecuteCommand(const std::optional<std::string> &username,const std::optional<std::vector<unsigned long> > &jobIds)
    {
        std::string userFlag = (username.has_value()) ? "-u " + username.value() + " " : "";
        std::string jobidFlag = (jobIds.has_value()) ? "-j " + ParseVector(jobIds.value()) : ""; // Comment from "man sacct": -S: Select jobs eligible after this time. Default is 00:00:00 of the current day

        std::string command = "sacct " + userFlag + jobidFlag + " --json > sacct.json";
        std::system(command.data());

        return command;
    }

    nlohmann::json JobManager::ReadJson(const std::string_view &strView)
    {
        std::ifstream f(strView.data());
        nlohmann::json data;
        try
        {
            data = nlohmann::json::parse(f);
        }
        catch (nlohmann::json::parse_error& ex)
        {
            std::cerr << "parse error at byte " << ex.byte << std::endl;
        }

        return data;
    }

    std::vector<Job> JobManager::FromJsonToJobVector(const nlohmann::json &j)
    {
        std::vector<Job> jobVec;
        JobStruct jobStruct;
        for (const auto &job : j["jobs"]) // I should check somewhere if I get any jobs at all
        {
            jobStruct = job.get<JobStruct>();
            if (jobStruct.taskId != 0)
                jobVec.push_back(Job(jobStruct));
        }

        return jobVec;
    }

    std::size_t JobManager::CountJobsByState(const std::vector<Job> &vec, Job::State state) const
    {
        return std::count_if(vec.begin(),vec.end(),[&state](const Job &j){return state == j.GetState();});
    }

    std::tuple<std::chrono::seconds,double> JobManager::PopulateVariables(const std::vector<Job> &jobVec)
    {
        m_numberOfJobs = jobVec.size();
        m_pendingCounter = 0;
        m_finishedCounter = 0;
        m_runningCounter = 0;
        m_completingCounter = 0;
        m_failedCounter = 0;
        m_preemptedCounter = 0;
        m_suspendedCounter = 0;
        m_stoppedCounter = 0;
        double sumUsedMem = 0;
        std::chrono::seconds sumRunTime(0);

        if (!m_userName.has_value())
        {
            m_userName = jobVec.at(0).GetName();
        }
        for (const Job &job : jobVec)
        {
            switch (job.GetState())
            {
                case Job::State::Pending :
                    ++m_pendingCounter;
                    break;

                case Job::State::Running :
                    ++m_runningCounter;
                    break;
                    
                case Job::State::Completed :
                    ++m_finishedCounter;
                    sumUsedMem += job.GetUsedMem()*m_toGiga;
                    sumRunTime += job.GetElapsedTime();
                    break;

                case Job::State::Completing :
                    ++m_completingCounter;
                    break;
                    
                case Job::State::Failed :
                    ++m_failedCounter;
                    break;

                case Job::State::Preempted :
                    ++m_preemptedCounter;
                    break;

                case Job::State::Suspended :
                    ++m_suspendedCounter;
                    break;

                case Job::State::Stopped :
                    ++m_stoppedCounter;
                    break;
            }
        }

        return std::make_tuple(sumRunTime/m_finishedCounter,sumUsedMem/m_finishedCounter);
    }

} // namespace SJM
