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
    argparse::ArgumentParser parser("monitor",std::string(SJM::Config::projectVersion));

    parser.add_argument("refresh").help("Time (in seconds) of the refresh period").required().scan<'i',unsigned>();
    parser.add_argument("--user","-u").help("Username for whom the jobs should be displayed. Default is the callee.");
    parser.add_argument("--jobs","-j").help("List of jobs you want to be monitored. Default is all jobs started since 00:00:00 of the current day.").nargs(1,10).scan<'i',unsigned long>();

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

    std::optional<std::string> name;
    parser.is_used("--user") ? name = parser.get<std::string>("--user") : name = std::nullopt;

    std::optional<std::vector<unsigned long> > jobs;
    parser.is_used("--jobs") ? jobs = parser.get<std::vector<unsigned long> >("--jobs") : jobs = std::nullopt;

    SJM::JobManager jm(name, jobs);

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
            
        /* auto document = jm.PrintStatus(parser.get<bool>("--minimal"),parser.get<bool>("--full"));
        auto screen = ftxui::Screen::Create(ftxui::Dimension::Full(),ftxui::Dimension::Fit(document));
        Render(screen, document);
        std::cout << resetPos;
        screen.Print();
        resetPos = screen.ResetPosition(); */

        std::this_thread::sleep_for(std::chrono::seconds(parser.get<unsigned>("refresh")));
    }

    std::cout << std::endl;
}