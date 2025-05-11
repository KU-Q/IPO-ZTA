IPO-ZTA provides a simulation framework for smart transportation systems and IoT environments, enabling analysis of network security and performance in vehicular networks.



### Prerequisites
- ns-3
- Python 3.8
- CMake
- C++ compiler (g++ or compatible)

### Installation

1. download and install ns-3

2. Clone the repository:

   ```
   git clone https://github.com/KU-Q/IPO-ZTA.git
   cd IPO-ZTA
   ```

   Add the documents corresponding to IPO-ZTA to the ns3 project internal.

3. Configure and build ns-3:

   ```
   ./waf configure --enable-examples
   ./waf
   ```

### Running Simulations

The main simulation script is located in `scratch/its-simulation`. You can run it with various parameters:

```bash
./waf --run "scratch/its-simulation[options]"
```

#### Available Command Line Parameters

| Parameter    | Description                          | Default Value |
| ------------ | ------------------------------------ | ------------- |
| totaltime    | Simulation end time                  | -             |
| nNodes       | Number of nodes (vehicles)           | -             |
| txp          | Transmit power (dB)                  | -             |
| phyMode      | Wifi Phy mode                        | -             |
| traceFile    | Ns2 movement trace file              | -             |
| mobTraceFile | Mobility Trace file                  | -             |
| rate         | Data rate                            | -             |
| verbose      | Verbosity level (0=quiet, 1=verbose) | -             |
| bsm          | WAVE BSM size (bytes)                | -             |
| interval     | WAVE BSM interval (s)                | -             |
| asciiTrace   | Enable ASCII trace                   | -             |
| pcap         | Enable PCAP file generation          | -             |
| schemeName   | Scheduling algorithm name            | -             |
| globalDbSize | Size of global database              | -             |

### Output Files
The simulation generates several output files:
- `.log` files: Detailed simulation logs
- `.mob` files: Mobility trace files
- `.xml` files: Animation and configuration files
- `/result/` directory: Contains simulation results and plots
- `.pcap` files: Packet capture files for network analysis

### Project Structure
```
IPO-ZTA/
├── contrib/          # Additional contributions
├── scratch/          # Simulation scripts
└── wave/            # WAVE module implementations
```

### Documentation
For more detailed information about ns-3, please refer to:
- [ns-3 Documentation](http://www.nsnam.org/documentation/)
- [ns-3 Tutorial](http://www.nsnam.org/tutorials.html)
- [ns-3 API Documentation](http://www.nsnam.org/doxygen/index.html)





