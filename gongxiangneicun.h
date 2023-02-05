#pragma once
#include "neicun.h"
#include "xitong.h"
namespace GXNC {

	BOOLEAN DGXNC(PVOID Address, PVOID Buffer, SIZE_T Size) {
		SIZE_T Bytes{ 0 };

		if (NT_SUCCESS(MmCopyVirtualMemory(gProcess, Address, IoGetCurrentProcess(), Buffer, Size, KernelMode, &Bytes))) {
			return TRUE;
		} return FALSE;
	}

	template <typename T>
	BOOLEAN XGXNC(PVOID Address, T Buffer, SIZE_T Size = sizeof(T)) {
		SIZE_T Bytes{ 0 };

		if (NT_SUCCESS(MmCopyVirtualMemory(IoGetCurrentProcess(), (PVOID)&Buffer, gProcess, Address, Size, KernelMode, &Bytes))) {
			return TRUE;
		} return FALSE;
	}

	BYTE GetStatus() {
		BYTE CurStatus{ 0 };
		DGXNC(gData.pStatus, &CurStatus, sizeof(SHORT));
		return CurStatus;
	}

	DWORD GetCode() {
		DWORD CurCode{ 0 };
		DGXNC(gData.pCode, &CurCode, sizeof(DWORD));
		return CurCode;
	}

	OperationData GetBuffer() {
		OperationData CurBuffer{ 0 };
		DGXNC(gData.SharedMemory, &CurBuffer, sizeof(OperationData));
		return CurBuffer;
	}

	BOOLEAN SetStatus(Status DesiredStatus) {
		return XGXNC<SHORT>(gData.pStatus, DesiredStatus);
	}

	BOOLEAN SetCode() {
		return XGXNC<DWORD>(gData.pCode, Complete);
	}

	BOOLEAN SetBuffer(OperationData Buffer) {
		return XGXNC<OperationData>(gData.SharedMemory, Buffer);
	}

	VOID Respond() {
		DWORD Code{ GetCode() };
		OperationData Params{ GetBuffer() };

		switch (Code) {

		case BaseRequest: {
			Process::GetBaseAddress(&Params);
			SetBuffer(Params);
			SetCode();
			SetStatus(Active);
		} break;

		case SizeRequest: {
			Process::GetMainModuleSize(&Params);
			SetBuffer(Params);
			SetCode();
			SetStatus(Active);
		} break;

		case PebRequest: {
			Process::GetPeb(&Params);
			SetBuffer(Params);
			SetCode();
			SetStatus(Active);
		} break;

		case QIPRequest: {
			Process::QueryInformation(&Params);
			SetBuffer(Params);
			SetCode();
			SetStatus(Active);
		} break;

		case CopyRequest: {
			Memory::CopyVirtualMemory(&Params);
			SetBuffer(Params);
			SetCode();
			SetStatus(Active);
		} break;

		//case AVMRequest: {
		//	Memory::AllocateVirtualMemory(&Params);
		//	SetBuffer(Params);
		//	SetCode();
		//	SetStatus(Active);
		//} break;

		//case FVMRequest: {
		//	Memory::FreeVirtualMemory(&Params);
		//	SetBuffer(Params);
		//	SetCode();
		//	SetStatus(Active);
		//} break;

		//case PVMRequest: {
		//	Memory::ProtectVirtualMemory(&Params);
		//	SetBuffer(Params);
		//	SetCode();
		//	SetStatus(Active);
		//} break;

		//case QVMRequest: {
		//	Memory::QueryVirtualMemory(&Params);
		//	SetBuffer(Params);
		//	SetCode();
		//	SetStatus(Active);
		//} break;

		case ModuleRequest: {
			Process::GetModuleInfo(&Params);
			SetBuffer(Params);
			SetCode();
			SetStatus(Active);
		} break;

		case IndexRequest: {
			Process::GetModuleInfoByIndex(&Params);
			SetBuffer(Params);
			SetCode();
			SetStatus(Active);
		} break;

		default: {
		} break;
		}
	}

	VOID Loop() {
		gProcess = Process::GetProcess(gData.ProcessId);

		if (gProcess == nullptr) {
			return;
		}

		for (;;) {

			if (*(DWORD*)((BYTE*)gProcess + ActiveThreadsOffset) == 1) {
				// We're the only active thread - the client must be trying to terminate
				ObfDereferenceObject(gProcess);
				return;
			}

			DWORD Status{ GetStatus() };

			switch (Status) {
			case Inactive: {
				Utils::Sleep(50);
			} break;

			case Active: {
				Utils::Sleep(1);
			} break;

			case Waiting: {
				Respond();
			} break;

			case Exit: {
				SetStatus(Inactive);
				ObfDereferenceObject(gProcess);
				return;
			} break;

			default: {
				Utils::Sleep(50);
			} break;
			}
		}
	}
}