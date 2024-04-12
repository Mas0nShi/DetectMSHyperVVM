#include <Windows.h>
#include <iostream>
#if _MSC_VER >= 1400    // VC2005
#include <intrin.h>    // __cpuid, __cpuidex
#else
#error Only Visual Studio 2005 and later are supported
#endif
#include "hvgdk.h"

bool HviIsAnyHypervisorPresent() {
    HV_CPUID_RESULT hvCpuIdResult = {};


    __cpuid(reinterpret_cast<int*>(&hvCpuIdResult), HvCpuIdFunctionVersionAndFeatures);

    if (!hvCpuIdResult.VersionAndFeatures.HypervisorPresent) {
        return false;
    }

    // 
    memset(&hvCpuIdResult, 0, sizeof(hvCpuIdResult));

    __cpuid(reinterpret_cast<int*>(&hvCpuIdResult), HvCpuIdFunctionHvInterface);
    
    return hvCpuIdResult.HvInterface.Interface != HvMicrosoftXboxNanovisor;
}

bool HviIsHypervisorVendorMicrosoft() {
    if (!HviIsAnyHypervisorPresent()) {
        return false;
    }

    HV_CPUID_RESULT hvCpuIdResult = {};
    __cpuid(reinterpret_cast<int*>(&hvCpuIdResult), HvCpuIdFunctionHvVendorAndMaxFunction);

    // Check if the vendor ID matches Microsoft's "Microsoft Hv"
    PUINT32 pdwVendorId = reinterpret_cast<PUINT32>(&(hvCpuIdResult.HvVendorAndMaxFunction.VendorName));

    return pdwVendorId[0] == HV_CPUID_HV_VENDOR_MICROSOFT_EBX &&
		pdwVendorId[1] == HV_CPUID_HV_VENDOR_MICROSOFT_ECX &&
		pdwVendorId[2] == HV_CPUID_HV_VENDOR_MICROSOFT_EDX;
}

bool HviIsHypervisorMicrosoftCompatible() {
    if (!HviIsAnyHypervisorPresent()) {
        return false;
    }

    HV_CPUID_RESULT hvCpuIdResult = {};

    __cpuid(reinterpret_cast<int*>(&hvCpuIdResult), HvCpuIdFunctionHvInterface);

    return hvCpuIdResult.HvInterface.Interface == HvMicrosoftHypervisorInterface;
}

// HvCpuIdFunctionMsHvFeatures
bool HviIsHypervisorFeaturePresent() {
    if (!HviIsAnyHypervisorPresent()) {
	return false;
}

	HV_CPUID_RESULT hvCpuIdResult = {};

	__cpuid(reinterpret_cast<int*>(&hvCpuIdResult), HvCpuIdFunctionMsHvFeatures);

	return (hvCpuIdResult.MsHvFeatures.PartitionPrivileges.Debugging == 0);
}

bool IsRunningInMicrosoftHyperVVM() {
    bool bIsAnyHypervisorPresent = HviIsAnyHypervisorPresent();
    bool bIsHypervisorVendorMicrosoft = HviIsHypervisorVendorMicrosoft();
    bool bIsHypervisorFeaturePresent = false;
    if (HviIsHypervisorMicrosoftCompatible()) bIsHypervisorFeaturePresent = HviIsHypervisorFeaturePresent();

    return bIsAnyHypervisorPresent && bIsHypervisorVendorMicrosoft && bIsHypervisorFeaturePresent;
}

int main()
{
    if (IsRunningInMicrosoftHyperVVM()) {
		std::cout << "[+] Running in Microsoft Hyper-V VM" << std::endl;
    }
    else {
        std::cout << "[-] Not running in Microsoft Hyper-V VM" << std::endl;
    }
}
