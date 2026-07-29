/**************************************************************************/
/*  grid_map_editor_plugin.h                                              */
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

#pragma once

#include "../grid_map.h"

#include "editor/docks/editor_dock.h"
#include "editor/plugins/editor_plugin.h"
#include "scene/gui/box_container.h"

class BaseButton;
class Button;
class ButtonGroup;
class ConfirmationDialog;
class EditorZoomWidget;
class FilterLineEdit;
class HSlider;
class ItemList;
class MenuButton;
class Node3DEditorPlugin;
class Node3DEditorViewport;
class SpinBox;
class Tree;
class TreeItem;

class GridMapEditor : public EditorDock {
	GDCLASS(GridMapEditor, EditorDock);

	static constexpr int32_t GRID_CURSOR_SIZE = 50;

	enum InputAction {
		INPUT_NONE,
		INPUT_TRANSFORM,
		INPUT_PAINT,
		INPUT_ERASE,
		INPUT_PICK,
		INPUT_SELECT,
		INPUT_PASTE,
	};

	enum DisplayMode {
		DISPLAY_THUMBNAIL,
		DISPLAY_LIST
	};

	InputAction input_action = INPUT_NONE;
	bool valid_mb_press = false;
	Panel *panel = nullptr;
	MenuButton *options = nullptr;
	SpinBox *floor = nullptr;
	double accumulated_floor_delta = 0.0;

	HBoxContainer *toolbar = nullptr;
	TightLocalVector<BaseButton *> viewport_shortcut_buttons;
	Ref<ButtonGroup> mode_buttons_group;
	// mode
	Button *transform_mode_button = nullptr;
	Button *select_mode_button = nullptr;
	Button *erase_mode_button = nullptr;
	Button *paint_mode_button = nullptr;
	Button *pick_mode_button = nullptr;
	// action
	Button *fill_action_button = nullptr;
	Button *move_action_button = nullptr;
	Button *duplicate_action_button = nullptr;
	Button *delete_action_button = nullptr;
	// rotation
	Button *rotate_x_button = nullptr;
	Button *rotate_y_button = nullptr;
	Button *rotate_z_button = nullptr;
	Button *clear_rotation_button = nullptr;

	EditorZoomWidget *zoom_widget = nullptr;
	Button *mode_thumbnail = nullptr;
	Button *mode_list = nullptr;
	FilterLineEdit *search_box = nullptr;
	HSlider *size_slider = nullptr;
	ConfirmationDialog *settings_dialog = nullptr;
	VBoxContainer *settings_vbc = nullptr;
	SpinBox *settings_pick_distance = nullptr;
	Label *spin_box_label = nullptr;

	struct SetItem {
		Vector3i position;
		int new_value = 0;
		int new_orientation = 0;
		int old_value = 0;
		int old_orientation = 0;
	};

	LocalVector<SetItem> set_items;

	GridMap *node = nullptr;
	// caching the node global transform to detect when the node has been
	// moved/scaled/rotated.
	Transform3D node_global_transform;
	Ref<MeshLibrary> mesh_library = nullptr;

	// Plane the cells are edited on. Its distance from the origin comes from edit_floor.
	Plane edit_plane;

	// Square cells are edited on the X, Y and Z planes. Hexagonal cells replace
	// the Z plane with the three planes of the axial coordinate system.
	enum EditAxis {
		AXIS_X = 0,
		AXIS_Y,
		AXIS_Z,
		AXIS_Q, // Runs northwest to southeast.
		AXIS_R, // Runs east to west; the hexagonal equivalent of AXIS_Z.
		AXIS_S, // Runs southwest to northeast.
		AXIS_MAX,
	};
	EditAxis edit_axis_select = AXIS_Y;
	int edit_floor[AXIS_MAX];
	int edit_main_vp = 0;

	bool allow_viewport_override = true;
	Viewport *last_viewport = nullptr;
	EditAxis viewport_axis = edit_axis_select;

	RID active_grid_instance;
	RID grid_mesh[3];
	RID grid_instance[3];
	RID cursor_mesh;
	RID cursor_instance;

	struct ClipboardItem {
		int cell_item = 0;
		// Cell the item was copied from; needed to restore a pending move.
		Vector3i source_cell;
		// Position of the item in local space, relative to the clipboard center.
		Vector3 position;
		int orientation = 0;
		RID instance;
	};

	LocalVector<ClipboardItem> clipboard_items;
	bool clipboard_is_move = false;

	Color default_color;
	Color erase_color;
	Color pick_color;
	Ref<StandardMaterial3D> indicator_mat;
	Ref<StandardMaterial3D> cursor_inner_mat;
	Ref<StandardMaterial3D> cursor_outer_mat;
	Ref<StandardMaterial3D> inner_mat;
	Ref<StandardMaterial3D> outer_mat;

	bool updating = false;

	struct Selection {
		Vector3 begin;
		Vector3 end;
		bool active = false;
		Vector<Vector3i> cells;
	} selection;
	Selection last_selection;
	RID selection_tile_mesh;
	RID selection_multimesh;
	RID selection_multimesh_instance;

	// orientation of the paste indicator; uses orientation from GridMap
	int paste_orientation = 0;

	bool cursor_visible = false;
	Transform3D cursor_transform;

	// cell index for the pointer
	Vector3i pointer_cell;

	int display_mode = DISPLAY_THUMBNAIL;
	int selected_palette = -1;
	int cursor_rot = 0;

