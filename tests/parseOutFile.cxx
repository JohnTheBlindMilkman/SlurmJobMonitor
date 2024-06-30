#include "Job.hxx"

int main(int argc, char *argv[])
{
    SJM::Job j(argv[1]);
    const bool isContinue =  j.Evaluate();
    const std::string jobId = j.GetJobId();
    const std::string taskId = j.GetTaskId();
    const int prct = j.GetPercentage();

    std::cout << "[Test:parseOutFile] State is " << j.PrintState() << " and expected was Started\n";
    std::cout << "j.Evaluate() returned " << isContinue << "\n";
    std::cout << "File Job ID is " << jobId << " and expected was 12345\n";
    std::cout << "File Task ID is " << taskId << " and expected was 101\n";
    std::cout << "Current percentage is " << prct << " and expected was 60\n";
    std::cout << std::endl;

    if (j.State() != SJM::Job::AnalysisState::Started)
        return 1;
    else if (jobId != "12345")
        return 1;
    else if (taskId != "101")
        return 1;
    else if (prct != 60)
        return 1;
    else
        return 0;
}