#ifndef __RESOURCE_MANAGER_H
#define __RESOURCE_MANAGER_H

#include <omnetpp.h>
#include <vector>
#include <map>
#include <memory>

using namespace omnetpp;

namespace nr {

class NRModule;  // Forward declaration

/**
 * @brief Structure representing a sidelink resource block
 */
struct ResourceBlock {
    int id;                  ///< Unique identifier for the resource block
    int subchannelIndex;     ///< Subchannel index
    int symbolIndex;         ///< Symbol index
    bool occupied;           ///< Occupation status
    int priority;            ///< Priority level of current allocation
    simtime_t allocTime;     ///< Time when the resource was allocated
    
    ResourceBlock() : 
        id(0), subchannelIndex(0), symbolIndex(0), 
        occupied(false), priority(0), allocTime(0) {}
};

/**
 * @brief Manager class for 5G NR V2X sidelink resource allocation
 *
 * Handles dynamic resource allocation, conflict resolution, and resource pool management
 * for V2X sidelink communication.
 */
class ResourceManager
{
  public:
    // Constructor and destructor
    ResourceManager(NRModule* parent);
    virtual ~ResourceManager();
    
    // Resource allocation interface
    bool allocateResources();
    bool allocateSpecific(int priority, int size);
    void release(int resourceId);
    bool checkAvailability(int size) const;
    
    // Status queries
    double getUtilization() const;
    int getAvailableBlocks() const;
    std::vector<int> getOccupiedResources() const;
    
    // Configuration
    void setPoolSize(int numSubchannels, int numSymbols);
    void setPeriodicity(simtime_t period);
    
  protected:
    // Internal utility functions
    bool initializePool();
    void clearPool();
    bool validateRequest(int priority, int size) const;
    std::vector<ResourceBlock*> findAvailableBlocks(int size) const;
    void markBlocksOccupied(const std::vector<ResourceBlock*>& blocks, int priority);
    void cleanExpiredAllocations();
    
    // Conflict resolution
    bool resolveConflict(const std::vector<ResourceBlock*>& blocks);
    bool isConflicting(const ResourceBlock* block1, const ResourceBlock* block2) const;
    
    // Monitoring and statistics
    void updateUtilizationStats();
    void logAllocation(const std::vector<ResourceBlock*>& blocks, int priority);
    
  private:
    // Parent module reference
    NRModule* parentModule;
    
    // Resource pool configuration
    int numSubchannels;
    int numSymbols;
    simtime_t periodicity;
    
    // Resource management
    std::vector<std::unique_ptr<ResourceBlock>> resourcePool;
    std::map<int, std::vector<ResourceBlock*>> activeAllocations;
    
    // Statistics
    double currentUtilization;
    int totalAllocations;
    int failedAllocations;
    
    // Internal state
    bool initialized;
    simtime_t lastCleanupTime;
    
    // Constants
    static const int MAX_RETRIES = 3;
    static const simtime_t CLEANUP_INTERVAL;
    
    // Utility functions
    int generateResourceId() const;
    bool isValidResourceId(int id) const;
    void validatePoolConfiguration() const;
    
    // Error handling
    void handleAllocationError(const char* message);
    void logResourceStatus() const;
};

}  // namespace nr

#endif // __RESOURCE_MANAGER_H