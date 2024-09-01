#pragma once
#include <cstdint>
#include "common.h"
#include "vector.h"
#include "memory.h"
#include "offset.h"
#include "Bone.h"
#include "client.dll.hpp"
#include "qangle.h"
#include <string>
#define INVALID_EHANDLE_INDEX 0xFFFFFFFF
#define ENT_ENTRY_MASK 0x7FFF
#define NUM_SERIAL_NUM_SHIFT_BITS 15
// @source: https://developer.valvesoftware.com/wiki/Entity_limit#Source_2_limits
#define ENT_MAX_NETWORKED_ENTRY 16384

class CEntityInstance;

class CEntityIdentity
{
public:
	CS_CLASS_NO_INITIALIZER(CEntityIdentity);

	// @note: handle index is not entity index
	inline std::uint32_t GetIndex()
	{
		return *reinterpret_cast<uint32_t*>((uintptr_t)this + 0x10);
	}
	inline std::uint32_t GetFlags()
	{
		return *reinterpret_cast<uint32_t*>((uintptr_t)this + cs2_dumper::schemas::client_dll::CEntityIdentity::m_flags);
	}

	[[nodiscard]] bool IsValid()
	{
		return GetIndex() != INVALID_EHANDLE_INDEX;
	}

	[[nodiscard]] int GetEntryIndex()
	{
		if (!IsValid())
			return ENT_ENTRY_MASK;

		return GetIndex() & ENT_ENTRY_MASK;
	}

	[[nodiscard]] int GetSerialNumber()
	{
		return GetIndex() >> NUM_SERIAL_NUM_SHIFT_BITS;
	}

	CEntityInstance* pInstance; // 0x00
};

//class CEntityInstance
//{
//public:
//	CS_CLASS_NO_INITIALIZER(CEntityInstance);
//
//	void GetSchemaClassInfo(void* pReturn)
//	{
//		return MEM::CallVFunc<void, 38U>(this, pReturn);
//	}
//	inline CEntityIdentity* GetIdentity()
//	{
//		*reinterpret_cast<CEntityIdentity**>((uintptr_t)this + cs2_dumper::schemas::client_dll::CEntityInstance::m_pEntity);
//	}
//	inline CBaseHandle GetRefEHandle()
//	{
//		CEntityIdentity* pIdentity = GetIdentity();
//		if (pIdentity == nullptr)
//			return CBaseHandle();
//
//		return CBaseHandle(pIdentity->GetEntryIndex(), pIdentity->GetSerialNumber() - (pIdentity->GetFlags() & 1));
//	}
//
//};

class CCollisionProperty
{
public:
	std::uint16_t CollisionMask()
	{
		return *reinterpret_cast<std::uint16_t*>(reinterpret_cast<std::uintptr_t>(this) + 0x38);
	}

	CS_CLASS_NO_INITIALIZER(CCollisionProperty);


	std::uint8_t GetSolidFlags()
	{
		return *reinterpret_cast<uint8_t*>((uintptr_t)this + cs2_dumper::schemas::client_dll::CCollisionProperty::m_usSolidFlags);
	}
	
};

class CBaseHandle
{
public:
	CBaseHandle() noexcept :
		nIndex(INVALID_EHANDLE_INDEX) { }

	CBaseHandle(const int nEntry, const int nSerial) noexcept
	{
		CS_ASSERT(nEntry >= 0 && (nEntry & ENT_ENTRY_MASK) == nEntry);
		CS_ASSERT(nSerial >= 0 && nSerial < (1 << NUM_SERIAL_NUM_SHIFT_BITS));

		nIndex = nEntry | (nSerial << NUM_SERIAL_NUM_SHIFT_BITS);
	}

	bool operator!=(const CBaseHandle& other) const noexcept
	{
		return nIndex != other.nIndex;
	}

	bool operator==(const CBaseHandle& other) const noexcept
	{
		return nIndex == other.nIndex;
	}

	bool operator<(const CBaseHandle& other) const noexcept
	{
		return nIndex < other.nIndex;
	}

	[[nodiscard]] bool IsValid() const noexcept
	{
		return nIndex != INVALID_EHANDLE_INDEX;
	}

	[[nodiscard]] int GetEntryIndex() const noexcept
	{
		return static_cast<int>(nIndex & ENT_ENTRY_MASK);
	}

	[[nodiscard]] int GetSerialNumber() const noexcept
	{
		return static_cast<int>(nIndex >> NUM_SERIAL_NUM_SHIFT_BITS);
	}

private:
	std::uint32_t nIndex;
};





class CEntityInstance
{
public:
	CS_CLASS_NO_INITIALIZER(CEntityInstance);

