#pragma once
#include "material.h"
#include "scene_ele.h"
#include "renderer.h"
#include "general_phong_rp.h"
#include "entity.h"
#include "resource_manager.h"
#include "rhi.h"
#include "fps.h"
#include "user_control.h"

#include <functional>



class TinyRenderer {
	RenderPipeline_sptr render_pipeline;
	ControllSystem_sptr controller;

	Snowflake_type scene_uuid{};

	struct Functions {
		std::function<void()> _UI_layout_update;
		std::function<void()> _physics_tick;
		std::function<void()> _reset;

	} functions;

public:
	void init() {
		// runtime environment
		RHI_InitConfig rhi_cfg;
		rhi_init(window_global, rhi_cfg);
		init_resource_manager();
		scene_uuid = load_scene(ResourceManager::scene_path / "scene.json");
		controller = std::make_shared<ControllSystem>();
		controller->register_camera(resource_manager_global.get_camera_by_name("camera"));
		// render
		render_pipeline = std::make_shared<GeneralPhongRenderPipeline>();
		bool render_prepared = render_pipeline->prepare(scene_uuid);
		if (!render_prepared) {
			printf("Render pipeline preparation failed\n");
			exit(-1);
		}
	}

	void loop(bool& need_break_loop) {
		while (render_next(window_global)) {
			// app input process
			controller->process_input();

			// prerender UI
			functions._UI_layout_update();

			// physics tick
			functions._physics_tick();

			// render tick
			render_pipeline->render(scene_uuid);

			// render UI
			render_ui(); // functor

			// if reset
			functions._reset();

			if (need_break_loop) {
				need_break_loop = false;
				break;
			}

		}
	}

	void terminate() { render_terminate(window_global); }
	void register_physics_tick(std::function<void()> foo) { this->functions._physics_tick = foo; }
	void register_reset(std::function<void()> foo) { this->functions._reset = foo; }
	void register_ui_layout_update(std::function<void()> foo) { this->functions._UI_layout_update = foo; }


	static Entity_ptr get_entity(const std::string& entity_name) {
		return resource_manager_global.get_entity_by_name("sphere_tet_entity");
	}
};