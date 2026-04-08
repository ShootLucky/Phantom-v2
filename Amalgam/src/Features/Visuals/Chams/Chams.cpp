#include "Chams.h"

#include "../Groups/Groups.h"
#include "../Materials/Materials.h"
#include "../FakeAngle/FakeAngle.h"
#include "../../Backtrack/Backtrack.h"

void CChams::Begin()
{
	m_tOriginalColor = I::RenderView->GetColorModulation();
	m_flOriginalBlend = I::RenderView->GetBlend();
	I::ModelRender->GetMaterialOverride(&m_pOriginalMaterial, &m_iOriginalOverride);
}
void CChams::End()
{
	I::RenderView->SetColorModulation(m_tOriginalColor);
	I::RenderView->SetBlend(m_flOriginalBlend);
	I::ModelRender->ForcedMaterialOverride(m_pOriginalMaterial, m_iOriginalOverride);
}

void CChams::DrawModel(CBaseEntity* pEntity, Chams_t& tChams, IMatRenderContext* pRenderContext, bool bTwoModels)
{
	const auto& vVisibleMaterials = !tChams.Visible.empty() ? tChams.Visible : std::vector<std::pair<std::string, Color_t>>{ { "None", {} } };
	const auto& vOccludedMaterials = !tChams.Occluded.empty() ? tChams.Occluded : std::vector<std::pair<std::string, Color_t>>{ { "None", {} } };

	Begin();
	if (bTwoModels)
	{
		pRenderContext->SetStencilEnable(true);

		pRenderContext->ClearBuffers(false, false, false);
		pRenderContext->SetStencilCompareFunction(STENCILCOMPARISONFUNCTION_ALWAYS);
		pRenderContext->SetStencilPassOperation(STENCILOPERATION_REPLACE);
		pRenderContext->SetStencilFailOperation(STENCILOPERATION_KEEP);
		pRenderContext->SetStencilZFailOperation(STENCILOPERATION_KEEP);
		pRenderContext->SetStencilReferenceValue(1);
		pRenderContext->SetStencilWriteMask(0xFF);
		pRenderContext->SetStencilTestMask(0x0);
	}
	for (auto& [sName, tColor] : vVisibleMaterials)
	{
		auto pMaterial = F::Materials.GetMaterial(FNV1A::Hash32(sName.c_str()));

		F::Materials.SetColor(pMaterial, tColor);
		I::ModelRender->ForcedMaterialOverride(pMaterial ? pMaterial->m_pMaterial : nullptr);
		if (pMaterial && pMaterial->m_bInvertCull)
			pRenderContext->CullMode(MATERIAL_CULLMODE_CW);

		m_bRendering = true;
		pEntity->DrawModel(STUDIO_RENDER);
		m_bRendering = false;

		if (pMaterial && pMaterial->m_bInvertCull)
			pRenderContext->CullMode(MATERIAL_CULLMODE_CCW);
	}
	if (bTwoModels)
	{
		pRenderContext->ClearBuffers(false, false, false);
		pRenderContext->SetStencilCompareFunction(STENCILCOMPARISONFUNCTION_EQUAL);
		pRenderContext->SetStencilPassOperation(STENCILOPERATION_KEEP);
		pRenderContext->SetStencilFailOperation(STENCILOPERATION_KEEP);
		pRenderContext->SetStencilZFailOperation(STENCILOPERATION_KEEP);
		pRenderContext->SetStencilReferenceValue(0);
		pRenderContext->SetStencilWriteMask(0x0);
		pRenderContext->SetStencilTestMask(0xFF);
		pRenderContext->DepthRange(0.f, 0.2f);

		for (auto& [sName, tColor] : vOccludedMaterials)
		{
			auto pMaterial = F::Materials.GetMaterial(FNV1A::Hash32(sName.c_str()));

			F::Materials.SetColor(pMaterial, tColor);
			I::ModelRender->ForcedMaterialOverride(pMaterial ? pMaterial->m_pMaterial : nullptr);
			if (pMaterial && pMaterial->m_bInvertCull)
				pRenderContext->CullMode(MATERIAL_CULLMODE_CW);

			m_bRendering = true;
			pEntity->DrawModel(STUDIO_RENDER);
			m_bRendering = false;

			if (pMaterial && pMaterial->m_bInvertCull)
				pRenderContext->CullMode(MATERIAL_CULLMODE_CCW);
		}

		pRenderContext->SetStencilEnable(false);
		pRenderContext->DepthRange(0.f, 1.f);

		m_mEntities[pEntity->entindex()];
	}
	End();
}



