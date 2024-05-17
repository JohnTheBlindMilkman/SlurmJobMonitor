/**
 * @file GlobalConstants.hxx
 * @author Jędrzej Kołaś (jedrzej.kolas.dokt@pw.edu.pl)
 * @brief 
 * @version 1.0
 * @date 2024-05-13
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#ifndef GlobalConstants_hxx
    #define GlobalConstants_hxx

    #include <string_view>

    namespace SJM
    {
        namespace GlobalConstants
        {
            /**
             * @brief Maximal value of the percentage available in .out files
             * 
             */
            constexpr int maxPercentage{99};
            /**
             * @brief Helper constant for extracting the percentage value from a file
             * 
             */
            constexpr int substrOffset{3};
            /**
             * @brief Helper constant for extracting the percentage value from a file
             * 
             */
            constexpr int substrLength{3};
            /**
             * @brief File extension of the SLURM files where the analysis is progressing
             * 
             */
            constexpr std::string_view fileExtension{".out"};
            /**
             * @brief Helper constant for finding the beginning of the analysis process
             * 
             */
            constexpr std::string_view beginingExpression{"at entry"};
            /**
             * @brief Helper constant for finding the end of the analysis process
             * 
             */
            constexpr std::string_view endingExpression{"Finished DST processing"};
            /**
             * @brief Helper constant for extracting the percentage value from a file
             * 
             */
            constexpr std::string_view percentExpression{" %"};
            
        } // namespace GlobalConstants
    } // namespace SJM
    

#endif