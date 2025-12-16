#include "JobManager.hxx"

namespace SJM
{
    JobManager::JobManager(const std::string &username,const std::vector<unsigned long> &jobIds) noexcept : 
    m_totalJobs(0), m_userName(username), m_jobIdsVector(jobIds), m_jobCollection({}), m_averageRunTime(std::chrono::seconds(0)),
    m_remainingTime(std::chrono::seconds(0)), m_eta(std::chrono::system_clock::now()), m_numberOfJobs(0), m_finishedCounter(0), m_runningCounter(0), 
    m_pendingCounter(0), m_failedCounter(0), m_requeueCounter(0), m_resizeCounter(0), m_suspendedCounter(0),
    m_totalMemAssigned(0.), m_predictedTotalMemUsed(0.), m_averagePastMemUsed(0.), m_hasJobsWithFinishedState(false), m_gui(),
    m_hexTrueCounter(
        {{'0',0},
        {'1',1},
        {'2',1},
        {'3',2},
        {'4',1},
        {'5',2},
        {'6',2},
        {'7',3},
        {'8',1},
        {'9',2},
        {'A',2},
        {'B',3},
        {'C',2},
        {'D',3},
        {'E',3},
        {'F',4}}
    )
    {
    }

    bool JobManager::UpdateJobs()
    {
        ExecuteCommand(m_userName,m_jobIdsVector);
        std::tie(m_jobCollection,m_pendingCounter) = FromJsonToJobVector(ReadJson(m_pathToJson));
        std::tie(m_averageRunTime,m_totalMemAssigned,m_predictedTotalMemUsed) = PopulateVariables(m_jobCollection);

        /* std::cout << m_totalJobs << "\n";
        std::cout << m_runningCounter + m_finishedCounter << "\n";
        if (m_totalJobs < m_runningCounter + m_finishedCounter)
            throw std::runtime_error("njobs is smaller than running jobs + finished jobs"); */

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
                m_predictedTotalMemUsed,
                m_totalMemAssigned,
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

    std::string JobManager::ExecuteCommand(const std::string &username,const std::vector<unsigned long> &jobIds)
    {
        std::string userFlag = (username != "") ? "-u " + username + " " : "";
        std::string jobidFlag = (jobIds.size() > 0) ? "-j " + ParseVector(jobIds) : ""; // Comment from "man sacct": -S: Select jobs eligible after this time. Default is 00:00:00 of the current day

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

    std::tuple<std::vector<Job>,std::size_t> JobManager::FromJsonToJobVector(const nlohmann::json &j)
    {
        std::vector<Job> jobVec;
        std::size_t njobs = 0;
        JobStruct jobStruct;
        for (const auto &job : j["jobs"]) // I should check somewhere if I get any jobs at all
        {
            jobStruct = job.get<JobStruct>();
            if (jobStruct.taskId != 0)
            {
                jobVec.push_back(Job(jobStruct));
            }
            else
            {
                njobs += ConvertBatchHash(job.get<JobArrayStruct>().nTasks);
            }
        }

        return std::make_tuple(jobVec,njobs);
    }

    unsigned JobManager::ConvertBatchHash(const std::string &str) const
    {
        unsigned counter = 0;
        for (const auto &letter : str.substr(2)) // hex begins with "0x" and I have to reject it
            counter += m_hexTrueCounter.at(letter);

        return counter;
    }

    std::size_t JobManager::CountJobsByState(const std::vector<Job> &vec, Job::State state) const
    {
        return std::count_if(vec.begin(),vec.end(),[&state](const Job &j){return state == j.GetState();});
    }

    std::tuple<std::chrono::seconds,long unsigned,long unsigned> JobManager::PopulateVariables(const std::vector<Job> &jobVec)
    {
        m_numberOfJobs = jobVec.size();
        m_totalJobs = 0;
        m_finishedCounter = 0;
        m_runningCounter = 0;
        m_failedCounter = 0;
        m_suspendedCounter = 0;
        m_requeueCounter = 0;
        m_resizeCounter = 0;
        long unsigned sumReqMem = 0, predictedUsedMem = 0;
        double sumUsedMem = 0, avgUsedMem = 0;
        std::chrono::seconds sumRunTime(0),avgRunTime(0);
        std::chrono::system_clock::time_point minStartTime(std::chrono::system_clock::now());

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
                    //++m_pendingCounter; // do nithing, I already managed this
                    break;

                case Job::State::Running :
                    ++m_runningCounter;
                    sumReqMem += job.GetRequestedMem()/1000;
                    break;
                    
                case Job::State::Completed :
                    ++m_finishedCounter;
                    sumUsedMem += job.GetUsedMem()*m_toGiga;
                    sumRunTime += job.GetElapsedTime();
                    minStartTime = std::min(minStartTime,job.GetStartTime());
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

       m_totalJobs = m_pendingCounter + m_runningCounter + m_finishedCounter;

        m_remainingTime = (m_finishedCounter > 0 && m_runningCounter > 0) ? std::chrono::duration_cast<std::chrono::seconds>(std::ceil((m_totalJobs - m_finishedCounter)/m_runningCounter) * (sumRunTime/m_finishedCounter)) : std::chrono::seconds(0);
        m_eta = minStartTime + m_remainingTime;
        
        if (m_finishedCounter > 0)
        {
            avgUsedMem = sumUsedMem/m_finishedCounter;
            avgRunTime = sumRunTime/m_finishedCounter;
            predictedUsedMem = avgUsedMem * m_runningCounter;
        }

        return std::make_tuple(avgRunTime,sumReqMem,predictedUsedMem);
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