// Converte índice do enum de modo de chams para nome de material do Amalgam
static inline const char* ChamsModeToMaterial(int iMode)
{
	switch (iMode)
	{
	case 1: return "Flat";
	case 2: return "Wireframe";
	case 3: return "Shaded";
	case 4: return "Glow";
	default: return nullptr;
	}
}

void CChams::Store(CTFPlayer* pLocal)
{
	m_vEntities.clear();
	if (!pLocal)
		return;

	// ── Fallback simples quando Groups não estão configurados ──────────────
	if (!F::Groups.GroupsActive())
	{
		int iEnemyMode = Vars::Visuals::Chams::EnemyEnabled.Value;
		int iTeamMode = Vars::Visuals::Chams::TeamEnabled.Value;

		if (!iEnemyMode && !iTeamMode)
			return;

		for (int i = 1; i <= I::ClientEntityList->GetHighestEntityIndex(); ++i)
		{
			auto pEntity = I::ClientEntityList->GetClientEntity(i)->As<CTFPlayer>();
			if (!pEntity || !pEntity->IsPlayer())
				continue;
			if (pEntity->entindex() == I::EngineClient->GetLocalPlayer())
				continue;
			if (!pEntity->IsAlive() || pEntity->IsDormant() || !pEntity->ShouldDraw())
				continue;
			if (!SDK::IsOnScreen(pEntity, false))
				continue;

			bool bEnemy = pEntity->m_iTeamNum() != pLocal->m_iTeamNum();
			int  iMode = bEnemy ? iEnemyMode : iTeamMode;
			if (!iMode)
				continue;

			const char* sMat = ChamsModeToMaterial(iMode);
			if (!sMat)
				continue;

			Color_t tVisible = bEnemy
				? Vars::Visuals::Chams::EnemyColor.Value
				: Vars::Visuals::Chams::TeamColor.Value;
			Color_t tOccluded = bEnemy
				? Vars::Visuals::Chams::EnemyXQZColor.Value
				: Vars::Visuals::Chams::TeamXQZColor.Value;

			Chams_t tChams;
			tChams.Visible.emplace_back(sMat, tVisible);
			if (Vars::Visuals::Chams::XQZ.Value)
				tChams.Occluded.emplace_back(sMat, tOccluded);

			m_vEntities.emplace_back(pEntity, tChams);
		}
		return;
	}

	// ── Caminho normal via Groups ──────────────────────────────────────────
	for (auto& [pEntity, pGroup] : F::Groups.GetGroup())
	{
		if (pEntity->IsDormant() || !pEntity->ShouldDraw())
			continue;

		if (pGroup->m_tChams()
			&& SDK::IsOnScreen(pEntity, pEntity->IsBaseCombatWeapon() || pEntity->IsWearable()))
			m_vEntities.emplace_back(pEntity, pGroup->m_tChams);

		if (pEntity->IsPlayer() && pEntity != pLocal && pGroup->m_iBacktrack & BacktrackEnum::Enabled && !pGroup->m_vBacktrackChams.empty()
			&& (F::Backtrack.GetFakeLatency() || F::Backtrack.GetFakeInterp() > G::Lerp || F::Backtrack.GetWindow()))
		{	// backtrack
			auto pWeapon = H::Entities.GetWeapon();
			if (pWeapon && (pGroup->m_iBacktrack & BacktrackEnum::Always || G::PrimaryWeaponType != EWeaponType::PROJECTILE))
			{
				bool bShowFriendly = false, bShowEnemy = true;
				if (G::PrimaryWeaponType == EWeaponType::MELEE && SDK::AttribHookValue(0, "speed_buff_ally", pWeapon) > 0)
					bShowFriendly = true;
				else if (pWeapon->GetWeaponID() == TF_WEAPON_MEDIGUN)
					bShowFriendly = true, bShowEnemy = false;

				if (bShowEnemy && pEntity->m_iTeamNum() != pLocal->m_iTeamNum() || bShowFriendly && pEntity->m_iTeamNum() == pLocal->m_iTeamNum())
					m_vEntities.emplace_back(pEntity, Chams_t(pGroup->m_vBacktrackChams, {}), pGroup->m_iBacktrack);
			}
		}
	}

	Group_t* pGroup = nullptr;
	if (F::FakeAngle.bDrawChams && F::FakeAngle.bBonesSetup
		&& F::Groups.GetGroup(TargetsEnum::FakeAngle, pGroup) && pGroup->m_tChams(true))
	{	// fakeangle
		m_vEntities.emplace_back(pLocal, pGroup->m_tChams, 1);
	}
}

