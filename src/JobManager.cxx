#include "JobManager.hxx"

namespace SJM
{
    void from_json(const nlohmann::json &j,Job &job)
    {
        job.currentState = j["state"]["current"].get<std::vector<std::string> >().at(0);
        job.elapsedTime = j["time"]["elapsed"].get<long unsigned>();
        job.endTime = j["time"]["end"].get<long unsigned>();
        job.startTime = j["time"]["start"].get<long unsigned>();
        job.maxTime = j["time"]["eligible"].get<long unsigned>();
        job.submissionTime = j["time"]["eligible"].get<long unsigned>();
        job.exitCodeStatus = j["exit_code"]["status"].get<std::vector<std::string> >().at(0);
        job.flags = j["flags"].get<std::vector<std::string> >();
        job.jobId = j["array"]["job_id"].get<long unsigned>();
        job.maxMemory = j["required"]["memory_per_node"]["number"].get<long unsigned>();
        job.node = j["nodes"].get<std::string>();
        job.partition = j["partition"].get<std::string>();
        job.priority = j["priority"]["number"].get<long unsigned>();
        job.qos = j["qos"].get<std::string>();
        job.stateReason = j["state"]["reason"].get<std::string>();
        job.taskId = j["array"]["task_id"]["number"].get<long unsigned>();
        if (job.currentState == "COMPLETED")
            job.usedMemory = j["steps"].at(0)["tres"]["requested"]["average"].at(1)["count"].get<long unsigned>();
    }


    JobManager::JobManager(const std::optional<std::string> &username,const std::optional<std::vector<unsigned long> > &jobIds) noexcept : 
    m_userName(username), 
    m_jobIdsVector(jobIds),
    m_stateMap(
        {{State::Pending,"PENDING"},
        {State::Running,"RUNNING"},
        {State::Completed,"COMPLETED"},
        {State::Failed,"FAILED"},
        {State::Timeout,"N/A"},
        {State::Resizing,"N/A"},
        {State::Deadline,"N/A"},
        {State::NodeFail,"N/A"},}
    )
    {      
    }

    bool JobManager::UpdateJobs()
    {
        std::cout << ExecuteCommand(m_userName,m_jobIdsVector) << "\n";
        m_jobCollection = FromJsonToJobVector(ReadJson(m_pathToJson));

        // this is not optimal, but I want things to be more explicit, and not being initialised around in the background
        m_numberOfJobs = m_jobCollection.size();
        m_pendingCounter = CountJobsByState(m_jobCollection,State::Pending);
        m_runningCounter = CountJobsByState(m_jobCollection,State::Running);
        m_finishedCounter = CountJobsByState(m_jobCollection,State::Completed);
        m_failedCounter = CountJobsByState(m_jobCollection,State::Failed);
        m_timeoutCounter = CountJobsByState(m_jobCollection,State::Timeout);
        m_resizingCounter = CountJobsByState(m_jobCollection,State::Resizing);
        m_deadlineCounter = CountJobsByState(m_jobCollection,State::Deadline);
        m_nodeFailCounter = CountJobsByState(m_jobCollection,State::NodeFail);

        (m_finishedCounter > 0) ? m_hasJobsWithFinishedState = true : m_hasJobsWithFinishedState = false;

        std::cout << "Pending: " << m_pendingCounter << "\n";
        std::cout << "Running: " << m_runningCounter << "\n";
        std::cout << "Finished: " << m_finishedCounter << "\n";
        std::cout <<std::boolalpha << "Some jobs have finished: " << m_hasJobsWithFinishedState << "\n";

        return ((m_runningCounter + m_pendingCounter) > 0) ? true : false;
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
        Job jobStruct;
        for (const auto &job : j["jobs"]) // I should check somewhere if I get any jobs at all
        {
            jobStruct = job.get<Job>();
            if (jobStruct.taskId != 0)
                jobVec.push_back(jobStruct);
        }

        return jobVec;
    }

    std::size_t JobManager::CountJobsByState(const std::vector<Job> &vec, State state) const
    {
        std::string strState = m_stateMap.at(state);
        return std::count_if(vec.begin(),vec.end(),[strState](const Job &j){return strState == j.currentState;});
    }

    /* ftxui::Color JobManager::GetColorByStatus(const Job::AnalysisState &state) const
    {
        switch (state)
        {
            case Job::AnalysisState::NotStarted :
                return ftxui::Color::GrayDark;

            case Job::AnalysisState::Started :
                return ftxui::Color::Yellow;
                
            case Job::AnalysisState::Finished :
                return ftxui::Color::Green;
                
            case Job::AnalysisState::Error :
                return ftxui::Color::Red;
            default:
                return ftxui::Color::Default;
        }
    }

    ftxui::Elements JobManager::CreateStatusBox()
    {
        ftxui::Elements elems;
        for (const auto &jobState : CreateJobStateVector())
        {
            elems.push_back(ftxui::text("   ") | ftxui::bgcolor(GetColorByStatus(jobState)));
        }

        return elems;
    } */
    

    /* ftxui::Element JobManager::PrintStatus(bool minimal, bool full)
    {
        ftxui::Elements contents,errors;
        const std::time_t ETAtime = std::chrono::system_clock::to_time_t(ETA);

        contents.push_back(ftxui::vbox(
                    ftxui::hbox(
                        ftxui::text("Total number of jobs: " + std::to_string(totalJobs)),
                        ftxui::filler(),
                        ftxui::text("Predicted remaining time: " + MakeTime(remainingTime).str())
                    ),
                    ftxui::hbox(
                        ftxui::text("Finished jobs: " + std::to_string(finishedCounter)),
                        ftxui::filler(),
                        ftxui::text("Predicted ETA: " + std::string(std::ctime(&ETAtime))) 
                    ),
                    ftxui::hbox(
                        ftxui::text("Currently running: " + std::to_string(runningCounter)),
                        ftxui::filler(),
                        ftxui::text("Previous average job runtime: " + MakeTime(avgFinishTime).str()) 
                    )
                ) | ftxui::border);

        errors = std::move(CreateJobErrorMsgVector());
        if (errors.size() > 0)
        {
            contents.push_back(ftxui::vbox(
                ftxui::text("Errors") | ftxui::center | ftxui::color(ftxui::Color::Orange1),
                ftxui::separator(),
                std::move(errors)) | ftxui::border);
        }

        if (full)
        {
            contents.push_back(ftxui::vbox(
                    ftxui::text("Currenlty running jobs") | ftxui::center,
                    ftxui::separator(),
                    ftxui::hflow(std::move(CreateTable()))) | ftxui::border
                );
        }
        else if (!minimal)
        {
            contents.push_back(
                ftxui::vbox(
                ftxui::hbox(
                    ftxui::text("Finished = "),
                    ftxui::text("   ") | ftxui::bgcolor(ftxui::Color::Green),
                    ftxui::text("   Running = "),
                    ftxui::text("   ") | ftxui::bgcolor(ftxui::Color::Yellow),
                    ftxui::text("   Pending = "),
                    ftxui::text("   ") | ftxui::bgcolor(ftxui::Color::GrayDark),
                    ftxui::text("   Error = "),
                    ftxui::text("   ") | ftxui::bgcolor(ftxui::Color::Red)) | ftxui::center,
                ftxui::separator(),
                ftxui::hflow(std::move(CreateStatusBox()))) | ftxui::border);
        }
        return ftxui::vbox(std::move(contents));
    } */

} // namespace SJM
