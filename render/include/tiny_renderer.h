#pragma once
#include "rhi.h"
#include "material.h"
#include "scene_ele.h"
#include "renderer.h"
#include "render_pipeline/general_phong_rp.h"
#include "entity.h"
#include "resource_manager.h"
#include "fps.h"
#include "user_control.h"
#include "grabber.h"

#include <functional>
#include <vector>

#include "UI_layout.h"

class TinyRenderer {

	std::vector<RenderPipeline_sptr> render_pipelines;

	ControllSystem_sptr controller;

	Snowflake_type scene_uuid{};

	struct Functions {
		std::function<void()> _UI_layout_update;
		std::function<void()> _UI_event_handler;
		std::function<void()> _physics_tick;
		std::function<void()> _reset;

	} functions;


public:
	void init() {
		// runtime environment
		RHI_InitConfig rhi_cfg;
		rhi_init(window_global, rhi_cfg);
		if (!init_resource_manager()) {
			printf("[ERROR] init resource manager\n");
			exit(-1);
		}
		scene_uuid = load_scene(ResourceManager::scene_path / "scene.json");
		controller = std::make_shared<ControllSystem>();

		auto& camera = resource_manager_global.get_camera_by_name("camera");
		camera->register_event_to_controller(*controller);
		camera->register_keyboard_to_controller(*controller);

		resource_manager_global.grabber = std::make_shared<Grabber>(camera);
		resource_manager_global.grabber->register_grabber_to_controller(*controller);

		// render
		render_pipelines.push_back(std::make_shared<GeneralPhongRenderPipeline>("GeneralPhong"));

		for (auto& rp : render_pipelines) {
			bool render_prepared = rp->prepare(scene_uuid);
			if (!render_prepared) {
				printf("Render pipeline \"%s\" preparation failed\n", rp->name);
				exit(-1);
			}
		}
	}

	void loop() {
		while (render_next(window_global)) {
			if (functions._UI_event_handler) this->functions._UI_event_handler();

			// app input process
			controller->process_input();

			// prerender UI
			if (functions._UI_layout_update) functions._UI_layout_update();\

			// physics tick
			if (functions._physics_tick) functions._physics_tick();

			// render tick
			for (auto& rp: render_pipelines) 
				rp->render(scene_uuid);

			// render UI
			render_ui(); 

			// if reset
			// if (functions._reset) functions._reset();
			
		}
	}

	void terminate() { render_terminate(window_global); }
	void register_physics_tick(std::function<void()> foo) { this->functions._physics_tick = foo; }
	void register_reset(std::function<void()> foo) { this->functions._reset = foo; }
	void register_ui_layout_update(std::function<void()> foo) { this->functions._UI_layout_update = foo; }
	void register_ui_event_handler(std::function<void()> foo) { this->functions._UI_event_handler = foo; }


	static Entity_ptr get_entity(const std::string& entity_name) {
		return resource_manager_global.get_entity_by_name("sphere_tet_entity");
	}
};