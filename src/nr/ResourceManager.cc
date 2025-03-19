#include "ResourceManager.h"
#include "NRModule.h"
#include <algorithm>
#include <stdexcept>

namespace nr {

// Define static constants
const simtime_t ResourceManager::CLEANUP_INTERVAL = 0.1;  // 100ms

ResourceManager::ResourceManager(NRModule* parent) :
    parentModule(parent),
    numSubchannels(0),
    numSymbols(0),
    periodicity(0),
    currentUtilization(0.0),
    totalAllocations(0),
    failedAllocations(0),
    initialized(false),
    lastCleanupTime(0)
{
    if (!parent) {
        throw std::runtime_error("ResourceManager: Parent module cannot be null");
    }
}

ResourceManager::~ResourceManager()
{
    clearPool();
}

bool ResourceManager::allocateResources()
{
    if (!initialized) {
        if (!initializePool()) {
            return false;
        }
    }

    // Clean expired allocations if needed
    if (simTime() - lastCleanupTime >= CLEANUP_INTERVAL) {
        cleanExpiredAllocations();
    }

    try {
        // Update utilization statistics
        updateUtilizationStats();
        return true;
    }
    catch (const std::exception& e) {
        handleAllocationError(e.what());
        return false;
    }
}

bool ResourceManager::allocateSpecific(int priority, int size)
{
    if (!validateRequest(priority, size)) {
        EV_WARN << "Invalid resource request: priority=" << priority << ", size=" << size << endl;
        return false;
    }

    try {
        // Find available resource blocks
        auto blocks = findAvailableBlocks(size);
        if (blocks.empty()) {
            EV_INFO << "No available blocks found for size " << size << endl;
            failedAllocations++;
            return false;
        }

        // Check for conflicts
        if (!resolveConflict(blocks)) {
            EV_WARN << "Resource conflict detected, allocation failed" << endl;
            failedAllocations++;
            return false;
        }

        // Allocate the blocks
        markBlocksOccupied(blocks, priority);
        totalAllocations++;

        // Log allocation
        logAllocation(blocks, priority);
        
        return true;
    }
    catch (const std::exception& e) {
        handleAllocationError(e.what());
        return false;
    }
}

void ResourceManager::release(int resourceId)
{
    if (!isValidResourceId(resourceId)) {
        EV_ERROR << "Invalid resource ID: " << resourceId << endl;
        return;
    }

    try {
        auto it = activeAllocations.find(resourceId);
        if (it != activeAllocations.end()) {
            // Release all blocks associated with this allocation
            for (auto block : it->second) {
                block->occupied = false;
                block->priority = 0;
                block->allocTime = 0;
            }
            activeAllocations.erase(it);
            
            EV_INFO << "Released resource ID " << resourceId << endl;
            updateUtilizationStats();
        }
    }
    catch (const std::exception& e) {
        handleAllocationError(e.what());
    }
}

bool ResourceManager::checkAvailability(int size) const
{
    if (size <= 0 || size > numSubchannels * numSymbols) {
        return false;
    }

    int availableCount = 0;
    for (const auto& block : resourcePool) {
        if (!block->occupied) {
            availableCount++;
            if (availableCount >= size) {
                return true;
            }
        }
    }
    return false;
}

double ResourceManager::getUtilization() const
{
    return currentUtilization;
}

int ResourceManager::getAvailableBlocks() const
{
    int count = 0;
    for (const auto& block : resourcePool) {
        if (!block->occupied) {
            count++;
        }
    }
    return count;
}

std::vector<int> ResourceManager::getOccupiedResources() const
{
    std::vector<int> occupied;
    for (const auto& block : resourcePool) {
        if (block->occupied) {
            occupied.push_back(block->id);
        }
    }
    return occupied;
}

void ResourceManager::setPoolSize(int subch, int symb)
{
    if (subch <= 0 || symb <= 0) {
        throw std::invalid_argument("Invalid pool size parameters");
    }
    
    numSubchannels = subch;
    numSymbols = symb;
    
    // Reinitialize the pool with new size
    clearPool();
    initialized = false;
    initializePool();
}

void ResourceManager::setPeriodicity(simtime_t period)
{
    if (period <= 0) {
        throw std::invalid_argument("Invalid periodicity value");
    }
    periodicity = period;
}

bool ResourceManager::initializePool()
{
    try {
        validatePoolConfiguration();
        
        // Create resource blocks
        int totalBlocks = numSubchannels * numSymbols;
        resourcePool.reserve(totalBlocks);
        
        for (int i = 0; i < numSubchannels; i++) {
            for (int j = 0; j < numSymbols; j++) {
                auto block = std::make_unique<ResourceBlock>();
                block->id = generateResourceId();
                block->subchannelIndex = i;
                block->symbolIndex = j;
                resourcePool.push_back(std::move(block));
            }
        }
        
        initialized = true;
        EV_INFO << "Resource pool initialized with " << totalBlocks << " blocks" << endl;
        return true;
    }
    catch (const std::exception& e) {
        handleAllocationError(e.what());
        return false;
    }
}

void ResourceManager::clearPool()
{
    resourcePool.clear();
    activeAllocations.clear();
    initialized = false;
}

bool ResourceManager::validateRequest(int priority, int size) const
{
    return (priority >= 0 && size > 0 && size <= numSubchannels * numSymbols);
}

std::vector<ResourceBlock*> ResourceManager::findAvailableBlocks(int size) const
{
    std::vector<ResourceBlock*> available;
    
    // Find consecutive free blocks
    for (const auto& block : resourcePool) {
        if (!block->occupied) {
            available.push_back(block.get());
            if (available.size() >= static_cast<size_t>(size)) {
                return available;
            }
        } else {
            available.clear();
        }
    }
    
    return std::vector<ResourceBlock*>();  // Return empty if not enough blocks found
}

void ResourceManager::markBlocksOccupied(const std::vector<ResourceBlock*>& blocks, int priority)
{
    int resourceId = generateResourceId();
    
    for (auto block : blocks) {
        block->occupied = true;
        block->priority = priority;
        block->allocTime = simTime();
    }
    
    activeAllocations[resourceId] = blocks;
}

void ResourceManager::cleanExpiredAllocations()
{
    simtime_t currentTime = simTime();
    std::vector<int> expiredIds;
    
    // Find expired allocations
    for (const auto& allocation : activeAllocations) {
        if (!allocation.second.empty()) {
            auto firstBlock = allocation.second[0];
            if (currentTime - firstBlock->allocTime >= periodicity) {
                expiredIds.push_back(allocation.first);
            }
        }
    }
    
    // Release expired allocations
    for (int id : expiredIds) {
        release(id);
    }
    
    lastCleanupTime = currentTime;
}

bool ResourceManager::resolveConflict(const std::vector<ResourceBlock*>& blocks)
{
    // Check for overlapping allocations
    for (auto block1 : blocks) {
        for (const auto& allocation : activeAllocations) {
            for (auto block2 : allocation.second) {
                if (isConflicting(block1, block2)) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool ResourceManager::isConflicting(const ResourceBlock* block1, const ResourceBlock* block2) const
{
    // Check if blocks overlap in time and frequency
    return (block1->subchannelIndex == block2->subchannelIndex &&
            block1->symbolIndex == block2->symbolIndex);
}

void ResourceManager::updateUtilizationStats()
{
    int occupiedBlocks = 0;
    int totalBlocks = resourcePool.size();
    
    for (const auto& block : resourcePool) {
        if (block->occupied) {
            occupiedBlocks++;
        }
    }
    
    currentUtilization = totalBlocks > 0 ? 
        static_cast<double>(occupiedBlocks) / totalBlocks : 0.0;
}

void ResourceManager::logAllocation(const std::vector<ResourceBlock*>& blocks, int priority)
{
    EV_INFO << "Allocated " << blocks.size() << " blocks with priority " << priority << endl;
    logResourceStatus();
}

int ResourceManager::generateResourceId() const
{
    // Simple implementation - could be made more sophisticated
    static int nextId = 1;
    return nextId++;
}

bool ResourceManager::isValidResourceId(int id) const
{
    return id > 0 && activeAllocations.find(id) != activeAllocations.end();
}

void ResourceManager::validatePoolConfiguration() const
{
    if (numSubchannels <= 0 || numSymbols <= 0) {
        throw std::runtime_error("Invalid resource pool configuration");
    }
}

void ResourceManager::handleAllocationError(const char* message)
{
    EV_ERROR << "Resource allocation error: " << message << endl;
    failedAllocations++;
}

void ResourceManager::logResourceStatus() const
{
    EV_INFO << "Resource pool status:" << endl
            << "  Total blocks: " << resourcePool.size() << endl
            << "  Available blocks: " << getAvailableBlocks() << endl
            << "  Current utilization: " << currentUtilization << endl
            << "  Total allocations: " << totalAllocations << endl
            << "  Failed allocations: " << failedAllocations << endl;
}

}  // namespace nr