// used: [d3d] api
#include <d3d11.h>

#include "interfaces.h"

// used: findpattern, callvirtual, getvfunc...
#include "memory.h"
#include "csgoinput.h"
#include "cgameresourceservice.h"
#include "iengineclient.h"
#include "crt.h"
#include "offset.hpp"

#pragma region interfaces_get
extern uintptr_t client_address;
using InstantiateInterfaceFn_t = void* (*)();

class CInterfaceRegister
{
public:
	InstantiateInterfaceFn_t fnCreate;
	const char* szName;
	CInterfaceRegister* pNext;
};

static const CInterfaceRegister* GetRegisterList(const wchar_t* wszModuleName)
{
	void* hModule = MEM::GetModuleBaseHandle(wszModuleName);
	if (hModule == nullptr)
		return nullptr;

	std::uint8_t* pCreateInterface = reinterpret_cast<std::uint8_t*>(MEM::GetExportAddress(hModule, CS_XOR("CreateInterface")));

	if (pCreateInterface == nullptr)
	{
		
		return nullptr;
	}

	return *reinterpret_cast<CInterfaceRegister**>(MEM::ResolveRelativeAddress(pCreateInterface, 0x3, 0x7));
}

template <typename T = void*>
T* Capture(const CInterfaceRegister* pModuleRegister, const char* szInterfaceName)
{
	for (const CInterfaceRegister* pRegister = pModuleRegister; pRegister != nullptr; pRegister = pRegister->pNext)
	{
		if (const std::size_t nInterfaceNameLength = CRT::StringLength(szInterfaceName);
			// found needed interface
			CRT::StringCompareN(szInterfaceName, pRegister->szName, nInterfaceNameLength) == 0 &&
			// and we've given full name with hardcoded digits
			(CRT::StringLength(pRegister->szName) == nInterfaceNameLength ||
				// or it contains digits after name
				CRT::StringToInteger<int>(pRegister->szName + nInterfaceNameLength, nullptr, 10) > 0))
		{
			// capture our interface
			void* pInterface = pRegister->fnCreate();



			return static_cast<T*>(pInterface);
		}
	}

	
	return nullptr;
}

#pragma endregion

bool I::Setup()
{
	bool bSuccess = true;
	 


	const auto pEngineRegisterList = GetRegisterList(ENGINE2_DLL);
	if (pEngineRegisterList == nullptr)
		return false;

	GameResourceService = Capture<IGameResourceService>(pEngineRegisterList, GAME_RESOURCE_SERVICE_CLIENT);
	bSuccess &= (GameResourceService != nullptr);

	Engine = Capture<IEngineClient>(pEngineRegisterList, SOURCE2_ENGINE_TO_CLIENT);
	bSuccess &= (Engine != nullptr);

	//Input = *reinterpret_cast<CCSGOInput**>(client_address + cs2_dumper::offsets::client_dll::dwCSGOInput);

	Input = *reinterpret_cast<CCSGOInput**>(MEM::ResolveRelativeAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("48 8B 0D ? ? ? ? E8 ? ? ? ? 8B BE 84 12 00 00")), 0x3, 0x7));
	bSuccess &= (Input != nullptr);


	GameTraceManager = *reinterpret_cast<CGameTraceManager**>(MEM::GetAbsoluteAddress(MEM::FindPattern(CLIENT_DLL, CS_XOR("4C 8B 3D ? ? ? ? 24 C9 0C 49 66 0F 7F 45")), 0x3, 0x0));
	bSuccess &= (GameTraceManager != nullptr);




	return bSuccess;
}


