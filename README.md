# Slurm Job Monitor

Slurm Job Monitor (SJM) is a script-like program which sole purpose is to monitor jobs run on the SLURM scheduler at GSI. Currently the program only supports monitoring standard DST analysis at HADES. If you wish to expand this functionality submit a pull request (this will need a lot of restructurisaition of the code, just be warned).

## Dependencies

This program utilises the following:

- [Argument Parser for Modern C++ ](https://github.com/p-ranav/argparse) by p-ranav
- [FTXUI](https://github.com/ArthurSonzogni/FTXUI) by Arthur Sozogni

## Instalation

Clone this repository and once youre inside the directory do:
```
mkdir build
cd build
cmake ..
make
```
and pray for successful compilation.

## Usage

For the script to work it need access to the output folder where SLURM saves the log files of the finished jobs. Before that happens, i.e. while the jobs are being executed, the .log file exists as an .out file (contents are the same). These files are what the SJM looks for and where it takes the information from.

The currently only working optino is using the `sshfs` command to mount the out/ folder on your local machine. Due to the inaccesibility of virgo from the outside, it is impossible to fetch the required dependencies when building this project there.

In order to run the program execcute the binary file `./monintor` with 3 mandatory arguments:

- path to mounted out/ directory
- the total number of jobs submitted to SLURM
- refresh rate (time in seconds) of the program

> **Important note:** do not set the refresh rate too high, the program takes some time to read all the files (expecially with a lot of files). Moreover, if the refresh rate is too quick it messes up the remaing time calculation. As of the moment of speaking 30s seems to be the sweet spot.

The SJM has three modes of printing the information:

- standard mode - information about the amount of running jobs and total jobs with a graphical representation of them
- minimal mode - onlu the information about running and total jobs
- full mode - prints a table with information about all jobs separately (ID, state, remaining time)

## Limitations

1. You have to use the `sshfs`.
2. Your job has to use the standard HTool analysis percentage print.
2. Your jobs have to end with the standard "Finished DST analysis".
3. SLURM output has to be all in one direcotry.

## Final Note

Please do not overuse this script. It utilises a lot of practices whoch any IT specialist at a large batch farm would advise against.