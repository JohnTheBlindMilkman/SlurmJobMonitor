# Slurm Job Monitor

Slurm Job Monitor (SJM) is a TUI script-like program which sole purpose is to monitor jobs run on the SLURM scheduler at GSI.

## Dependencies

This program utilises the following:

- [Argument Parser for Modern C++ ](https://github.com/p-ranav/argparse) by p-ranav
- [FTXUI](https://github.com/ArthurSonzogni/FTXUI) by Arthur Sozogni
- [doxygen-awesome-css](https://github.com/jothepro/doxygen-awesome-css) by jothepro (only if building with doxygen option)

## Instalation

Clone this repository and once youre inside the directory do:
```
cmake -S . -B build/
cmake --build build/ --target monitor --config Release
```
and pray for successful compilation. 
- If you wish to create documentation add `-DSJM_ENABLE_DOXYGEN=ON` flag after the `-B build/` (make sure to specify building of the documentation in the `--target` flag). 
- If you wand to use a different build system, specify it in the first `cmake` command, e.g. if you want o use ninja: `cmake -S . -B build/ -G Ninja`. 
- If you want to use a debugger because something is broken or you broke something, or you want to run a profiler, change the `--config Release` flag to `Debug` to have symobls generated.

## Usage

The program utilieses the use of `sacct` command from whcich it is able to retreive information about specific jobs run by SLURM. To run this program simply type `./monitor`. This will show all jobs which are running for the user since midnight and update the status every two minutes.

Currently there are additional flags for running the program:
- `-u` or `--user` to specify for which user you want to monitor the jobs
- `-j` or `--jobs` to provide a list of up to 10 job IDs which you want to monitor
- `-s` or `--slow` to slow down the polling refresh rate by a factor of two (can be stacked up to 3 times)
- `-f` or `--fast` to speed up the polling refresh rate by a factor of two (can be stacked up to 3 times)

The flags for printing help and version are also supported.

During its execution the program will prnt a TUI-like interface which will show:
- The percentage of jobs which have finished
- The average memory usage (only after at least one job has finished its execution)
- The average runtime (only after at least one job has finished its execution)
- The estimated duration which the analysis will run and ETA (only after at least one job has finished its execution)
- Colorful tiles whcich represent a job (each color represents the current state of the job, e.g. pending, running, completed)

![An example of the TUI the user can expect to see when running the program](/images/tui_example.png)

## Known Issues

1. The program will crash if all requested jobs are still pending
2. The program is incorrectly calculating the duration and ETA times

## Final Note

Please do not overuse this script. It utilises a lot of practices whoch any IT specialist at a large batch farm would advise against.