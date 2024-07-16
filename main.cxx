/**
 * @file main.cxx
 * @author Jędrzej Kołaś (jedrzej.kolas.dokt@pw.edu.pl)
 * @brief Main file of the program
 * @version 1.2.2
 * @date 2024-06-02
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#include "argparse/argparse.hpp"

#include "JobManager.hxx"
#include "Config.hxx"

#include "signal.h"
#include <thread>
#include <chrono>
#include <optional>

/**
 * @brief Small helper function for gracefully exiting the program when Ctrl+C is pressed
 * 
 * @param signum 
 */
void terminateProgram(int signum)
{
    (void)signum; // does nothing; I put this to supress the warning about signum being unused
    std::cout << "\nRecieved interrupt - stopping execution\n" << std::endl;
    std::exit(1);
}

int main(int argc, char *argv[])
{   
    constexpr float maxMult{8.};
    constexpr float minMult{1./8.};
    constexpr std::chrono::seconds baseTime{120};
    float multiplier = 1; 
    std::chrono::seconds refreshTime = baseTime;

    argparse::ArgumentParser parser("monitor",std::string(SJM::Config::projectVersion));

    parser.add_argument("njobs").help("total number of submitted jobs").required().scan<'i',long unsigned>();
    parser.add_argument("--user","-u").help("username for whom the jobs should be displayed. Default is the callee");
    parser.add_argument("--jobs","-j").help("list of jobs you want to be monitored. Default is all jobs started since 00:00:00 of the current day").nargs(1,10).scan<'i',unsigned long>();
    auto &group = parser.add_mutually_exclusive_group();
    group.add_argument("-f","--fast")
        .action([&](const auto &){multiplier /= 2.;})
        .append()
        .default_value(false)
        .implicit_value(true)
        .nargs(0)
        .help("increase the base refresh time by a factor of two");
    group.add_argument("-s","--slow")
        .action([&](const auto &){multiplier *= 2.;})
        .append()
        .default_value(false)
        .implicit_value(true)
        .nargs(0)
        .help("decrease the base refresh time by a factor of two");

    parser.add_description("Slurm Job Monitor (" + std::string(SJM::Config::projectVersion) + 
        ") - Monitoring script for the SLURM batchfarm at GSI");

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

    SJM::JobManager jm(
        parser.get<long unsigned>("njobs"), 
        parser.is_used("--user") ? parser.get<std::string>("--user") : "", 
        parser.is_used("--jobs") ? parser.get<std::vector<unsigned long> >("--jobs") : std::vector<unsigned long>()
        );

    multiplier = std::min(multiplier,maxMult); // see if we exceeded the predefined bounds
    multiplier = std::max(multiplier,minMult);

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
            
        jm.UpdateGui();

        std::this_thread::sleep_for(baseTime*multiplier);
    }

    std::cout << std::endl;
}