# ITS Simulation Project

This project simulates an Intelligent Transportation System (ITS) using NS-3.

## Project Structure

```
its-simulation/
├── its-simulation.cc    # 主程序文件
├── config.h            # 配置文件
└── output/             # 输出目录
    ├── its-simulation.pcap  # PCAP文件
    └── its-simulation.tr    # ASCII跟踪文件
```

## Building and Running

1. Make sure you're in the NS-3 root directory:
```bash
cd /path/to/ns-3-dev
```

2. Build the project:
```bash
./waf configure --enable-examples
./waf build
```

3. Run the simulation:
```bash
./waf --run its-simulation
```

## Output Files

The simulation generates two types of output files:

1. PCAP files (`output/its-simulation.pcap`):
   - Contains network traffic data
   - Can be analyzed using Wireshark or similar tools

2. ASCII trace files (`output/its-simulation.tr`):
   - Contains detailed simulation events
   - Useful for debugging and analysis

## Configuration

You can modify simulation parameters in `config.h`:

- Number of vehicles
- Simulation time
- Packet size and interval
- Mobility model parameters
- Network parameters

## Analyzing Results

1. Open PCAP files in Wireshark:
```bash
wireshark output/its-simulation.pcap
```

2. Analyze ASCII trace files:
```bash
# View the trace file
cat output/its-simulation.tr
```

## Notes

- Make sure you have enough disk space for output files
- Adjust simulation parameters based on your system capabilities
- The output directory is automatically created when running the simulation 