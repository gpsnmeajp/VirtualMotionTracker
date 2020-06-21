#include "VRWatchdogProvider.h"
namespace VMTDriver {
    EVRInitError VRWatchdogProvider::Init(IVRDriverContext* pDriverContext)
    {
        VR_INIT_WATCHDOG_DRIVER_CONTEXT(pDriverContext)
        return EVRInitError::VRInitError_None;
    }
    void VRWatchdogProvider::Cleanup()
    {
        VR_CLEANUP_WATCHDOG_DRIVER_CONTEXT()
    }
}