	enum Menu {
		MENU_OPTION_NEXT_LEVEL,
		MENU_OPTION_PREV_LEVEL,
		MENU_OPTION_LOCK_VIEW,
		MENU_OPTION_X_AXIS,
		MENU_OPTION_Y_AXIS,
		MENU_OPTION_Z_AXIS,
		MENU_OPTION_Q_AXIS,
		MENU_OPTION_R_AXIS,
		MENU_OPTION_S_AXIS,
		MENU_OPTION_ROTATE_AXIS_CW,
		MENU_OPTION_ROTATE_AXIS_CCW,
		MENU_OPTION_VIEWPORT_OVERRIDE,
		MENU_OPTION_CURSOR_ROTATE_Y,
		MENU_OPTION_CURSOR_ROTATE_X,
		MENU_OPTION_CURSOR_ROTATE_Z,
		MENU_OPTION_CURSOR_BACK_ROTATE_Y,
		MENU_OPTION_CURSOR_BACK_ROTATE_X,
		MENU_OPTION_CURSOR_BACK_ROTATE_Z,
		MENU_OPTION_CURSOR_CLEAR_ROTATION,
		MENU_OPTION_PASTE_SELECTS,
		MENU_OPTION_SELECTION_DUPLICATE,
		MENU_OPTION_SELECTION_MOVE,
		MENU_OPTION_SELECTION_CLEAR,
		MENU_OPTION_SELECTION_FILL,
		MENU_OPTION_GRIDMAP_SETTINGS

	};

	struct AreaDisplay {
		RID mesh;
		RID instance;
	};

	Tree *categories = nullptr;
	MarginContainer *item_palette_mc = nullptr;
	ItemList *mesh_library_palette = nullptr;
	Label *info_message = nullptr;

	void update_grid(); // Change which and where the grid is displayed.
	void _draw_hex_grid(RID p_mesh, const Vector3 &p_cell_size);
	void _draw_hex_r_axis_grid(RID p_mesh, const Vector3 &p_cell_size);
	void _draw_plane_grid(RID p_mesh, const Vector3 &p_axis_n1, const Vector3 &p_axis_n2, const Vector3 &p_cell_size);
	void _draw_grids(const Vector3 &p_cell_size);
	void _update_cell_shape(GridMap::CellShape p_cell_shape);
	void _update_options_menu();
	void _build_selection_meshes();
	void _menu_option(int);
	void update_palette();
	void _update_resource_preview(const String &p_path, const Ref<Texture2D> &p_preview, const Ref<Texture2D> &p_small_preview, int p_idx);
	void _update_mesh_library();
	void _set_display_mode(int p_mode);
	void _item_selected_cbk(int idx);
	void _update_cursor_transform();
	void _update_cursor_instance();
	void _on_tool_mode_changed();
	void _update_theme();

	void _text_changed(const String &p_text);
	void _mesh_library_palette_input(const Ref<InputEvent> &p_ie);

	void _icon_size_changed(float p_value);

	void _clear_clipboard_data();
	void _set_clipboard_data();
	void _update_paste_indicator();
	void _do_paste();
	void _cancel_pending_move();
	void _show_viewports_transform_gizmo(bool p_value);
	void _update_selection();
	void _set_selection(bool p_active, const Vector3 &p_begin = Vector3(), const Vector3 &p_end = Vector3());
	AABB _get_selection() const;
	bool _has_selection() const;
	Array _get_selected_cells() const;

	void _update_edit_axis();
	Vector3::Axis _get_facing_axis(const Basis &p_grid_basis, const Vector3 &p_direction) const;
	EditAxis _get_edit_axis() const { return allow_viewport_override ? viewport_axis : edit_axis_select; }

	void _view_state_changed(Node3DEditorViewport *p_viewport);

	String _get_cursor_coordinates() const;

	void _floor_changed(float p_value);
	void _floor_mouse_exited();

	void _delete_selection();
	void _delete_selection_with_undo();
	void _fill_selection();
	void _clear_selection_with_undo();
	void _setup_paste_mode();

	bool do_input_action(Camera3D *p_camera, const Point2 &p_point, bool p_click);

	friend class GridMapEditorPlugin;

	struct ItemCategoryMapping {
		AHashMap<StringName, HashSet<StringName>> category_to_category_children;
		AHashMap<StringName, HashSet<int>> category_to_items;
	};

	void _on_categories_item_activated();

	void _rebuild_categories();
	void _add_child_categories_recursive(Tree *p_categories, TreeItem *p_ti_parent, const StringName &p_category, const ItemCategoryMapping &p_mapping);

protected:
	void _notification(int p_what);
	static void _bind_methods();

	virtual void update_layout(EditorDock::DockLayout p_layout, int p_slot) override;

public:
	EditorPlugin::AfterGUIInput forward_spatial_input_event(Camera3D *p_camera, const Ref<InputEvent> &p_event);

	void edit(GridMap *p_gridmap);
	GridMapEditor();
	~GridMapEditor();
};

class GridMapEditorPlugin : public EditorPlugin {
	GDCLASS(GridMapEditorPlugin, EditorPlugin);

	GridMapEditor *grid_map_editor = nullptr;

	void _overlay_update_requested();

protected:
	void _notification(int p_what);
	static void _bind_methods();

public:
	virtual void forward_3d_draw_over_viewport(Control *p_overlay) override;
	virtual EditorPlugin::AfterGUIInput forward_3d_gui_input(Camera3D *p_camera, const Ref<InputEvent> &p_event) override { return grid_map_editor->forward_spatial_input_event(p_camera, p_event); }
	virtual String get_plugin_name() const override { return "GridMap"; }
	virtual void edit(Object *p_object) override;
	virtual bool handles(Object *p_object) const override;
	virtual void make_visible(bool p_visible) override;

	GridMap *get_current_grid_map() const;
	void set_selection(const Vector3i &p_begin, const Vector3i &p_end);
	void clear_selection();
	AABB get_selection() const;
	bool has_selection() const;
	Array get_selected_cells() const;
	void set_selected_palette_item(int p_item) const;
	int get_selected_palette_item() const;
};
