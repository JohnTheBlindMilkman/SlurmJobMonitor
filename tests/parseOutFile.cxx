#include "Job.hxx"

#include <thread>

int main(int argc, char *argv[])
{
    SJM::Job j(argv[1]);
    bool isError =  j.Evaluate();
    //std::this_thread::sleep_for(std::chrono::seconds(5));
    //j.Evaluate();

    std::cout << "[Test:parseOutFile] State is " << j.PrintState() << " , and expected was Finished\n";
    if (isError)
        std::cout << "j.Evaluate() returned error state\n";
    std::cout << std::endl;

    if (j.State() != SJM::Job::AnalysisState::Started)
        return 1;
    else
        return 0;
}