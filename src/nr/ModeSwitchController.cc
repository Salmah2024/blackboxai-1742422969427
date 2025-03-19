#include "ModeSwitchController.h"
#include "NRModule.h"
#include <algorithm>
#include <stdexcept>

namespace nr {

// Define static constants
const simtime_t ModeSwitchController::MIN_SWITCH_INTERVAL = 1.0;  // 1 second
const double ModeSwitchController::DEFAULT_RSRP_THRESHOLD = -110.0;  // dBm

ModeSwitchController::ModeSwitchController(NRModule* parent) :
    parentModule(parent),
    currentMode(V2XMode::MODE_2),  // Start in autonomous mode
    lastSwitchTime(0),
    lastEvaluationTime(0),
    totalSwitches(0),
    currentRSRP(0)
{
    if (!parent) {
        throw std::runtime_error("ModeSwitchController: Parent module cannot be null");
    }
    
    // Initialize enabled modes
    enabledModes[V2XMode::MODE_1] = true;
    enabledModes[V2XMode::MODE_2] = true;
    enabledModes[V2XMode::MODE_3] = true;
    enabledModes[V2XMode::MODE_4] = true;
    
    // Initialize metrics
    initializeMetrics();
}

ModeSwitchController::~ModeSwitchController()
{
    // Cleanup if needed
}

bool ModeSwitchController::evaluateSwitch()
{
    try {
        // Update current metrics
        collectPerformanceMetrics();
        
        // Check if we're in a stable state and enough time has passed
        if (!isStableState() || hasRecentlySwitched()) {
            return false;
        }
        
        // Evaluate network conditions
        double networkQuality = measureNetworkQuality();
        bool rsrpCondition = checkRSRPCondition();
        bool resourceCondition = checkResourceAvailability();
        
        // Determine if switch is needed based on conditions
        if (rsrpCondition && resourceCondition) {
            V2XMode targetMode = static_cast<V2XMode>(determineTargetMode());
            
            if (targetMode != currentMode && validateModeTransition(targetMode)) {
                EV_INFO << "Mode switch evaluation suggests switching to mode " 
                       << static_cast<int>(targetMode) << endl;
                return true;
            }
        }
        
        lastEvaluationTime = simTime();
        return false;
    }
    catch (const std::exception& e) {
        handleSwitchError(e.what());
        return false;
    }
}

bool ModeSwitchController::executeSwitch(int newMode)
{
    try {
        V2XMode targetMode = static_cast<V2XMode>(newMode);
        
        // Validate the transition
        if (!validateModeTransition(targetMode)) {
            EV_WARN << "Invalid mode transition from " << static_cast<int>(currentMode)
                    << " to " << newMode << endl;
            return false;
        }
        
        // Store old mode for logging
        V2XMode oldMode = currentMode;
        
        // Perform the switch
        if (meetsTransitionRequirements(targetMode)) {
            currentMode = targetMode;
            lastSwitchTime = simTime();
            totalSwitches++;
            
            // Update history
            updateModeHistory(targetMode);
            
            // Log success
            onModeSwitchSuccess(oldMode, targetMode);
            return true;
        }
        else {
            onModeSwitchFailure(targetMode, "Transition requirements not met");
            return false;
        }
    }
    catch (const std::exception& e) {
        handleSwitchError(e.what());
        return false;
    }
}

int ModeSwitchController::determineTargetMode()
{
    // Collect current performance metrics
    collectPerformanceMetrics();
    
    // Decision logic based on network conditions and performance
    if (currentRSRP > switchParams.rsrpThreshold + switchParams.hysteresis) {
        // Good network conditions - prefer network scheduled mode
        return static_cast<int>(V2XMode::MODE_1);
    }
    else if (currentMetrics.resourceUtilization > 0.8) {
        // High resource utilization - consider semi-persistent scheduling
        return static_cast<int>(V2XMode::MODE_3);
    }
    else if (currentRSRP < switchParams.rsrpThreshold - switchParams.hysteresis) {
        // Poor network conditions - use autonomous mode
        return static_cast<int>(V2XMode::MODE_2);
    }
    
    // Default to current mode if no clear decision
    return static_cast<int>(currentMode);
}

void ModeSwitchController::setParameters(const ModeSwitchParams& params)
{
    switchParams = params;
    validateParameters();
}

void ModeSwitchController::enableMode(V2XMode mode, bool enabled)
{
    enabledModes[mode] = enabled;
}

bool ModeSwitchController::isModeEnabled(V2XMode mode) const
{
    auto it = enabledModes.find(mode);
    return it != enabledModes.end() && it->second;
}

bool ModeSwitchController::validateModeTransition(V2XMode targetMode) const
{
    // Check if target mode is enabled
    if (!isModeEnabled(targetMode)) {
        EV_WARN << "Target mode " << static_cast<int>(targetMode) << " is not enabled" << endl;
        return false;
    }
    
    // Check if transition is valid
    if (!isValidTransition(currentMode, targetMode)) {
        EV_WARN << "Invalid transition from mode " << static_cast<int>(currentMode)
                << " to mode " << static_cast<int>(targetMode) << endl;
        return false;
    }
    
    return true;
}

bool ModeSwitchController::checkRSRPCondition() const
{
    return std::abs(currentRSRP - switchParams.rsrpThreshold) > switchParams.hysteresis;
}

bool ModeSwitchController::checkResourceAvailability() const
{
    // Check resource utilization through parent module
    return currentMetrics.resourceUtilization < 0.9;  // 90% threshold
}

void ModeSwitchController::updateModeHistory(V2XMode newMode)
{
    modeHistory.push_back(std::make_pair(simTime(), newMode));
    
    // Keep history size bounded
    if (modeHistory.size() > MAX_HISTORY_SIZE) {
        modeHistory.erase(modeHistory.begin());
    }
}

double ModeSwitchController::measureNetworkQuality() const
{
    // Combine multiple metrics for network quality assessment
    double quality = 0.0;
    quality += 0.4 * currentMetrics.packetDeliveryRatio;
    quality += 0.3 * (1.0 - currentMetrics.latency / 100.0);  // Normalize latency
    quality += 0.3 * (1.0 - currentMetrics.resourceUtilization);
    
    return quality;
}

void ModeSwitchController::collectPerformanceMetrics()
{
    // Update metrics from parent module
    if (parentModule) {
        // These would be actual measurements in a real implementation
        currentMetrics.packetDeliveryRatio = 0.95;  // Example value
        currentMetrics.latency = 20.0;              // Example value (ms)
        currentMetrics.resourceUtilization = 0.7;   // Example value
        
        // Update RSRP measurement
        currentRSRP = -105.0;  // Example value (dBm)
    }
}

bool ModeSwitchController::evaluatePerformanceThresholds() const
{
    return currentMetrics.packetDeliveryRatio > 0.9 &&
           currentMetrics.latency < 50.0 &&
           currentMetrics.resourceUtilization < 0.8;
}

void ModeSwitchController::onModeSwitchSuccess(V2XMode oldMode, V2XMode newMode)
{
    EV_INFO << "Successfully switched from mode " << static_cast<int>(oldMode)
            << " to mode " << static_cast<int>(newMode) << endl;
    logModeTransition(oldMode, newMode, true);
}

void ModeSwitchController::onModeSwitchFailure(V2XMode targetMode, const char* reason)
{
    EV_ERROR << "Failed to switch to mode " << static_cast<int>(targetMode)
             << ": " << reason << endl;
    logModeTransition(currentMode, targetMode, false);
}

bool ModeSwitchController::isStableState() const
{
    return (simTime() - lastEvaluationTime) >= switchParams.timeToTrigger;
}

bool ModeSwitchController::hasRecentlySwitched() const
{
    return (simTime() - lastSwitchTime) < MIN_SWITCH_INTERVAL;
}

void ModeSwitchController::validateParameters() const
{
    if (switchParams.rsrpThreshold > -70 || switchParams.rsrpThreshold < -140) {
        throw std::runtime_error("Invalid RSRP threshold value");
    }
    
    if (switchParams.hysteresis < 0 || switchParams.hysteresis > 10) {
        throw std::runtime_error("Invalid hysteresis value");
    }
    
    if (switchParams.timeToTrigger < 0) {
        throw std::runtime_error("Invalid time to trigger value");
    }
}

void ModeSwitchController::initializeMetrics()
{
    currentMetrics.packetDeliveryRatio = 1.0;
    currentMetrics.latency = 0.0;
    currentMetrics.resourceUtilization = 0.0;
}

void ModeSwitchController::handleSwitchError(const char* message)
{
    EV_ERROR << "Mode switch error: " << message << endl;
}

void ModeSwitchController::logModeTransition(V2XMode oldMode, V2XMode newMode, bool success)
{
    EV_INFO << "Mode transition: " << static_cast<int>(oldMode)
            << " -> " << static_cast<int>(newMode)
            << " [" << (success ? "SUCCESS" : "FAILURE") << "]"
            << " at time " << simTime() << endl;
}

bool ModeSwitchController::isValidTransition(V2XMode from, V2XMode to) const
{
    // Define valid transitions
    // In this implementation, all transitions are allowed between enabled modes
    return isModeEnabled(from) && isModeEnabled(to);
}

bool ModeSwitchController::meetsTransitionRequirements(V2XMode targetMode) const
{
    // Check specific requirements for each target mode
    switch (targetMode) {
        case V2XMode::MODE_1:
            return currentRSRP > switchParams.rsrpThreshold;
            
        case V2XMode::MODE_2:
            return true;  // Always allowed as fallback
            
        case V2XMode::MODE_3:
            return currentMetrics.resourceUtilization < 0.8;
            
        case V2XMode::MODE_4:
            return currentMetrics.packetDeliveryRatio > 0.8;
            
        default:
            return false;
    }
}

}  // namespace nr