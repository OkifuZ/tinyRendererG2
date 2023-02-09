#pragma once
#include "snowflake.h"
#include "glm/glm.hpp"
#include <memory>

class RenderPipeline;
typedef std::shared_ptr<RenderPipeline> RenderPipeline_ptr;


class RenderPipeline {
public:

	virtual bool prepare(Snowflake_type scene_uuid) {
		// validation & set everything of scene onload
		// compile shader: program_id
		// mesh data: cpu->gpu vertex attribute object
		// material: cpu->=gpu inject_material_to_shader

		return false; // if success, return true
	}

	virtual void render(Snowflake_type scene_uuid, Snowflake_type camera) {
		// pass begin
		// 1. clear_render_target
		// 2. get_culling_result - optional
		// 3. get_target_shader
		// 4. bound_material_to_shader
		// 5. bound_mesh_to_shader
		// 6. bound_camera_to_shader
		// 7. draw call
		// 8. draw_skybox - optional
		// pass end

		// other passes
		// ...
	}

};

