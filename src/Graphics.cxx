#include "Graphics.hxx"

namespace SJM
{
    ftxui::Element Graphics::PrintStatus(const std::vector<Job> &jobVec, const GraphicsDisplayInfo &info) const
    {
        ftxui::Elements contents;

        contents.push_back(ftxui::hbox(
            RenderMemUsage(info.usedMem,info.reqMem),
            RenderBatchInfo(info.finishedJobs,info.runningJobs,info.nJobs,info.name) | ftxui::flex
        ));
        contents.push_back(RenderProgressBar(info.finishedJobs,info.nJobs));
        contents.push_back(RenderStatusBlock(jobVec,info.nJobs));

        return ftxui::vbox(std::move(contents));
    }

    ftxui::Element Graphics::RenderStatusBlock(const std::vector<Job> &jobVec, std::size_t njobs) const
    {
        ftxui::Elements list;
        unsigned counter = 0;

        for (const Job &j : jobVec)
        {
            ++counter;
            list.push_back(ftxui::text("   ") | ftxui::bgcolor(GetColorByStatus(j.GetState())));
        }
        while (counter < njobs)
        {
            ++counter;
            list.push_back(ftxui::text("   ") | ftxui::bgcolor(ftxui::Color::GrayDark));
        }
        

        return ftxui::vbox(
                ftxui::hbox(
                    ftxui::text("Completed = "),
                    ftxui::text("   ") | ftxui::bgcolor(ftxui::Color::Green),
                    ftxui::text("   Running = "),
                    ftxui::text("   ") | ftxui::bgcolor(ftxui::Color::Yellow),
                    ftxui::text("   Pending = "),
                    ftxui::text("   ") | ftxui::bgcolor(ftxui::Color::GrayDark),
                    ftxui::text("   Error = "),
                    ftxui::text("   ") | ftxui::bgcolor(ftxui::Color::Red)
                ) | ftxui::center,
                ftxui::hbox(
                    ftxui::text("Completing = "),
                    ftxui::text("   ") | ftxui::bgcolor(ftxui::Color::GreenLight),
                    ftxui::text("   Preempted = "),
                    ftxui::text("   ") | ftxui::bgcolor(ftxui::Color::RedLight),
                    ftxui::text("   Suspended = "),
                    ftxui::text("   ") | ftxui::bgcolor(ftxui::Color::White),
                    ftxui::text("   Stoppped = "),
                    ftxui::text("   ") | ftxui::bgcolor(ftxui::Color::Magenta)
                ) | ftxui::center,
                ftxui::separator(),
                ftxui::hflow(std::move(list))) | ftxui::border;
    }

    ftxui::Element Graphics::RenderProgressBar(std::size_t finished, std::size_t njobs) const
    {
        double percentage = static_cast<double>(finished) / static_cast<double>(njobs);
        return ftxui::hbox(
            ftxui::text("Total Progress: "),
            ftxui::gauge(percentage) | ftxui::flex,
            ftxui::text(" " + std::to_string(finished) + "/" + std::to_string(njobs))
        ) | ftxui::border;
    }

    ftxui::Element Graphics::RenderMemUsage(unsigned avgUsed, unsigned requested) const
    {
        float prct = static_cast<float>(avgUsed) / static_cast<float>(requested);

        return ftxui::vbox(
                ftxui::text("Average Memory Usage") | ftxui::center,
                ftxui::hbox(
                    ftxui::vbox(
                        ftxui::text("100%") | ftxui::align_right,
                        ftxui::text("    ") | ftxui::align_right,
                        ftxui::text("80%") | ftxui::align_right,
                        ftxui::text("    ") | ftxui::align_right,
                        ftxui::text("60%") | ftxui::align_right,
                        ftxui::text("    ") | ftxui::align_right,
                        ftxui::text("40%") | ftxui::align_right,
                        ftxui::text("    ") | ftxui::align_right,
                        ftxui::text("20%") | ftxui::align_right,
                        ftxui::text("    ") | ftxui::align_right,
                        ftxui::text("0%") | ftxui::align_right
                    ),
                    ftxui::separator(),
                    ftxui::hbox(
                        ftxui::gaugeUp(prct) | ftxui::color(ftxui::Color::Yellow),
                        ftxui::gaugeUp(prct) | ftxui::color(ftxui::Color::Yellow),
                        ftxui::gaugeUp(prct) | ftxui::color(ftxui::Color::Yellow),
                        ftxui::gaugeUp(prct) | ftxui::color(ftxui::Color::Yellow),
                        ftxui::gaugeUp(prct) | ftxui::color(ftxui::Color::Yellow),
                        ftxui::gaugeUp(prct) | ftxui::color(ftxui::Color::Yellow),
                        ftxui::gaugeUp(prct) | ftxui::color(ftxui::Color::Yellow),
                        ftxui::gaugeUp(prct) | ftxui::color(ftxui::Color::Yellow),
                        ftxui::gaugeUp(prct) | ftxui::color(ftxui::Color::Yellow),
                        ftxui::gaugeUp(prct) | ftxui::color(ftxui::Color::Yellow),
                        ftxui::gaugeUp(prct) | ftxui::color(ftxui::Color::Yellow),
                        ftxui::gaugeUp(prct) | ftxui::color(ftxui::Color::Yellow),
                        ftxui::gaugeUp(prct) | ftxui::color(ftxui::Color::Yellow),
                        ftxui::gaugeUp(prct) | ftxui::color(ftxui::Color::Yellow),
                        ftxui::gaugeUp(prct) | ftxui::color(ftxui::Color::Yellow)
                    )
                ),
                ftxui::separator(),
                ftxui::text("Requested: " + std::to_string(requested) + " MB") | ftxui::center
            ) | ftxui::border;
    }

    ftxui::Element Graphics::RenderBatchInfo(std::size_t finished, std::size_t running, std::size_t njobs, std::string user) const
    {
        return ftxui::vbox(
            ftxui::text("Job summary for user " + user) | ftxui::center,
            ftxui::separator(),
            ftxui::hbox(
                ftxui::text("Total number of jobs: " + std::to_string(njobs)),
                ftxui::filler(),
                ftxui::text("Predicted remaining time: PLACEHOLDER")
            ),
            ftxui::hbox(
                ftxui::text("Finished jobs: " + std::to_string(finished)),
                ftxui::filler(),
                ftxui::text("Predicted ETA: PLACEHOLDER") 
            ),
            ftxui::hbox(
                ftxui::text("Currently running: " + std::to_string(running)),
                ftxui::filler(),
                ftxui::text("Previous average job runtime: PLACEHOLDER") 
            )
        ) | ftxui::border;
    }

    ftxui::Color Graphics::GetColorByStatus(const Job::State state) const
    {
        switch (state)
        {
            case Job::State::Pending :
                return ftxui::Color::GrayDark;

            case Job::State::Running :
                return ftxui::Color::Yellow;
                
            case Job::State::Completed :
                return ftxui::Color::Green;

            case Job::State::Completing :
                return ftxui::Color::GreenLight;
                
            case Job::State::Failed :
                return ftxui::Color::Red;

            case Job::State::Preempted :
                return ftxui::Color::RedLight;

            case Job::State::Suspended :
                return ftxui::Color::White;

            case Job::State::Stopped :
                return ftxui::Color::Magenta;

            default:
                return ftxui::Color::Default;
        }
    }

} // namespace SJM