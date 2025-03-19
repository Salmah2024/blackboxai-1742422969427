#include "NRModule.h"
#include <inet/common/ModuleAccess.h>
#include <inet/common/lifecycle/NodeStatus.h>
#include <simu5g/stack/phy/layer/NRPhy.h>

using namespace simu5g;  // Add this for NRPhy

namespace nr {

Define_Module(NRModule);

NRModule::NRModule() : 
    numerologyIndex(0),
    carrierFrequency(0),
    bandwidth(0),
    sidelinkEnabled(false),
    resourceManager(nullptr),
    modeSwitchController(nullptr),
    isTransmitting(false),
    lastAllocationTime(0),
    resourceAllocationTimer(nullptr),
    modeSwitchEvaluationTimer(nullptr)
{
}

NRModule::~NRModule()
{
    // Clean up timers
    cancelAndDelete(resourceAllocationTimer);
    cancelAndDelete(modeSwitchEvaluationTimer);
    
    // Clean up managers
    delete resourceManager;
    delete modeSwitchController;
}

void NRModule::initialize(int stage)
{
    if (stage == 0) {
        // Register signals for statistics collection
        resourceAllocationSignal = registerSignal("resourceAllocation");
        modeSwitchSignal = registerSignal("modeSwitch");
        sidelinkQualitySignal = registerSignal("sidelinkQuality");
        
        // Read configuration parameters
        try {
            numerologyIndex = par("numerologyIndex");
            carrierFrequency = par("carrierFrequency").doubleValue();
            bandwidth = par("bandwidth");
            sidelinkEnabled = par("sidelinkEnabled");
            
            // Validate parameters
            validateParameters();
        }
        catch (const std::exception& e) {
            handleError(e.what());
            throw;
        }
        
        // Initialize timers
        resourceAllocationTimer = new cMessage("resourceAllocationTimer");
        modeSwitchEvaluationTimer = new cMessage("modeSwitchEvaluationTimer");
        
        // Create managers
        resourceManager = new ResourceManager(this);
        modeSwitchController = new ModeSwitchController(this);
        
        EV_INFO << "NRModule initialized with numerology " << numerologyIndex 
                << " at " << carrierFrequency/1e9 << " GHz" << endl;
    }
    else if (stage == 1) {
        // Schedule initial events
        scheduleNextResourceAllocation();
        scheduleNextModeSwitchEvaluation();
        
        // Initialize statistics collection
        initializeStatistics();
    }
}

void NRModule::handleMessage(cMessage *msg)
{
    try {
        if (msg->isSelfMessage()) {
            if (msg == resourceAllocationTimer) {
                processResourceAllocation();
                scheduleNextResourceAllocation();
            }
            else if (msg == modeSwitchEvaluationTimer) {
                evaluateModeSwitching();
                scheduleNextModeSwitchEvaluation();
            }
        }
        else {
            // Handle incoming messages
            cPacket *packet = check_and_cast<cPacket *>(msg);
            processPacket(packet);
        }
    }
    catch (const std::exception& e) {
        EV_ERROR << "Error handling message: " << e.what() << endl;
        handleError(e.what());
    }
}

void NRModule::validateParameters()
{
    if (numerologyIndex < 0 || numerologyIndex > 4) {
        throw cRuntimeError("Invalid numerology index %d (valid range: 0-4)", numerologyIndex);
    }
    
    if (carrierFrequency <= 0) {
        throw cRuntimeError("Invalid carrier frequency %f", carrierFrequency);
    }
    
    if (bandwidth <= 0) {
        throw cRuntimeError("Invalid bandwidth %d", bandwidth);
    }
}

void NRModule::processResourceAllocation()
{
    EV_INFO << "Processing resource allocation at " << simTime() << endl;
    
    try {
        // Update resource utilization statistics
        updateResourceUtilization();
        
        // Perform resource allocation
        if (resourceManager->allocateResources()) {
            emit(resourceAllocationSignal, 1);  // Success
            logResourceStatus();
        }
        else {
            emit(resourceAllocationSignal, 0);  // Failure
            EV_WARN << "Resource allocation failed" << endl;
        }
    }
    catch (const std::exception& e) {
        EV_ERROR << "Error in resource allocation: " << e.what() << endl;
        emit(resourceAllocationSignal, -1);  // Error
    }
}

void NRModule::evaluateModeSwitching()
{
    EV_INFO << "Evaluating mode switching at " << simTime() << endl;
    
    try {
        if (isModeSwitchAllowed()) {
            bool switchRequired = modeSwitchController->evaluateSwitch();
            if (switchRequired) {
                int newMode = modeSwitchController->determineTargetMode();
                bool success = switchMode(newMode);
                emit(modeSwitchSignal, success ? newMode : -1);
            }
        }
    }
    catch (const std::exception& e) {
        EV_ERROR << "Error in mode switching: " << e.what() << endl;
        emit(modeSwitchSignal, -2);  // Error
    }
}

void NRModule::scheduleNextResourceAllocation()
{
    // Schedule next resource allocation based on numerology
    simtime_t slot_duration = (double)(0.001) / (1 << numerologyIndex);  // in seconds
    scheduleAt(simTime() + slot_duration, resourceAllocationTimer);
}

void NRModule::scheduleNextModeSwitchEvaluation()
{
    // Evaluate mode switching every 100ms
    scheduleAt(simTime() + 0.1, modeSwitchEvaluationTimer);
}

bool NRModule::requestResource(int priority, int size)
{
    if (!isResourceAvailable(size)) {
        EV_WARN << "Resource not available for size " << size << endl;
        return false;
    }
    
    try {
        bool allocated = resourceManager->allocateSpecific(priority, size);
        if (allocated) {
            lastAllocationTime = simTime();
            isTransmitting = true;
            EV_INFO << "Resource allocated: priority=" << priority << ", size=" << size << endl;
        }
        return allocated;
    }
    catch (const std::exception& e) {
        EV_ERROR << "Error requesting resource: " << e.what() << endl;
        return false;
    }
}

void NRModule::releaseResource(int resourceId)
{
    try {
        resourceManager->release(resourceId);
        isTransmitting = false;
        EV_INFO << "Resource " << resourceId << " released" << endl;
    }
    catch (const std::exception& e) {
        EV_ERROR << "Error releasing resource: " << e.what() << endl;
    }
}

bool NRModule::switchMode(int newMode)
{
    if (!isModeSwitchAllowed()) {
        EV_WARN << "Mode switch not allowed at this time" << endl;
        return false;
    }
    
    try {
        bool success = modeSwitchController->executeSwitch(newMode);
        notifyModeSwitchComplete(success);
        return success;
    }
    catch (const std::exception& e) {
        EV_ERROR << "Error switching mode: " << e.what() << endl;
        return false;
    }
}

void NRModule::finish()
{
    // Record final statistics
    recordScalar("resourceUtilization", resourceManager->getUtilization());
    recordScalar("totalModeSwitches", modeSwitchController->getTotalSwitches());
    
    // Log final status
    EV_INFO << "NRModule finishing at " << simTime() 
            << ", total mode switches: " << modeSwitchController->getTotalSwitches() << endl;
}

void NRModule::handleError(const char* message)
{
    EV_ERROR << "Error in NRModule: " << message << endl;
    // Optionally throw exception or handle error recovery
}

void NRModule::initializeStatistics()
{
    // Initialize statistics recording
    WATCH(isTransmitting);
    WATCH(lastAllocationTime);
}

void NRModule::updateResourceUtilization()
{
    double utilization = resourceManager->getUtilization();
    emit(sidelinkQualitySignal, utilization);
}

bool NRModule::isResourceAvailable(int size) const
{
    return resourceManager->checkAvailability(size);
}

bool NRModule::isModeSwitchAllowed() const
{
    // Prevent too frequent mode switches
    return (simTime() - modeSwitchController->getLastSwitchTime()) > 1.0;
}

void NRModule::notifyModeSwitchComplete(bool success)
{
    EV_INFO << "Mode switch " << (success ? "completed" : "failed") << endl;
}

void NRModule::logResourceStatus()
{
    EV_INFO << "Current resource status:" << endl
            << "  Utilization: " << resourceManager->getUtilization() << endl
            << "  Available blocks: " << resourceManager->getAvailableBlocks() << endl;
}

}  // namespace nr