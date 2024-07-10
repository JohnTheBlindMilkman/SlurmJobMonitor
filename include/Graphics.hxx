/**
 * @file Graphics.hxx
 * @author Jędrzej Kołaś (jedrzej.kolas.dokt@pw.edu.pl)
 * @brief Class for creating the terminal gui using FTXUI library.
 * @version 2.0.0
 * @date 2024-07-08
 * 
 * @copyright Copyright (c) 2024
 * 
 */
#ifndef Graphics_hxx
    #define Graphics_hxx

    #include "ftxui/dom/elements.hpp"
    #include "ftxui/screen/screen.hpp"
    #include "ftxui/dom/table.hpp"

    #include "Job.hxx"

    namespace SJM
    {
        struct GraphicsDisplayInfo
        {
            std::string name,remainigTime,ETA,avgPastRuntime;
            std::size_t nJobs,finishedJobs,runningJobs;
            double usedMem;
            unsigned long reqMem;
            bool hasFinishedJobs;
        };
        

        class Graphics
        {
            public:
                /**
                 * @brief Class constructor
                 * 
                 */
                Graphics(/* args */) = default;
                /**
                 * @brief Return the terminal gui document for given job vector
                 * 
                 * @param jobVec collection of jobs obtained by JobManager
                 * @return ftxui::Element document
                 */
                [[nodiscard]] ftxui::Element PrintStatus(const std::vector<Job> &jobVec, const GraphicsDisplayInfo &info) const;

            private:
                /**
                 * @brief Create colored status block for each job 
                 * 
                 * @param jobVec collection of jobs obtained by JobManager
                 * @param njobs total amout of jobs
                 * @return ftxui::Element 
                 */
                [[nodiscard]] ftxui::Element RenderStatusBlock(const std::vector<Job> &jobVec, std::size_t njobs) const;
                /**
                 * @brief Create progress bar of the whole batch
                 * 
                 * @param finished number of finished of jobs
                 * @param njobs total amout of jobs
                 * @return ftxui::Element 
                 */
                [[nodiscard]] ftxui::Element RenderProgressBar(std::size_t finished, std::size_t njobs) const;
                /**
                 * @brief Create memory usage bar of finished jobs
                 * 
                 * @param jobVec collection of jobs obtained by JobManager
                 * @return ftxui::Element 
                 */
                [[nodiscard]] ftxui::Element RenderMemUsage(unsigned avgUsed, unsigned requested) const;
                /**
                 * @brief Create info bar with basic information about the batch jobs & ETA
                 * 
                 * @param finished 
                 * @param running 
                 * @param njobs 
                 * @param user 
                 * @return ftxui::Element 
                 */
                [[nodiscard]] ftxui::Element RenderBatchInfo(std::size_t finished, std::size_t running, std::size_t njobs, std::string user, std::string remTime, std::string eta, std::string avgRun) const;
                [[nodiscard]] std::pair<std::string,ftxui::Color> GetColorByStatus(const Job::State state) const;
        };

    } // namespace SJM
    

#endif