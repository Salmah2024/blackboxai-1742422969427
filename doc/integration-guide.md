# Integration Guide for 5G NR V2X Simulation

This guide provides detailed instructions for installing and configuring all external dependencies required for the 5G NR V2X simulation environment.

## 1. OMNeT++ Installation

1. Download OMNeT++ 5.6.2 or later from [omnetpp.org](https://omnetpp.org/download/)

2. Extract and install:
```bash
tar xvfz omnetpp-5.6.2-src.tgz
cd omnetpp-5.6.2
source setenv
./configure
make -j$(nproc)
```

3. Set environment variable:
```bash
export OMNETPP_ROOT=/path/to/omnetpp-5.6.2
```

## 2. SUMO Installation

1. Install dependencies:
```bash
# Ubuntu/Debian
sudo apt-get install libxerces-c-dev libproj-dev libgdal-dev libfox-1.6-dev
```

2. Clone and build SUMO:
```bash
git clone https://github.com/eclipse/sumo
cd sumo
mkdir build && cd build
cmake ..
make -j$(nproc)
```

3. Set environment variable:
```bash
export SUMO_ROOT=/path/to/sumo
export PATH=$PATH:$SUMO_ROOT/bin
```

## 3. Veins Installation

1. Clone Veins:
```bash
git clone https://github.com/sommer/veins
cd veins
```

2. Configure and build:
```bash
./configure
make -j$(nproc)
```

3. Set environment variable:
```bash
export VEINS_ROOT=/path/to/veins
```

## 4. INET Installation

1. Clone INET:
```bash
git clone https://github.com/inet-framework/inet
cd inet
```

2. Build:
```bash
make makefiles
make -j$(nproc)
```

3. Set environment variable:
```bash
export INET_ROOT=/path/to/inet
```

## 5. Simu5G Installation

1. Clone Simu5G:
```bash
git clone https://github.com/Simu5G/simu5g
cd simu5g
```

2. Build:
```bash
make makefiles
make -j$(nproc)
```

3. Set environment variable:
```bash
export SIMU5G_ROOT=/path/to/simu5g
```

## Verification Steps

1. Test OMNeT++ Installation:
```bash
which omnetpp
# Should show path to OMNeT++ installation
```

2. Test SUMO Installation:
```bash
sumo --version
# Should display SUMO version information
```

3. Test Veins Integration:
```bash
cd $VEINS_ROOT/examples/veins
./run -u Cmdenv
# Should run without errors
```

4. Test INET:
```bash
cd $INET_ROOT/examples/inet
./run_inet -u Cmdenv
# Should run without errors
```

5. Test Simu5G:
```bash
cd $SIMU5G_ROOT/examples/NR
./run -u Cmdenv
# Should run without errors
```

## Common Issues and Solutions

### 1. Build Errors

- **Missing Dependencies**: Ensure all required development libraries are installed
```bash
sudo apt-get install build-essential gcc g++ bison flex perl \
    qt5-default libqt5opengl5-dev tcl-dev tk-dev libxml2-dev \
    zlib1g-dev default-jre doxygen graphviz libwebkit2gtk-4.0-dev
```

- **Incompatible Versions**: Verify that all components are using compatible versions
- **Path Issues**: Double-check environment variables are set correctly

### 2. Runtime Errors

- **SUMO not found**: Ensure SUMO_HOME environment variable is set
- **OMNeT++ errors**: Verify OMNeT++ is properly installed and environment is sourced
- **Library linking errors**: Check if all libraries are built with the same OMNeT++ version

### 3. Integration Issues

- **Veins/SUMO Connection**: Test with veins_launchd and verify TraCI connection
- **INET Framework**: Ensure INET is built with the correct OMNeT++ version
- **Simu5G Integration**: Verify compatibility with installed INET version

## Additional Resources

- [OMNeT++ Documentation](https://doc.omnetpp.org/)
- [SUMO Documentation](https://sumo.dlr.de/docs/)
- [Veins Documentation](https://veins.car2x.org/documentation/)
- [INET Documentation](https://inet.omnetpp.org/docs/users-guide/)
- [Simu5G Documentation](https://github.com/Simu5G/simu5g/wiki)

## Support

For issues specific to this project:
1. Check the troubleshooting section in README.md
2. Review simulation logs for error messages
3. Verify all environment variables are set correctly
4. Ensure all dependencies are built with compatible versions