#include "Bone.h"
#include "client.dll.hpp"
#include "tool.h"
bool CBone::UpdateAllBoneData(const uintptr_t& EntityPawnAddress)
{
	using namespace cs2_dumper::schemas::client_dll;
	if (EntityPawnAddress == 0) 
		return false;
	this->EntityPawnAddress = EntityPawnAddress;

	uintptr_t GameSceneNode = 0;
	uintptr_t BoneArrayAddress = 0;

	GameSceneNode = *reinterpret_cast<uintptr_t*>(EntityPawnAddress + C_BaseEntity::m_pGameSceneNode);
	if (!GameSceneNode)return false;

	BoneArrayAddress = *reinterpret_cast<uintptr_t*>(GameSceneNode + CSkeletonInstance::m_modelState + 0x80);
	if (!BoneArrayAddress)return false;

	BoneJointData BoneArray[30]{};
	BoneJointData* pBoneData = (BoneJointData*)BoneArrayAddress;
	for (int i = 0; i < 30; i++)
	{
		BoneArray[i] = pBoneData[i];
	}

	for (int i = 0; i < 30; i++)
	{
		Vector2D_t ScreenPos;
		bool IsVisible = false;

		//if (WorldToScreen(BoneArray[i].Pos, ScreenPos))
		//	IsVisible = true;

		this->BonePosList.push_back({ BoneArray[i].Pos ,ScreenPos,IsVisible });
	}

	return this->BonePosList.size() > 0;
}

bool CBone::GetBoneData(const uintptr_t& EntityPawnAddress,int boneId, Vector_t& vec_pos)
{
	using namespace cs2_dumper::schemas::client_dll;
	if (EntityPawnAddress == 0)
		return false;
	this->EntityPawnAddress = EntityPawnAddress;

	uintptr_t GameSceneNode = 0;
	uintptr_t BoneArrayAddress = 0;

	GameSceneNode = *reinterpret_cast<uintptr_t*>(EntityPawnAddress + C_BaseEntity::m_pGameSceneNode);
	if (!GameSceneNode)return false;

	BoneArrayAddress = *reinterpret_cast<uintptr_t*>(GameSceneNode + CSkeletonInstance::m_modelState + 0x80);
	if (!BoneArrayAddress)return false;

	
	BoneJointData* pBoneData = (BoneJointData*)BoneArrayAddress;
	vec_pos = pBoneData[boneId].Pos;

}