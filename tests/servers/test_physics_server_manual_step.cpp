/**************************************************************************/
/*  test_physics_server_manual_step.cpp                                   */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "tests/test_macros.h"

TEST_FORCE_LINK(test_physics_server_manual_step)

#include "modules/modules_enabled.gen.h"

// The test harness installs the dummy physics servers, so these cases construct
// the real backends directly rather than going through PhysicsServer*DManager.

#ifdef MODULE_GODOT_PHYSICS_2D_ENABLED
#include "modules/godot_physics_2d/godot_physics_server_2d.h"
#endif

#ifdef MODULE_GODOT_PHYSICS_3D_ENABLED
#include "modules/godot_physics_3d/godot_physics_server_3d.h"
#endif

namespace TestPhysicsServerManualStep {

#ifdef MODULE_GODOT_PHYSICS_2D_ENABLED

TEST_CASE("[PhysicsServer2D] Manually stepping an inactive space advances it") {
	GodotPhysicsServer2D server;
	server.init();

	const RID space = server.space_create();
	// Deliberately left inactive: an active space is stepped by the engine instead.
	const RID body = server.body_create();
	server.body_set_space(body, space);
	server.body_set_mode(body, PS2DE::BODY_MODE_RIGID);

	const RID shape = server.circle_shape_create();
	server.shape_set_data(shape, 0.5);
	server.body_add_shape(body, shape);

	server.body_set_state(body, PS2DE::BODY_STATE_LINEAR_VELOCITY, Vector2(4, 0));

	const Transform2D before = server.body_get_state(body, PS2DE::BODY_STATE_TRANSFORM);
	for (int i = 0; i < 10; i++) {
		server.space_step(space, 1.0 / 60.0);
	}
	const Transform2D after = server.body_get_state(body, PS2DE::BODY_STATE_TRANSFORM);

	CHECK_MESSAGE(after.get_origin().x > before.get_origin().x,
			"A body in a manually stepped space should move.");

	server.free(shape);
	server.free(body);
	server.free(space);
	server.finish();
}

TEST_CASE("[PhysicsServer2D] An inactive space only advances when stepped") {
	GodotPhysicsServer2D server;
	server.init();

	const RID space = server.space_create();
	const RID body = server.body_create();
	server.body_set_space(body, space);
	server.body_set_mode(body, PS2DE::BODY_MODE_RIGID);

	const RID shape = server.circle_shape_create();
	server.shape_set_data(shape, 0.5);
	server.body_add_shape(body, shape);

	server.body_set_state(body, PS2DE::BODY_STATE_LINEAR_VELOCITY, Vector2(4, 0));

	const Transform2D before = server.body_get_state(body, PS2DE::BODY_STATE_TRANSFORM);
	// No space_step() call here.
	const Transform2D after = server.body_get_state(body, PS2DE::BODY_STATE_TRANSFORM);

	CHECK_MESSAGE(after.get_origin().is_equal_approx(before.get_origin()),
			"A body should not move while its space is neither active nor stepped.");

	server.free(shape);
	server.free(body);
	server.free(space);
	server.finish();
}

TEST_CASE("[PhysicsServer2D] Flushing queries with an invalid space keeps the flushing flag clear") {
	GodotPhysicsServer2D server;
	server.init();

	CHECK_FALSE(server.is_flushing_queries());

	ERR_PRINT_OFF;
	server.space_flush_queries(RID());
	ERR_PRINT_ON;

	// Raising the flag before validating the argument would leave the server
	// permanently "flushing", which silently disables every guard that reads
	// is_flushing_queries() (such as Area2D::set_monitorable).
	CHECK_MESSAGE(!server.is_flushing_queries(),
			"An invalid space must not leave the server stuck in the flushing state.");

	server.finish();
}

TEST_CASE("[PhysicsServer2D] Manually stepping an active space is rejected") {
	GodotPhysicsServer2D server;
	server.init();

	const RID space = server.space_create();
	server.space_set_active(space, true);

	const RID body = server.body_create();
	server.body_set_space(body, space);
	server.body_set_mode(body, PS2DE::BODY_MODE_RIGID);

	const RID shape = server.circle_shape_create();
	server.shape_set_data(shape, 0.5);
	server.body_add_shape(body, shape);

	server.body_set_state(body, PS2DE::BODY_STATE_LINEAR_VELOCITY, Vector2(4, 0));

	const Transform2D before = server.body_get_state(body, PS2DE::BODY_STATE_TRANSFORM);
	ERR_PRINT_OFF;
	server.space_step(space, 1.0 / 60.0);
	server.space_flush_queries(space);
	ERR_PRINT_ON;
	const Transform2D after = server.body_get_state(body, PS2DE::BODY_STATE_TRANSFORM);

	CHECK_MESSAGE(after.get_origin().is_equal_approx(before.get_origin()),
			"An active space must not be advanced by a manual step.");
	CHECK_FALSE(server.is_flushing_queries());

	server.space_set_active(space, false);
	server.free(shape);
	server.free(body);
	server.free(space);
	server.finish();
}

#endif // MODULE_GODOT_PHYSICS_2D_ENABLED

#ifdef MODULE_GODOT_PHYSICS_3D_ENABLED

TEST_CASE("[PhysicsServer3D] Manually stepping an inactive space advances it") {
	GodotPhysicsServer3D server;
	server.init();

	const RID space = server.space_create();
	const RID body = server.body_create();
	server.body_set_space(body, space);
	server.body_set_mode(body, PS3DE::BODY_MODE_RIGID);

	const RID shape = server.sphere_shape_create();
	server.shape_set_data(shape, 0.5);
	server.body_add_shape(body, shape);

	server.body_set_state(body, PS3DE::BODY_STATE_LINEAR_VELOCITY, Vector3(4, 0, 0));

	const Transform3D before = server.body_get_state(body, PS3DE::BODY_STATE_TRANSFORM);
	for (int i = 0; i < 10; i++) {
		server.space_step(space, 1.0 / 60.0);
	}
	const Transform3D after = server.body_get_state(body, PS3DE::BODY_STATE_TRANSFORM);

	CHECK_MESSAGE(after.origin.x > before.origin.x,
			"A body in a manually stepped space should move.");

	server.free(shape);
	server.free(body);
	server.free(space);
	server.finish();
}

TEST_CASE("[PhysicsServer3D] Stepping one space leaves another untouched") {
	GodotPhysicsServer3D server;
	server.init();

	const RID shape = server.sphere_shape_create();
	server.shape_set_data(shape, 0.5);

	const RID space_a = server.space_create();
	const RID body_a = server.body_create();
	server.body_set_space(body_a, space_a);
	server.body_set_mode(body_a, PS3DE::BODY_MODE_RIGID);
	server.body_add_shape(body_a, shape);
	server.body_set_state(body_a, PS3DE::BODY_STATE_LINEAR_VELOCITY, Vector3(4, 0, 0));

	const RID space_b = server.space_create();
	const RID body_b = server.body_create();
	server.body_set_space(body_b, space_b);
	server.body_set_mode(body_b, PS3DE::BODY_MODE_RIGID);
	server.body_add_shape(body_b, shape);
	server.body_set_state(body_b, PS3DE::BODY_STATE_LINEAR_VELOCITY, Vector3(4, 0, 0));

	const Vector3 b_before = server.body_get_state(body_b, PS3DE::BODY_STATE_TRANSFORM).operator Transform3D().origin;
	for (int i = 0; i < 10; i++) {
		server.space_step(space_a, 1.0 / 60.0);
	}
	const Vector3 a_after = server.body_get_state(body_a, PS3DE::BODY_STATE_TRANSFORM).operator Transform3D().origin;
	const Vector3 b_after = server.body_get_state(body_b, PS3DE::BODY_STATE_TRANSFORM).operator Transform3D().origin;

	CHECK_MESSAGE(a_after.x > 0, "The stepped space should have advanced.");
	CHECK_MESSAGE(b_after.is_equal_approx(b_before), "An unstepped space must not advance.");

	server.free(body_a);
	server.free(space_a);
	server.free(body_b);
	server.free(space_b);
	server.free(shape);
	server.finish();
}

TEST_CASE("[PhysicsServer3D] Flushing queries with an invalid space keeps the flushing flag clear") {
	GodotPhysicsServer3D server;
	server.init();

	CHECK_FALSE(server.is_flushing_queries());

	ERR_PRINT_OFF;
	server.space_flush_queries(RID());
	ERR_PRINT_ON;

	CHECK_MESSAGE(!server.is_flushing_queries(),
			"An invalid space must not leave the server stuck in the flushing state.");

	server.finish();
}

TEST_CASE("[PhysicsServer3D] Manually stepping an active space is rejected") {
	GodotPhysicsServer3D server;
	server.init();

	const RID space = server.space_create();
	server.space_set_active(space, true);

	const RID body = server.body_create();
	server.body_set_space(body, space);
	server.body_set_mode(body, PS3DE::BODY_MODE_RIGID);

	const RID shape = server.sphere_shape_create();
	server.shape_set_data(shape, 0.5);
	server.body_add_shape(body, shape);

	server.body_set_state(body, PS3DE::BODY_STATE_LINEAR_VELOCITY, Vector3(4, 0, 0));

	const Transform3D before = server.body_get_state(body, PS3DE::BODY_STATE_TRANSFORM);
	ERR_PRINT_OFF;
	server.space_step(space, 1.0 / 60.0);
	server.space_flush_queries(space);
	ERR_PRINT_ON;
	const Transform3D after = server.body_get_state(body, PS3DE::BODY_STATE_TRANSFORM);

	CHECK_MESSAGE(after.origin.is_equal_approx(before.origin),
			"An active space must not be advanced by a manual step.");
	CHECK_FALSE(server.is_flushing_queries());

	server.space_set_active(space, false);
	server.free(shape);
	server.free(body);
	server.free(space);
	server.finish();
}

#endif // MODULE_GODOT_PHYSICS_3D_ENABLED

} // namespace TestPhysicsServerManualStep
