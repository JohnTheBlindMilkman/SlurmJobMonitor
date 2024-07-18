#include "Job.hxx"

namespace SJM
{
    void from_json(const nlohmann::json &j,JobStruct &job)
    {
        job.currentState = j["state"]["current"].get<std::vector<std::string> >().at(0);
        job.elapsedTime = j["time"]["elapsed"].get<long unsigned>();
        job.endTime = j["time"]["end"].get<long unsigned>();
        job.startTime = j["time"]["start"].get<long unsigned>();
        job.maxTime = j["time"]["eligible"].get<long unsigned>();
        job.submissionTime = j["time"]["submission"].get<long unsigned>();
        job.name = j["association"]["user"].get<std::string>();
        job.exitCodeStatus = j["exit_code"]["status"].get<std::vector<std::string> >().at(0);
        job.flags = j["flags"].get<std::vector<std::string> >();
        job.jobId = j["array"]["job_id"].get<long unsigned>();
        job.maxMemory = j["required"]["memory_per_node"]["number"].get<long unsigned>();
        job.node = j["nodes"].get<std::string>();
        job.partition = j["partition"].get<std::string>();
        job.priority = j["priority"]["number"].get<long unsigned>();
        job.stateReason = j["state"]["reason"].get<std::string>();
        job.taskId = j["array"]["task_id"]["number"].get<long unsigned>();
        if (job.currentState == "COMPLETED")
            job.usedMemory = j["steps"].at(0)["tres"]["requested"]["average"].at(1)["count"].get<long unsigned>();
        else
            job.usedMemory = 0;
    }

    void from_json(const nlohmann::json &j, JobArrayStruct &job)
    {
        job.nTasks = j["array"]["task"].get<std::string>();
    }

    Job::Job(const JobStruct &j) : 
    m_stateMap(
        {{"REQUEUED",State::Requeued},
        {"RESIZING",State::Resizing},
        {"PENDING",State::Pending},
        {"RUNNING",State::Running},
        {"COMPLETED",State::Completed},
        {"FAILED",State::Failed},
        {"NODE_FAIL",State::NodeFail},
        {"OUT_OF_MEMORY",State::OutOfMemory},
        {"REVOKED",State::Revoked},
        {"PREEMPTED",State::Preempted},
        {"SUSPENDED",State::Suspended},
        {"TIMEOUT",State::Timeout},
        {"DEADLINE",State::Deadline},
        {"CANCELLED",State::Cancelled},
        {"BOOT_FAIL",State::BootFail}}
    ),
    m_partitionMap(
        {{"main",Partition::Main},
        {"long",Partition::Long},
        {"grid",Partition::Grid},
        {"high_mem",Partition::HighMem},
        {"gpu",Partition::Gpu},
        {"debug",Partition::Debug}}
    )
    {
        m_currentState = m_stateMap.at(j.currentState);
        m_partition = m_partitionMap.at(j.partition);
        m_exitCodeStatus = j.exitCodeStatus;
        m_node = j.node;
        m_stateReason = j.stateReason;
        m_jobId = j.jobId;
        m_taskId = j.taskId;
        m_priority = j.priority;
        m_usedMemory = j.usedMemory;
        m_maxMemory = j.maxMemory;
        m_elapsedTime = MakeDuration(j.elapsedTime);
        m_maxTime = MakeDuration(j.maxTime);
        m_startTime = MakeTimePoint(j.startTime);
        m_endTime = MakeTimePoint(j.endTime);
        m_submissionTime = MakeTimePoint(j.submissionTime);
        m_flags = j.flags;
    }

} // namespace SJM
