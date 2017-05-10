#pragma once
#include "Enums.hpp"
#include "IInformationProvider.hpp"
#include <string>

// Property Name
#define STORAGE_SIZE "StorageSize"
#define POWER_SOURCE "PowerSource"

namespace Microsoft { namespace Applications { namespace Telemetry {
namespace PAL {

class IDeviceInformation : public IInformationProvider
{
public:
    /// <summary>
    /// Gets the unique ID of the current device
    /// </summary>
    /// <returns>The unique ID of the current device</returns>
    virtual std::string const& GetDeviceId() const = 0;

    /// <summary>
    /// Gets the manufacturer of the current device
    /// </summary>
    /// <returns>The manufacturer of the current device</returns>
    virtual std::string const& GetManufacturer() const = 0;

    /// <summary>
    /// Gets the model of the current device
    /// </summary>
    /// <returns>The model of the current device</returns>
    virtual std::string const& GetModel() const = 0;

    /// <summary>
    /// The OS achitecture type, such as "x86" or "x64".
    /// </summary>
    /// <returns>OS architecture</returns>
    virtual OsArchitectureType GetOsArchitectureType() const = 0;

    /// <summary>
    /// Gets cpu make
    /// </summary>
    /// <returns>The cpu make</returns>
    virtual std::string const& GetCpuMake() const = 0;

    /// <summary>
    /// Gets cpu model
    /// </summary>
    /// <returns>The cpu model</returns>
    virtual std::string const& GetCpuModel() const = 0;

    /// <summary>
    /// Gets memory size
    /// </summary>
    /// <returns>The memory size in MB (mebibytes)</returns>
    virtual size_t GetMemorySize() const = 0;

    /// <summary>
    /// Gets storage size
    /// </summary>
    /// <returns>The storage size in MB (mebibytes)</returns>
    virtual size_t GetStorageSize() const = 0;

    /// <summary>
    /// Gets screen count
    /// </summary>
    /// <returns>The screen count</returns>
    virtual size_t GetScreenCount()const = 0;

    /// <summary>
    /// Gets screen width in pixels
    /// </summary>
    /// <returns> Screen width in pixels</returns>
    virtual size_t GetScreenWidthInPixels() const = 0;

    /// <summary>
    /// Gets screen width DPI
    /// </summary>
    /// <returns>Screen width DPI</returns>
    virtual double GetScreenWidthDPI() const = 0;

    /// <summary>
    /// Gets screen width in inches
    /// </summary>
    /// <returns>Screen width in inches</returns>
    virtual double GetScreenWidthInInches() const = 0;

    /// <summary>
    /// Gets screen height in pixels
    /// </summary>
    /// <returns>Screen height in pixels</returns>
    virtual size_t GetScreenHeightInPixels() const = 0;

    /// <summary>
    /// Gets screen height DPI
    /// </summary>
    /// <returns>Screen height DPI</returns>
    virtual double GetScreenHeightDPI() const = 0;

    /// <summary>
    /// Gets screen height in inches
    /// </summary>
    /// <returns>Screen height in inches</returns>
    virtual double GetScreenHeightInInches() const = 0;

    /// <summary>
    /// Gets availability of the mouse
    /// </summary>
    /// <returns>Mouse availability</returns>
    virtual bool IsMouseAvailable() const = 0;

    /// <summary>
    /// Gets availability of the mouse
    /// </summary>
    /// <returns>Keyboard availability</returns>
    virtual bool IsKeyboardAvailable() const = 0;

    /// <summary>
    /// Gets availability of the Digitizer
    /// </summary>
    /// <returns>Digitizer availability</returns>
    virtual bool IsDigitizerAvailable() const = 0;

    /// <summary>
    /// Gets touch availability
    /// </summary>
    /// <returns>Touch availability</returns>
    virtual bool IsTouchAvailable() const = 0;

    /// <summary>
    /// Gets availability of the front camera
    /// </summary>
    /// <returns>Front camera availability</returns>
    virtual bool IsFrontCameraAvailable() const = 0;

    /// <summary>
    /// Gets availability of the rear camera
    /// </summary>
    /// <returns>Rear camera availability</returns>
    virtual bool IsRearCameraAvailable() const = 0;

    /// <summary>
    /// Gets the power source the device is currently using.
    /// </summary>
    /// <returns>Source of power the device is using</returns>
    virtual PowerSource GetPowerSource() const = 0;
};

} // PAL
}}}
