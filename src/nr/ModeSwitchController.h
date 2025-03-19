#ifndef __MODE_SWITCH_CONTROLLER_H
#define __MODE_SWITCH_CONTROLLER_H

#include <omnetpp.h>
#include <vector>
#include <map>

using namespace omnetpp;

namespace nr {

class NRModule;  // Forward declaration

/**
 * @brief Enumeration of V2X operation modes
 */
enum class V2XMode {
    MODE_1,        ///< Network scheduled mode
    MODE_2,        ///< Autonomous mode
    MODE_3,        ///< Semi-persistent scheduling
    MODE_4         ///< Sensing-based semi-persistent scheduling
};

/**
 * @brief Structure to hold mode switching parameters
 */
struct ModeSwitchParams {
    double rsrpThreshold;      ///< RSRP threshold for mode switching (dBm)
    double hysteresis;         ///< Hysteresis margin (dB)
    simtime_t timeToTrigger;   ///< Time to trigger switching (s)
    
    ModeSwitchParams() :
        rsrpThreshold(-110.0),  // Default values
        hysteresis(3.0),
        timeToTrigger(1.0) {}
};

/**
 * @brief Controller class for V2X mode switching decisions
 *
 * Handles the logic for switching between different V2X operation modes
 * based on network conditions, resource availability, and configured thresholds.
 */
class ModeSwitchController
{
  public:
    // Constructor and destructor
    ModeSwitchController(NRModule* parent);
    virtual ~ModeSwitchController();
    
    // Mode switching interface
    bool evaluateSwitch();
    bool executeSwitch(int newMode);
    int determineTargetMode();
    
    // Configuration
    void setParameters(const ModeSwitchParams& params);
    void enableMode(V2XMode mode, bool enabled);
    
    // Status queries
    V2XMode getCurrentMode() const { return currentMode; }
    simtime_t getLastSwitchTime() const { return lastSwitchTime; }
    int getTotalSwitches() const { return totalSwitches; }
    bool isModeEnabled(V2XMode mode) const;
    
  protected:
    // Internal utility functions
    bool validateModeTransition(V2XMode targetMode) const;
    bool checkRSRPCondition() const;
    bool checkResourceAvailability() const;
    void updateModeHistory(V2XMode newMode);
    
    // Monitoring and metrics
    double measureNetworkQuality() const;
    void collectPerformanceMetrics();
    bool evaluatePerformanceThresholds() const;
    
    // Event handling
    void onModeSwitchSuccess(V2XMode oldMode, V2XMode newMode);
    void onModeSwitchFailure(V2XMode targetMode, const char* reason);
    
  private:
    // Parent module reference
    NRModule* parentModule;
    
    // Current state
    V2XMode currentMode;
    simtime_t lastSwitchTime;
    simtime_t lastEvaluationTime;
    
    // Configuration
    ModeSwitchParams switchParams;
    std::map<V2XMode, bool> enabledModes;
    
    // Statistics and history
    int totalSwitches;
    std::vector<std::pair<simtime_t, V2XMode>> modeHistory;
    double currentRSRP;
    
    // Performance metrics
    struct Metrics {
        double packetDeliveryRatio;
        double latency;
        double resourceUtilization;
        
        Metrics() : packetDeliveryRatio(0), latency(0), resourceUtilization(0) {}
    } currentMetrics;
    
    // Constants
    static const simtime_t MIN_SWITCH_INTERVAL;
    static const int MAX_HISTORY_SIZE = 100;
    static const double DEFAULT_RSRP_THRESHOLD;
    
    // Utility functions
    bool isStableState() const;
    bool hasRecentlySwitched() const;
    void validateParameters() const;
    void initializeMetrics();
    
    // Error handling
    void handleSwitchError(const char* message);
    void logModeTransition(V2XMode oldMode, V2XMode newMode, bool success);
    
    // Mode transition validation
    bool isValidTransition(V2XMode from, V2XMode to) const;
    bool meetsTransitionRequirements(V2XMode targetMode) const;
};

}  // namespace nr

#endif // __MODE_SWITCH_CONTROLLER_H