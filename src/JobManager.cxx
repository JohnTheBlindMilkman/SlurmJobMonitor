#include "JobManager.hxx"

namespace SJM
{
    JobManager::JobManager(std::size_t njobs,const std::string &username,const std::vector<unsigned long> &jobIds) noexcept : 
    m_totalJobs(njobs), m_userName(username), m_jobIdsVector(jobIds), m_jobCollection({}), m_averageRunTime(std::chrono::seconds(0)),
    m_remainingTime(std::chrono::seconds(0)), m_eta(std::chrono::system_clock::now()), m_numberOfJobs(0), m_finishedCounter(0), m_runningCounter(0), 
    m_pendingCounter(0), m_failedCounter(0), m_requeueCounter(0), m_resizeCounter(0), m_suspendedCounter(0),
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
        auto document = m_gui.PrintStatus(
            m_jobCollection,
            {
                m_userName,
                PrintTime(m_remainingTime),
                PrintTime(m_eta),
                PrintTime(m_averageRunTime),
                m_totalJobs,
                m_finishedCounter,
                m_runningCounter,
                m_averagePastMemUsed,
                m_jobCollection.at(0).GetRequestedMem()/1000,
                m_hasJobsWithFinishedState
            }
        );
        auto screen = ftxui::Screen::Create(ftxui::Dimension::Full(),ftxui::Dimension::Fit(document));
        ftxui::Render(screen, document);
        std::cout << m_resetPos;
        screen.Print();
        m_resetPos = screen.ResetPosition();
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
        m_failedCounter = 0;
        m_suspendedCounter = 0;
        m_requeueCounter = 0;
        m_resizeCounter = 0;
        double sumUsedMem = 0;
        std::chrono::seconds sumRunTime(0);

        if (m_userName.empty())
        {
            m_userName = jobVec.at(0).GetName();
        }
        for (const Job &job : jobVec)
        {
            switch (job.GetState())
            {
                case Job::State::Requeued :
                    ++m_requeueCounter;
                    break;

                case Job::State::Resizing :
                    ++m_resizeCounter;
                    break;

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
                    
                case Job::State::Failed :
                    ++m_failedCounter;
                    break;

                case Job::State::NodeFail :
                    ++m_failedCounter;
                    break;

                case Job::State::OutOfMemory :
                    ++m_failedCounter;
                    break;

                case Job::State::Revoked :
                    ++m_failedCounter;
                    break;

                case Job::State::Preempted :
                    ++m_failedCounter;
                    break;

                case Job::State::Timeout :
                    ++m_failedCounter;
                    break;

                case Job::State::Deadline :
                    ++m_failedCounter;
                    break;

                case Job::State::Cancelled :
                    ++m_failedCounter;
                    break;

                case Job::State::BootFail :
                    ++m_failedCounter;
                    break;

                case Job::State::Suspended :
                    ++m_suspendedCounter;
                    break;
            }
        }

        m_remainingTime = (m_finishedCounter > 0) ? std::chrono::duration_cast<std::chrono::seconds>(std::ceil((m_totalJobs - m_finishedCounter)/m_runningCounter) * (sumRunTime/m_finishedCounter)) : std::chrono::seconds(0);
        m_eta = std::chrono::system_clock::now() + m_remainingTime;
        
        return std::make_tuple(sumRunTime/m_finishedCounter,sumUsedMem/m_finishedCounter);
    }

    std::string JobManager::PrintTime(std::chrono::seconds time) const
    {
        std::stringstream ss;
        
        using namespace std::chrono;
        using days = std::chrono::duration<int, std::ratio<86400>>;

        auto d = duration_cast<days>(time);
        time -= d;
        auto h = duration_cast<hours>(time);
        time -= h;
        auto m = duration_cast<minutes>(time);
        time -= m;
        auto s = duration_cast<seconds>(time);

        auto dc = d.count();
        auto hc = h.count();
        auto mc = m.count();
        auto sc = s.count();

        ss.fill('0');
        if (dc) {
            ss << d.count() << "d";
        }
        if (dc || hc) {
            if (dc) { ss << ":" << std::setw(2); } //pad if second set of numbers
            ss << h.count() << "h";
        }
        if (dc || hc || mc) {
            if (dc || hc) { ss << ":" << std::setw(2); }
            ss << m.count() << "m";
        }
        if (dc || hc || mc || sc) {
            if (dc || hc || mc) { ss << ":" << std::setw(2); }
            ss << s.count() << 's';
        }

        return ss.str();
    }

    std::string JobManager::PrintTime(std::chrono::system_clock::time_point time) const
    {
        std::stringstream ss;

        std::time_t timePoint = std::chrono::system_clock::to_time_t(time);
        std::tm tm = *std::localtime(&timePoint);
        ss << std::put_time(&tm,"%T %F %Z");

        return ss.str();
    }

} // namespace SJM