	void GetSchemaClassInfo(void* pReturn)
	{
		return MEM::CallVFunc<void, 38U>(this, pReturn);
	}
	inline CEntityIdentity* GetIdentity()
	{
		return *reinterpret_cast<CEntityIdentity**>((uintptr_t)this + cs2_dumper::schemas::client_dll::CEntityInstance::m_pEntity);
	}
	inline CBaseHandle GetRefEHandle()
	{
		CEntityIdentity* pIdentity = GetIdentity();
		if (pIdentity == nullptr)
			return CBaseHandle();

		return CBaseHandle(pIdentity->GetEntryIndex(), pIdentity->GetSerialNumber() - (pIdentity->GetFlags() & 1));
	}

};



template <typename T>
class CUtlVector {
	public:
		auto begin() const { return m_data; }
		auto end() const { return m_data + m_size; }

		auto At(int i) const { return m_data[i]; }
		auto AtPtr(int i) const { return m_data + i; }

		bool Exists(T val) const {
			for (const auto& it : *this)
				if (it == val) return true;
			return false;
		}
		bool IsEmpty() const { return m_size == 0; }

		int m_size;
		char pad0[0x4];  // no idea
		T* m_data;
		char pad1[0x8];  // no idea
};


class C_CSPlayerPawn
{
public:
	C_CSPlayerPawn(std::uintptr_t base) :pthis(base) {}
	C_CSPlayerPawn() :pthis(0) {}
	std::uint32_t GetOwnerHandleIndex();
	std::uint16_t GetCollisionMask();
	std::uintptr_t pthis;
	CBone BoneData;
	[[nodiscard]] Vector_t GetEyePosition()
	{
		Vector_t vecEyePosition = Vector_t(0.0f, 0.0f, 0.0f);
		MEM::CallVFunc<void, 166U>((C_CSPlayerPawn*)pthis, &vecEyePosition);
		return vecEyePosition;
	}
	inline Vector_t GetPos()
	{
		const std::uintptr_t m_vOldOrigin = 0x1274;
		return *reinterpret_cast<Vector_t*>(pthis + m_vOldOrigin);
	}
	inline int GetHealth()
	{
		using namespace cs2_dumper::schemas::client_dll;
		const uintptr_t m_iHealth = 0x324;
		return *reinterpret_cast<int*>(pthis + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iHealth);
	}
	inline Vector_t GetViewAngle()
	{
		using namespace cs2_dumper::schemas::client_dll;

		return *reinterpret_cast<Vector_t*>(pthis + C_CSPlayerPawnBase::m_angEyeAngles);
	}

	inline Vector_t GetCameraPos()
	{
		using namespace cs2_dumper::schemas::client_dll;
		return *reinterpret_cast<Vector_t*>(pthis + C_CSPlayerPawnBase::m_vecLastClipCameraPos);
	}

	inline unsigned long GetShotsFired()
	{
		using namespace cs2_dumper::schemas::client_dll;
		return *reinterpret_cast<unsigned long*>(pthis + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_iShotsFired);

	}

	inline uint8_t GetTeamId()
	{
		
		return *reinterpret_cast<uint8_t*>(pthis + cs2_dumper::schemas::client_dll::C_BaseEntity::m_iTeamNum);
	}


	inline bool update_bone_data()
	{
		return BoneData.UpdateAllBoneData(pthis);

	}
	inline Vector_t get_bone_data(int bone_id, Vector_t& vec_pos)
	{
		return BoneData.GetBoneData(pthis, bone_id, vec_pos);
	}

	inline QAngle_t GetAimPunchAngle()
	{
		return *reinterpret_cast<QAngle_t*>(pthis + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_aimPunchAngle);
	}
	inline CUtlVector<QAngle_t> GetAimPunchCache()
	{
		return *reinterpret_cast<CUtlVector<QAngle_t>*>(pthis + cs2_dumper::schemas::client_dll::C_CSPlayerPawn::m_aimPunchCache);
	}
	
	inline CCollisionProperty* GetCollision()
	{
		return *reinterpret_cast<CCollisionProperty**>((uintptr_t)pthis + cs2_dumper::schemas::client_dll::C_BaseEntity::m_pCollision);
	}
	inline CBaseHandle  GetOwnerHandle()
	{
		return *reinterpret_cast<CBaseHandle*>((uintptr_t)pthis + cs2_dumper::schemas::client_dll::C_BaseEntity::m_hOwnerEntity);
	}
	inline CBaseHandle GetRefEHandle_()
	{
		return ((CEntityInstance*)pthis)->GetRefEHandle();
	}
	

};

class CCSPlayerController
{
public:
	CCSPlayerController(uintptr_t base)
	{
		this->Address = base;
	}
	CCSPlayerController()
	{
		this->Address = 0;
	}
	CCSPlayerController(const CCSPlayerController& ref)
	{
		this->Address = ref.Address;
	}
	uintptr_t Address = 0;

	uintptr_t Pawn = 0;
public:
	int GetTeamID();
	int GetHealth();
	int GetIsAlive();
	inline CBaseHandle GetPawnHandle()
	{
		using namespace cs2_dumper::schemas::client_dll;
		return *reinterpret_cast<CBaseHandle*>(Address + cs2_dumper::schemas::client_dll::CBasePlayerController::m_hPawn);
	}
};
