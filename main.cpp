#include "qianzhi.h"

NTSTATUS DriverEntry(DRIVER_OBJECT* DriverObject, UNICODE_STRING* RegistryPath) {
	return Siji::ChuShiHua();
}