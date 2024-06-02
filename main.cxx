#include "argparse/argparse.hpp"

#include "JobManager.hxx"
#include "Config.hxx"

#include "signal.h"
#include <thread>
#include <chrono>

void terminateProgram(int signum)
{
    (void)signum; // does nothing; I put this to supress the warning about signum being unused
    std::cout << "\nRecieved interrupt - stopping execution\n" << std::endl;
    std::exit(1);
}

int main(int argc, char *argv[])
{    
    argparse::ArgumentParser parser("monitor",std::string(SJM::Config::projectVersion));

    parser.add_argument("path").help("Path to SLURM output direcotry").required();
    parser.add_argument("njobs").help("Number of jobs that were submitted to SLURM").required().scan<'i',unsigned>();
    parser.add_argument("refresh").help("Time (in seconds) of the refresh frequency").required().scan<'i',unsigned>();
    auto &group = parser.add_mutually_exclusive_group();
    group.add_argument("-m","--minimal").help("Print minimal amount of information").flag();
    group.add_argument("-f","--full").help("Print all available information").flag();

    parser.add_description("Slurm Job Monitor (" + std::string(SJM::Config::projectVersion) + ") - Monitoring script for standard HADES DST macro execution on SLURM batchfatm at GSI");

    try 
    {
        parser.parse_args(argc, argv);
    }
    catch (const std::exception& err) 
    {
        std::cerr << err.what() << std::endl;
        std::cerr << parser;
        std::exit(1);
    }

    std::string resetPos;
    SJM::JobManager jm(parser.get<std::string>("path"),parser.get<unsigned>("njobs"));

    while (true)
    {
        if (!jm.UpdateJobs())
        {
            std::cout << "\nAll the jobs have finished\n";
            break;
        }
        else if (signal(SIGINT,terminateProgram) == SIG_IGN)
        {
            signal(SIGINT,SIG_IGN);
        }
            
        auto document = jm.PrintStatus(parser.get<bool>("--minimal"),parser.get<bool>("--full"));
        auto screen = ftxui::Screen::Create(ftxui::Dimension::Full(),ftxui::Dimension::Fit(document));
        Render(screen, document);
        std::cout << resetPos;
        screen.Print();
        resetPos = screen.ResetPosition();

        std::this_thread::sleep_for(std::chrono::seconds(parser.get<unsigned>("refresh")));
    }

    std::cout << std::endl;
}