if(SJM_ENABLE_TESTS)

    add_executable(parseLogFile parseLogFile.cxx)
    target_link_libraries(parseLogFile PRIVATE base)
    target_compile_features(parseLogFile  PRIVATE cxx_std_20)

    add_executable(parseOutFile parseOutFile.cxx)
    target_link_libraries(parseOutFile PRIVATE base)
    target_compile_features(parseOutFile  PRIVATE cxx_std_20)

    add_test(NAME parseLogFile COMMAND parseLogFile "${CMAKE_SOURCE_DIR}/tests/apr12ana_all_11.log")
    add_test(NAME parseOutFile COMMAND parseOutFile "${CMAKE_SOURCE_DIR}/tests/slurm12345_101.out")

endif()