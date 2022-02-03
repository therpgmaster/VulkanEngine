#pragma once
#include "Core/engine_renderer.h"
#include "Core/GPU/Material.h"
#include "Core/EngineSettings.h"

namespace EngineCore
{
	/*	the materials manager holds material objects that are in active use,
	*	and will contain functionality for sorting, garbage collection, etc. */
	class MaterialsManager 
	{
	public:
		MaterialsManager(EngineRenderer& r, const EngineRenderSettings& rs, EngineDevice& d);

		/*	creates a new managed material and returns a handle to it 
			do not attempt to copy or move materials created this way! 
			caller is responsible for reporting when they start/stop using the material! */
		MaterialHandle createMaterial(const MaterialCreateInfo& matInfo);

		void matReportUserAddOrRemove(const MaterialHandle& mh, const int8_t& num);

	private:
		struct mgrMatInfo
		{ 
			mgrMatInfo(Material* p) : ptr{ p } {};
			Material* ptr; 
			uint32_t users = 0;
		};
		// managed materials
		std::vector<mgrMatInfo> materials;
		// note that the returned pointer may be invalidated by the container
		mgrMatInfo* find(const Material* m);

		void freeMaterial(mgrMatInfo& m);
		void freeUnusedMaterials();

		EngineDevice& device;
		EngineRenderer& renderer;
		const EngineRenderSettings& engineRenderSettings;
	};
}
