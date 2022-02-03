#include "Core/GPU/MaterialsManager.h"

namespace EngineCore 
{
	MaterialsManager::MaterialsManager(EngineRenderer& r, const EngineRenderSettings& rs, EngineDevice& d)
		: renderer{ r }, engineRenderSettings{ rs }, device{ d } {}
	
	MaterialHandle MaterialsManager::createMaterial(const MaterialCreateInfo& matInfo)
	{
		Material* m = new Material(matInfo, engineRenderSettings, 
								renderer.getSwapchainRenderPass(), device);
		// take ownership of the material object 
		// try to find an empty handle first, otherwise create a new one
		for (auto& h : materials) 
		{ 
			if (!h.ptr) 
			{ 
				h.ptr = m;
				return MaterialHandle(m, this);
			} 
		}
		materials.push_back(mgrMatInfo(m));
		return MaterialHandle(m, this);
	}

	MaterialsManager::mgrMatInfo* MaterialsManager::find(const Material* m)
	{
		for (auto& h : materials) { if (h.ptr == m) { return &h; } }
	}

	void MaterialsManager::freeMaterial(MaterialsManager::mgrMatInfo& m)
	{
		// delete the material and its associated resources
		// this should only be done when the material is no longer in use
		delete m.ptr;
		m.ptr = nullptr;
	}

	void MaterialsManager::freeUnusedMaterials() 
	{
		if (materials.empty()) { return; }
		for (auto& h : materials) { if (h.users < 1) { freeMaterial(h); } }
	}

	void MaterialsManager::matReportUserAddOrRemove(const MaterialHandle& mh, const int8_t& num)
	{
		const auto* p = mh.get();
		if (!p) { return; }

		auto* m = find(p); // find the material in internal container
		if (m) 
		{ 
			m->users += num; // increment (or decrement, if value is negative)
			if (m->users < 0) { m->users = 0; }
		} 
	}
}