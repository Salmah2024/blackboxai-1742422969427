# 5G NR V2X Sidelink Simulation

This project implements a 5G NR V2X sidelink simulation environment using OMNeT++, focusing on resource allocation and mode switching. It integrates with SUMO for traffic simulation, Veins for vehicular networking, INET for network protocols, and Simu5G for 5G NR modeling.

## Prerequisites

- OMNeT++ 5.6.2 or later
- SUMO 1.8.0 or later
- Veins 5.1 or later
- INET 4.2.2 or later
- Simu5G (latest version)
- CMake 3.1 or later
- C++ compiler (GCC 7.3 or later)

## Environment Setup

Set the following environment variables:

```bash
export OMNETPP_ROOT=/path/to/omnet
export SUMO_ROOT=/path/to/sumo
export VEINS_ROOT=/path/to/veins
export INET_ROOT=/path/to/inet
export SIMU5G_ROOT=/path/to/simu5g
```

## Building the Project

1. Create build directory:
```bash
mkdir build && cd build
```

2. Configure with CMake:
```bash
cmake .. \
    -DOMNETPP_ROOT=$OMNETPP_ROOT \
    -DSUMO_ROOT=$SUMO_ROOT \
    -DVEINS_ROOT=$VEINS_ROOT \
    -DINET_ROOT=$INET_ROOT \
    -DSIMU5G_ROOT=$SIMU5G_ROOT
```

3. Build:
```bash
make -j$(nproc)
```

## Running Simulations

1. Basic simulation:
```bash
./run_simulation -u Cmdenv -c Urban -n ../simulations/networks
```

2. With GUI (Qtenv):
```bash
./run_simulation -u Qtenv -c Urban
```

## Project Structure

```
5g-nr-v2x/
├── src/                    # Source files
│   ├── nr/                 # 5G NR specific modules
│   ├── v2x/               # V2X communication modules
│   └── utils/             # Utility classes
├── simulations/           # Simulation configurations
│   ├── scenarios/         # Simulation scenarios
│   ├── networks/          # Network definitions
│   └── config/           # Configuration files
├── tests/                 # Unit tests
├── doc/                   # Documentation
├── cmake/                 # CMake modules
└── CMakeLists.txt        # Main CMake configuration
```

## Troubleshooting

1. Missing Dependencies
- Ensure all environment variables are set correctly
- Verify library versions are compatible
- Check CMake error messages for missing paths

2. Build Issues
- Clean build directory and rebuild
- Verify compiler version compatibility
- Check for detailed error messages in CMake output

3. Runtime Issues
- Check simulation logs for errors
- Verify network configuration
- Monitor resource usage

## License

This project is licensed under the MIT License - see the LICENSE file for details.