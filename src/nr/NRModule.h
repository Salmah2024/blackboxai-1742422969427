#ifndef __NR_MODULE_H
#define __NR_MODULE_H

#include <omnetpp.h>
#include <inet/common/INETDefs.h>
#include <simu5g/stack/phy/layer/NRPhy.h>
#include "ResourceManager.h"
#include "ModeSwitchController.h"

using namespace omnetpp;

namespace nr {

/**
 * @brief Main module for 5G NR V2X sidelink communication
 *
 * This module handles the core functionality of 5G NR V2X sidelink,
 * including resource allocation and mode switching.
 */
class NRModule : public cSimpleModule
{
  protected:
    // Configuration parameters
    int numerologyIndex;         ///< 5G NR numerology (0-4)
    double carrierFrequency;     ///< Carrier frequency in Hz
    int bandwidth;               ///< Bandwidth in MHz
    bool sidelinkEnabled;        ///< Flag for sidelink capability
    
    // Resource management
    ResourceManager* resourceManager;
    ModeSwitchController* modeSwitchController;
    
    // Statistics
    simsignal_t resourceAllocationSignal;
    simsignal_t modeSwitchSignal;
    simsignal_t sidelinkQualitySignal;
    
    // Internal state
    bool isTransmitting;
    simtime_t lastAllocationTime;
    
    // Self messages for periodic events
    cMessage *resourceAllocationTimer;
    cMessage *modeSwitchEvaluationTimer;
    
  protected:
    // OMNeT++ module interface
    virtual void initialize(int stage) override;
    virtual void handleMessage(cMessage *msg) override;
    virtual void finish() override;
    virtual int numInitStages() const override { return 2; }
    
    // Internal utility functions
    void scheduleNextResourceAllocation();
    void scheduleNextModeSwitchEvaluation();
    void processResourceAllocation();
    void evaluateModeSwitching();
    
  public:
    NRModule();
    virtual ~NRModule();
    
    // Public interface
    bool isSidelinkEnabled() const { return sidelinkEnabled; }
    double getCarrierFrequency() const { return carrierFrequency; }
    int getBandwidth() const { return bandwidth; }
    
    // Resource management interface
    bool requestResource(int priority, int size);
    void releaseResource(int resourceId);
    
    // Mode switching interface
    void triggerModeSwitchEvaluation();
    bool switchMode(int newMode);
    
  private:
    // Utility functions
    void initializeStatistics();
    void registerSignals();
    void checkConfiguration();
    void logResourceStatus();
    
    // Error handling
    void handleError(const char* message);
    void validateParameters();
    
    // Resource management helpers
    bool isResourceAvailable(int size) const;
    void updateResourceUtilization();
    
    // Mode switching helpers
    bool isModeSwitchAllowed() const;
    void notifyModeSwitchComplete(bool success);
};

}  // namespace nr

#endif // __NR_MODULE_H