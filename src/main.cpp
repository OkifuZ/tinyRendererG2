#include "scene3d.h"
#include "mpm.h"
#include "ticktock.h"
#include "material.h"
#include "renderer.h"
#include "general_phong_rp.h"
#include "entity.h"
#include "resource_manager.h"
#include "rhi.h"
#include "user_control.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <filesystem>
#include <random>



double phyTick_time = 0.0;
double renderTick_time = 0.0;
int main_loop_cnt = 0;


void main_print_profiler() {
	if (main_loop_cnt == 0) {
		phyTick_time = -1.0;
		renderTick_time = -1.0;
	}
	else {
		phyTick_time = phyTick_time / (double)main_loop_cnt;
		renderTick_time = renderTick_time / (double)main_loop_cnt;
	}
	printf("--Main per-frame profiler(average)--\n");
	printf("-physical:   %.6f\n", phyTick_time);
	printf("-render      %.6f\n", renderTick_time);
	printf("------------------------------------\n");
}

void init_particles_pos(Entity_ptr entity) {
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<> dis(0, 1);//uniform distribution between 0 and 1
	
	entity->instance_data.instance_num = 30 * 30 * 30;
	for (size_t i = 0; i < 30; i++) {
		for (size_t j = 0; j < 30; j++) {
			for (size_t k = 0; k < 30; k++) {
				float x = float(dis(gen)) * 10;
				float y = float(dis(gen)) * 10;
				float z = float(dis(gen)) * 10;
				entity->instance_data.offset.insert(entity->instance_data.offset.end(),
					{ x,y,z });
				entity->instance_data.scale.push_back(0.2f + float(dis(gen))*0.1f);
				entity->instance_data.color.insert(entity->instance_data.color.end(),
					{ 0.6f, 0.5f, 0.9f });

			}
		}
	}
}

void mesh_to_instance(MeshDataContainer_ptr mesh, Entity_ptr entity) {
	entity->instance_data.instance_num = mesh->verts_num;
	entity->instance_data.offset = mesh->verts;
}

void update_particles_pos(Entity_ptr entity) {
	for (size_t i = 0; i < entity->instance_data.instance_num; i++) {
		entity->instance_data.offset[i * 3 + 0] += 0.01f;
		entity->instance_data.offset[i * 3 + 1] += 0.01f;
		entity->instance_data.offset[i * 3 + 2] += 0.01f;
	}
	entity->instance_data.offset_dirty = true;
}

void update_mesh_vert(MeshDataContainer_ptr mesh) {

	for (size_t i = 0; i < mesh->verts_num; i++) {
		mesh->verts[i * 3 + 0] *= 1.005f;
		mesh->verts[i * 3 + 1] *= 1.005f;
		mesh->verts[i * 3 + 2] *= 1.005f;
	}
	mesh->verts_dirty = true;
}

