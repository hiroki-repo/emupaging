// dllmain.cpp : DLL アプリケーションのエントリ ポイントを定義します。
#include "pch.h"

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

UINT8* directmem = nullptr;

UINT32 list4kpages[1048576];

bool ismmio4kpageswr[1048576];
bool ismmio4kpagesrd[1048576];

typedef UINT8 typeofmemread(int);
typedef void typeofmemwrite(int, int);
typedef UINT8 typeofioread(int);
typedef void typeofiowrite(int, int);

UINT8(*memread[1048576])(int);
void(*memwrite[1048576])(int,int);

UINT8(*ioread[65536])(int);
void(*iowrite[65536])(int, int);

UINT32 addressmask = ~0;

extern "C" __declspec(dllexport) void* getptr4debug(int prm_0) {
	switch (prm_0) {
	case 0:
		return &list4kpages;
		break;
	case 1:
		return &ismmio4kpageswr;
		break;
	case 2:
		return &ismmio4kpagesrd;
		break;
	case 3:
		return &memread;
		break;
	case 4:
		return &memwrite;
		break;
	case 5:
		return &ioread;
		break;
	case 6:
		return &iowrite;
		break;
	}
}

extern "C" __declspec(dllexport) void setaddrmask(UINT32 prm_0) { addressmask = prm_0; }
extern "C" __declspec(dllexport) UINT32 getaddrmask(void) { return addressmask; }

extern "C" __declspec(dllexport) void setaddrspace(UINT32 prm_0, void* prm_1,int prm_2) {
	if (prm_2 & 4) {
		switch (prm_2&3) {
		case 0:
			memwrite[(prm_0 >> 12)] = (typeofmemwrite*)prm_1;
			ismmio4kpageswr[(prm_0 >> 12)] = true;
			break;
		case 1:
			memread[(prm_0 >> 12)] = (typeofmemread*)prm_1;
			ismmio4kpagesrd[(prm_0 >> 12)] = true;
			break;
		case 2:
			iowrite[(prm_0 >> 12)] = (typeofiowrite*)prm_1;
			break;
		case 3:
			ioread[(prm_0 >> 12)] = (typeofioread*)prm_1;
			break;
		}
	}
	else {
		list4kpages[(prm_0 >> 12)] = (UINT32)(prm_1);
		ismmio4kpagesrd[(prm_0 >> 12)] = false;
		ismmio4kpageswr[(prm_0 >> 12)] = false;
	}
}

extern "C" __declspec(dllexport) UINT32 cpumemaccess(int prm_0, int prm_1, int prm_2) {
	UINT32 addr4mac = ((UINT32)prm_0) & addressmask;
	switch (prm_2 & 3) {
	case 0:
		if (ismmio4kpageswr[(addr4mac >> 12)] == true) {
			if (memwrite[(addr4mac >> 12)] != nullptr) { (memwrite[(addr4mac >> 12)])(addr4mac, prm_1); }
			return 0;
		}
		else {
			directmem[list4kpages[(addr4mac >> 12)] + (addr4mac & 0xFFF)] = prm_1;
		}
		return 0;
		break;
	case 1:
		if (ismmio4kpagesrd[(addr4mac >> 12)] == true) {
			if (memread[(addr4mac >> 12)] != nullptr) { return (memread[(addr4mac >> 12)])(addr4mac); }
			return 0;
		}
		else {
			return directmem[list4kpages[(addr4mac >> 12)] + (addr4mac & 0xFFF)];
		}
		break;
	case 2:
		if (iowrite[prm_0] != nullptr) { (iowrite[prm_0])(prm_0,prm_1); }
		return 0;
		break;
	case 3:
		if (ioread[prm_0]!=nullptr){ return (ioread[prm_0])(prm_0); }
		return 0;
		break;
	}
	return 0;
}