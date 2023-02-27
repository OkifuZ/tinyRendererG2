#include "grabber.h"



void Grabber::register_grabber_to_controller(ControllSystem& ctr_sys) {
	auto on_pointer_press_event = [this](const MouseState& mouse_state) {
		if (!enabled) return;

		ray_caster->cast_ray_from_camera(camera, mouse_state.cursor_x, mouse_state.cursor_y);
		ray_caster->intersect_ray_with_entities();
		grabbed_entity = ray_caster->ray->intersects.entity;
		if (grabbed_entity)
			printf("hit entity %s, dis %.3f\n",
				ray_caster->ray->intersects.entity->name.c_str(), ray_caster->ray->intersects.triangle.dis);
		else {
			printf("miss\n");
			return;
		}

		TinyPhyxSole* phy_object = grabbed_entity->phy_object;
		if (phy_object) {
			distance = ray_caster->ray->intersects.triangle.dis;
			glm::vec3 pos = ray_caster->ray->ori + ray_caster->ray->direction * distance;
			phy_object->start_grab(pos);
			phy_object->choose_point(pos);
		}

		if (camera) camera->freezed = true;
	};

	ctr_sys.register_mouse_event_function(
		ControllSystem::MOUSE_EVENT::ON_LEFT_POINTER_PRESS, on_pointer_press_event);

	auto on_pointer_release_event = [this](const MouseState& mouse_state) {
		if (!enabled) return;
		if (!grabbed_entity) return;

		TinyPhyxSole* phy_object = grabbed_entity->phy_object;
		if (phy_object) {
			ray_caster->cast_ray_from_camera(camera, mouse_state.cursor_x, mouse_state.cursor_y);
			glm::vec3 pos = ray_caster->ray->ori + ray_caster->ray->direction * distance;
			phy_object->end_grab(pos);
		}
		grabbed_entity = nullptr;

		if (camera) camera->freezed = false;
	};
	ctr_sys.register_mouse_event_function(
		ControllSystem::MOUSE_EVENT::ON_LEFT_POINTER_RELEASE, on_pointer_release_event);

	auto on_pointer_down_event = [this](const MouseState& mouse_state) {
		if (!enabled) return;
		if (!grabbed_entity) return;

		TinyPhyxSole* phy_object = grabbed_entity->phy_object;
		if (phy_object) {
			ray_caster->cast_ray_from_camera(camera, mouse_state.cursor_x, mouse_state.cursor_y);
			glm::vec3 pos = ray_caster->ray->ori + ray_caster->ray->direction * distance;
			phy_object->move_grab(pos);
		}
	};
	ctr_sys.register_mouse_event_function(
		ControllSystem::MOUSE_EVENT::ON_LEFT_POINTER_DOWN, on_pointer_down_event);

}