void CChams::RenderMain()
{
	auto pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!pRenderContext)
		return;

	for (auto& tInfo : m_vEntities)
	{
		if (!tInfo.m_iFlags)
			DrawModel(tInfo.m_pEntity, tInfo.m_tChams, pRenderContext);
		else
		{
			m_iFlags = tInfo.m_iFlags;

			auto pPlayer = tInfo.m_pEntity->As<CTFPlayer>();
			const float flOldInvisibility = pPlayer->m_flInvisibility();
			pPlayer->m_flInvisibility() = 0.f;
			DrawModel(tInfo.m_pEntity, tInfo.m_tChams, pRenderContext, false);
			pPlayer->m_flInvisibility() = flOldInvisibility;

			m_iFlags = false;
		}
	}
}

void CChams::RenderBacktrack(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo)
{
	auto pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!pRenderContext)
		return;

	auto pEntity = I::ClientEntityList->GetClientEntity(pInfo.entity_index)->As<CTFPlayer>();
	if (!pEntity || !pEntity->IsPlayer())
		return;

	std::vector<TickRecord*> vRecords = {};
	if (!F::Backtrack.GetRecords(pEntity, vRecords))
		return;
	vRecords = F::Backtrack.GetValidRecords(vRecords);
	if (!vRecords.size())
		return;

	bool bDrawLast = m_iFlags & BacktrackEnum::Last;
	bool bDrawFirst = m_iFlags & BacktrackEnum::First;

	pRenderContext->DepthRange(0.f, m_iFlags & BacktrackEnum::IgnoreZ ? 0.2f : 1.f);

	auto drawModel = [&](Vec3& vOrigin, const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld, float flBlend)
		{
			if (!SDK::IsOnScreen(pEntity, vOrigin))
				return;

			float flOriginalBlend = I::RenderView->GetBlend();
			I::RenderView->SetBlend(flBlend * flOriginalBlend);
			static auto IVModelRender_DrawModelExecute = U::Hooks.m_mHooks["IVModelRender_DrawModelExecute"];
			IVModelRender_DrawModelExecute->Call<void>(I::ModelRender, pState, pInfo, pBoneToWorld);
			I::RenderView->SetBlend(flOriginalBlend);
		};
	if (!bDrawLast && !bDrawFirst)
	{
		for (auto pRecord : vRecords)
		{
			if (float flBlend = Math::RemapVal(pEntity->GetAbsOrigin().DistTo(pRecord->m_vOrigin), 1.f, 24.f, 0.f, 1.f))
				drawModel(pRecord->m_vOrigin, pState, pInfo, pRecord->m_aBones, flBlend);
		}
	}
	else
	{
		if (bDrawLast)
		{
			auto pRecord = vRecords.back();
			if (float flBlend = Math::RemapVal(pEntity->GetAbsOrigin().DistTo(pRecord->m_vOrigin), 1.f, 24.f, 0.f, 1.f))
				drawModel(pRecord->m_vOrigin, pState, pInfo, pRecord->m_aBones, flBlend);
		}
		if (bDrawFirst)
		{
			auto pRecord = vRecords.front();
			if (float flBlend = Math::RemapVal(pEntity->GetAbsOrigin().DistTo(pRecord->m_vOrigin), 1.f, 24.f, 0.f, 1.f))
				drawModel(pRecord->m_vOrigin, pState, pInfo, pRecord->m_aBones, flBlend);
		}
	}

	pRenderContext->DepthRange(0.f, 1.f);
}
void CChams::RenderFakeAngle(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo)
{
	//auto pRenderContext = I::MaterialSystem->GetRenderContext();
	//if (!pRenderContext)
	//	return;

	//pRenderContext->DepthRange(0.f, Vars::Chams::FakeAngle::IgnoreZ.Value ? 0.2f : 1.f);

	static auto IVModelRender_DrawModelExecute = U::Hooks.m_mHooks["IVModelRender_DrawModelExecute"];
	IVModelRender_DrawModelExecute->Call<void>(I::ModelRender, pState, pInfo, F::FakeAngle.aBones);

	//pRenderContext->DepthRange(0.f, 1.f);
}
void CChams::RenderHandler(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
	if (!m_iFlags)
	{
		static auto IVModelRender_DrawModelExecute = U::Hooks.m_mHooks["IVModelRender_DrawModelExecute"];
		IVModelRender_DrawModelExecute->Call<void>(I::ModelRender, pState, pInfo, pBoneToWorld);
	}
	else
	{
		if (pInfo.entity_index != I::EngineClient->GetLocalPlayer())
			RenderBacktrack(pState, pInfo);
		else
			RenderFakeAngle(pState, pInfo);
	}
}

