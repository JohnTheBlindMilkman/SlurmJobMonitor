#include "Job.hxx"

int main(int argc, char *argv[])
{
    SJM::Job j(argv[1]);

    auto expectedTime = std::chrono::seconds(4206);
    auto actualTime = j.GetRunTime();

    std::cout << "[Test:parseLogFile] Run time is " << actualTime.count() << ", and expected was " << expectedTime.count() << "\n";
    std::cout << "State is " << j.PrintState() << " , and expected was Finished\n";
    std::cout << std::endl;

    if (actualTime != expectedTime || j.State() != SJM::Job::AnalysisState::Finished)
        return 1;
    else
        return 0;
}