int main()
{
	// runtime environment
	RHI_InitConfig rhi_cfg;
	rhi_init(window_global, rhi_cfg);

	// asset loading
	const std::filesystem::path& material_path = "../resource/material";
	const std::filesystem::path& shader_path   = "../resource/shader";
	const std::filesystem::path& mesh_path     = "../resource/mesh";

	RenderMaterial_ptr mat = std::make_shared<RenderMaterial>();
	load_material(*mat, (material_path / "general_phong.json").string());
	Snowflake_type mat_uuid = resource_manager_global.add_material(mat);

	Shader_ptr shader = std::make_shared<Shader>();
	load_shader(shader, 
		(shader_path / "instance_phong.vert").string(),
		(shader_path / "instance_phong.frag").string());
	Snowflake_type shader_uuid = resource_manager_global.add_shader(shader, "general_phong");

	MeshDataContainer_ptr mesh = std::make_shared<MeshDataContainer>();
	load_mesh(mesh, (mesh_path / "sphere_smooth.obj").string());
	Snowflake_type mesh_uuid = resource_manager_global.add_mesh(mesh);

	Entity_ptr entity = std::make_shared<Entity>();
	entity->name = "sphere";
	entity->mesh_uuid = mesh_uuid;
	entity->material_uuid = mat_uuid;
	Snowflake_type entity_uuid = resource_manager_global.add_entity(entity);

	// draw gizmo

	DirectLight_ptr direct_light = std::make_shared<DirectLight>();
	direct_light->transform.look_at(glm::vec3{ 3, 3, 3 }, glm::vec3{0, 0, 0});
	direct_light->light_common_attr.specular = glm::vec3(0.2f);
	Snowflake_type dir_light_uuid = resource_manager_global.add_directLight(direct_light);
	PointLight_ptr point_light = std::make_shared<PointLight>();
	point_light->transform.translate = {2, 2, -2};
	Snowflake_type pt_light_uuid = resource_manager_global.add_pointLight(point_light);

	SceneData_ptr scene = std::make_shared<SceneData>();
	Snowflake_type scene_uuid = resource_manager_global.add_scene(scene);
	scene->entities.push_back(entity_uuid);
	scene->direct_lights.push_back(dir_light_uuid);
	//scene->point_lights.push_back(pt_light_uuid);

	Camera_ptr camera = std::make_shared<Camera>();
	bool camera_ini_valid = camera->cam_look_at({ 12, 12, 12 }, {0, 0, 0}, { 0, 1, 0 });
	if (!camera_ini_valid) {
		printf("Invalid initial camera direction\n");
		exit(-1);
	}
	Snowflake_type camera_uuid = resource_manager_global.add_camera(camera);

	ControllSystem controller;
	controller.register_camera(camera);



	// TODO: update instance data
	init_particles_pos(entity);

	// render
	RenderPipeline_ptr render_pipeline = std::make_shared<GeneralPhongRenderPipeline>();
	bool render_prepared = render_pipeline->prepare(scene_uuid);
	if (!render_prepared) {
		printf("Render pipeline preparation failed\n");
		exit(-1);
	}

	while (!render_should_quit(window_global)) {
		glfwPollEvents(); // window(GLFW related events)
		
		controller.process_input();

		// update_particles_pos(entity);
		// update_mesh_vert(mesh);

		render_pipeline->render(scene_uuid, camera_uuid);

		glfwSwapBuffers(window_global.window);
	}


	render_terminate(window_global);
	exit(0);

	/*
	Scene3D scene("mpm99", 800u, 800u);
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(scene.window, true);
	const char* glsl_version = "#version 130";
	ImGui_ImplOpenGL3_Init(glsl_version);

	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	while (!scene.should_quit()) {

		// window
		glfwPollEvents(); // window(GLFW related events)
		scene.display_fps(); // change window title

		// physical step

		// input handling

		// rendering
		// before rendering
		int display_w, display_h;
		glfwGetFramebufferSize(scene.window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);

		scene.render();
		
		// after rendering
		glfwSwapBuffers(scene.window);
		
		// gizmo rendering

		// GUI rendering & GUI event handling

		// 
	}


	glfwDestroyWindow(scene.window);
	glfwTerminate();
	*/
	return 0;
}

/*
	use_cuda_gl_interop = false;
	// mpm_load_config("../mpm_cfg.json"); // call this before Scene()!

	Scene scene("mpm99", n_particles, 800u, 800u, use_cuda_gl_interop, &cu_x_handle);
	// scene.set_camera(pos, lookat, up=(0,1,0));
	// scene.set_pointLight(pos);
	// scene.set_directLight(dir);

	float radius = 2.1;

	// mpm_initialize();
	float* pos_data_ptr = use_cuda_gl_interop ? nullptr : x; // just a hint, can be deleted

	while (!scene.should_quit()) {

		TICK(PHY);
		// mpm_step();
		phyTick_time += TOCK_VAL(PHY);

		// scene.handle_input() // mouse, keyboard

		TICK(RENDER);
		// scene.set_background_color(glm::vec3(0.191f, 0.308f, 0.308f));
		// scene.set_skybox();
		// scene.set_model("floor", pos, rot, scale);
		// scene.set_particles(use_cuda_gl_interop, pos_data_ptr, n_particles, radius, color);
		// scene.render();
		renderTick_time += TOCK_VAL(RENDER);

		main_loop_cnt += 1;
	}
	// mpm_end();
	// mpm_print_profiler();

	// main_print_profiler();

*/

/*
Scene3D scene("mpm99", 800u, 800u);
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(scene.window, true);
	const char* glsl_version = "#version 130";
	ImGui_ImplOpenGL3_Init(glsl_version);


	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	while (!scene.should_quit()) {

		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		scene.render();
		ImGui::ShowDemoWindow();

		//{
		//	static float f = 0.0f;
		//	static int counter = 0;

		//	ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		//	ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		//	ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
		//	ImGui::Checkbox("Another Window", &show_another_window);

		//	ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		//	ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

		//	if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
		//		counter++;
		//	ImGui::SameLine();
		//	ImGui::Text("counter = %d", counter);

		//	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		//	ImGui::End();
		//}

		ImGui::Render();
		int display_w, display_h;
		glfwGetFramebufferSize(scene.window, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);
		glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
		glClear(GL_COLOR_BUFFER_BIT);
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		scene.display_fps();

		glfwSwapBuffers(scene.window);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(scene.window);
	glfwTerminate();

	return 0;

*/