bool CChams::RenderViewmodel(void* ecx, int flags, int* iReturn)
{
	if (!F::Groups.GroupsActive())
		return false;

	auto pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!pRenderContext)
		return false;

	Group_t* pGroup = nullptr;
	if (!F::Groups.GetGroup(TargetsEnum::ViewmodelWeapon, pGroup) || !pGroup->m_tChams(true))
		return false;

	Begin();
	for (auto& [sName, tColor] : pGroup->m_tChams.Visible)
	{
		auto pMaterial = F::Materials.GetMaterial(FNV1A::Hash32(sName.c_str()));

		F::Materials.SetColor(pMaterial, tColor);
		I::ModelRender->ForcedMaterialOverride(pMaterial ? pMaterial->m_pMaterial : nullptr);

		if (pMaterial && pMaterial->m_bInvertCull)
			pRenderContext->CullMode(G::FlipViewmodels ? MATERIAL_CULLMODE_CCW : MATERIAL_CULLMODE_CW);

		static auto CBaseAnimating_InternalDrawModel = U::Hooks.m_mHooks["CBaseAnimating_InternalDrawModel"];
		*iReturn = CBaseAnimating_InternalDrawModel->Call<int>(ecx, flags);

		if (pMaterial && pMaterial->m_bInvertCull)
			pRenderContext->CullMode(G::FlipViewmodels ? MATERIAL_CULLMODE_CW : MATERIAL_CULLMODE_CCW);
	}
	End();

	return true;
}
bool CChams::RenderViewmodel(const DrawModelState_t& pState, const ModelRenderInfo_t& pInfo, matrix3x4* pBoneToWorld)
{
	if (!F::Groups.GroupsActive())
		return false;

	auto pRenderContext = I::MaterialSystem->GetRenderContext();
	if (!pRenderContext)
		return false;

	Group_t* pGroup = nullptr;
	if (!F::Groups.GetGroup(TargetsEnum::ViewmodelHands, pGroup) || !pGroup->m_tChams(true))
		return false;

	Begin();
	for (auto& [sName, tColor] : pGroup->m_tChams.Visible)
	{
		auto pMaterial = F::Materials.GetMaterial(FNV1A::Hash32(sName.c_str()));

		F::Materials.SetColor(pMaterial, tColor);
		I::ModelRender->ForcedMaterialOverride(pMaterial ? pMaterial->m_pMaterial : nullptr);

		if (pMaterial && pMaterial->m_bInvertCull)
			pRenderContext->CullMode(G::FlipViewmodels ? MATERIAL_CULLMODE_CCW : MATERIAL_CULLMODE_CW);

		static auto IVModelRender_DrawModelExecute = U::Hooks.m_mHooks["IVModelRender_DrawModelExecute"];
		IVModelRender_DrawModelExecute->Call<void>(I::ModelRender, pState, pInfo, pBoneToWorld);

		if (pMaterial && pMaterial->m_bInvertCull)
			pRenderContext->CullMode(G::FlipViewmodels ? MATERIAL_CULLMODE_CW : MATERIAL_CULLMODE_CCW);
	}
	End();

	return true;
}