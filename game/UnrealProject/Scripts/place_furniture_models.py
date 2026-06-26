import math
import os

import unreal


PROJECT_MAP = "/Game/Maps/L_Greenhouse_MVP"
ACTOR_PREFIX = "MVP_Furniture_"
BED_LABEL = ACTOR_PREFIX + "Living_Bed"
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BRICK_WALL_SOURCE = os.path.join(PROJECT_ROOT, "SourceTextures", "Walls", "brick_wall.png")
CUBE = "/Engine/BasicShapes/Cube.Cube"

MAIN_LENGTH = 2350.0
MAIN_WIDTH = 1500.0
SIDE_LENGTH = 750.0
SIDE_WIDTH = 1000.0
WALL_HEIGHT = 600.0
WALL_THICKNESS = 120.0
FLOOR_THICKNESS = 20.0

MAIN_X_MIN = -MAIN_LENGTH / 2.0
MAIN_X_MAX = MAIN_LENGTH / 2.0
SIDE_X_MIN = MAIN_X_MIN - SIDE_LENGTH
Y_BOTTOM = -50.0
Y_TOP = Y_BOTTOM + MAIN_WIDTH
Y_SIDE_BOTTOM = Y_TOP - SIDE_WIDTH


FURNITURE = [
    {
        "key": "fridge",
        "asset": "/Game/models/furniture/fridge/fridge",
        "label": "Living_Fridge",
        "location": (-1879.589, 509.219, 4.077),
        "rotation": (0.0, 90.0, 0.0),
        "scale3d": (0.020383, 0.020383, 0.020383),
        "snap_to_floor": False,
    },
    {
        "key": "lower_kitchen_cabinet_01",
        "asset": "/Game/models/furniture/Lower_Kitchen_Cabinet/Lower_Kitchen_Cabinet",
        "label": "Living_Lower_Kitchen_Cabinet_01",
        "location": (-1890.222, 651.661, 0.0),
        "rotation": (0.0, -90.0, 0.0),
        "scale3d": (1.390835, 1.390835, 1.390835),
        "snap_to_floor": False,
    },
    {
        "key": "lower_kitchen_cabinet_02",
        "asset": "/Game/models/furniture/Lower_Kitchen_Cabinet/Lower_Kitchen_Cabinet",
        "label": "Living_Lower_Kitchen_Cabinet_02",
        "location": (-1890.222, 816.197, 0.0),
        "rotation": (0.0, -90.0, 0.0),
        "scale3d": (1.390835, 1.390835, 1.390835),
        "snap_to_floor": False,
    },
    {
        "key": "oven",
        "asset": "/Game/models/furniture/Oven/oven",
        "label": "Living_Oven",
        "location": (-1885.223, 961.317, 0.0),
        "rotation": (0.0, -90.0, 0.0),
        "scale3d": (0.950276, 0.866365, 0.968646),
        "snap_to_floor": False,
    },
    {
        "key": "microwave",
        "asset": "/Game/models/furniture/Microwave/microwave",
        "label": "Living_Microwave",
        "location": (-1890.222, 612.931, 129.139),
        "rotation": (0.0, -90.0, 0.0),
        "scale3d": (0.765625, 0.765625, 0.765625),
        "snap_to_floor": False,
    },
    {
        "key": "desk_setup",
        "asset": "/Game/models/furniture/desk+setup/desk+setup",
        "label": "Desk_Office_Setup",
        "location": (-770.452, 1374.016, 0.0),
        "rotation": (0.0, 180.0, 0.0),
        "scale3d": (1.091453, 1.091453, 1.091453),
        "snap_to_floor": False,
    },
    {
        "key": "storage_shelf_01",
        "asset": "/Game/models/furniture/Storage_Shelf/storge_shelf",
        "label": "Storage_Shelf_01",
        "location": (-377.315, 739.827, 0.0),
        "rotation": (0.0, 90.0, 0.0),
        "scale3d": (0.620174, 0.620174, 0.620174),
        "snap_to_floor": False,
    },
]


def load_mesh(path):
    mesh = unreal.EditorAssetLibrary.load_asset(path)
    if not mesh:
        raise RuntimeError(f"Missing mesh asset: {path}")
    if not isinstance(mesh, unreal.StaticMesh):
        raise RuntimeError(f"Asset is not a StaticMesh: {path}")
    return mesh


def mesh_size(mesh):
    bounds = mesh.get_bounds()
    extent = bounds.box_extent
    return extent.x * 2.0, extent.y * 2.0, extent.z * 2.0


def scale_for_spec(mesh, spec):
    size_x, size_y, size_z = mesh_size(mesh)
    candidates = []

    if spec.get("target_height") and size_z > 0.0:
        candidates.append(spec["target_height"] / size_z)

    max_xy = max(size_x, size_y)
    if spec.get("target_max_xy") and max_xy > 0.0:
        candidates.append(spec["target_max_xy"] / max_xy)

    if not candidates:
        return spec.get("scale", 1.0)

    return min(candidates)


def snap_actor_to_floor(actor, floor_z=0.0):
    origin, extent = actor.get_actor_bounds(False)
    bottom_z = origin.z - extent.z
    actor.add_actor_world_offset(unreal.Vector(0.0, 0.0, floor_z - bottom_z), False, False)


def ensure_folder(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def set_editor_property_if_available(obj, property_name, value):
    try:
        obj.set_editor_property(property_name, value)
    except Exception:
        pass


def import_source_texture(name, source_path):
    ensure_folder("/Game/Art/Textures")
    if not os.path.exists(source_path):
        raise RuntimeError(f"Missing source texture: {source_path}")

    destination_path = f"/Game/Art/Textures/{name}"
    texture = unreal.EditorAssetLibrary.load_asset(destination_path)
    if not texture:
        task = unreal.AssetImportTask()
        task.set_editor_property("filename", source_path)
        task.set_editor_property("destination_path", "/Game/Art/Textures")
        task.set_editor_property("destination_name", name)
        task.set_editor_property("automated", True)
        task.set_editor_property("replace_existing", True)
        task.set_editor_property("save", True)
        unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
        texture = unreal.EditorAssetLibrary.load_asset(destination_path)

    if not texture:
        raise RuntimeError(f"Failed to import texture: {source_path}")

    set_editor_property_if_available(texture, "address_x", unreal.TextureAddress.TA_MIRROR)
    set_editor_property_if_available(texture, "address_y", unreal.TextureAddress.TA_CLAMP)
    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    return texture


def make_textured_material(name, texture, roughness, u_tiling, v_tiling):
    ensure_folder("/Game/Art/Materials")
    path = f"/Game/Art/Materials/{name}"
    existing = unreal.EditorAssetLibrary.load_asset(path)
    if existing:
        unreal.EditorAssetLibrary.delete_loaded_asset(existing)

    material = unreal.AssetToolsHelpers.get_asset_tools().create_asset(
        name,
        "/Game/Art/Materials",
        unreal.Material,
        unreal.MaterialFactoryNew(),
    )

    coordinate_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionTextureCoordinate, -680, 0
    )
    coordinate_expr.set_editor_property("u_tiling", u_tiling)
    coordinate_expr.set_editor_property("v_tiling", v_tiling)

    texture_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionTextureSample, -450, 0
    )
    texture_expr.set_editor_property("texture", texture)
    unreal.MaterialEditingLibrary.connect_material_expressions(coordinate_expr, "", texture_expr, "UVs")
    unreal.MaterialEditingLibrary.connect_material_property(
        texture_expr,
        "",
        unreal.MaterialProperty.MP_BASE_COLOR,
    )

    roughness_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -400, 160
    )
    roughness_expr.set_editor_property("r", roughness)
    unreal.MaterialEditingLibrary.connect_material_property(
        roughness_expr,
        "",
        unreal.MaterialProperty.MP_ROUGHNESS,
    )

    unreal.MaterialEditingLibrary.layout_material_expressions(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


def resize_actor(label, location, scale, material=None):
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        if actor.get_actor_label() != label:
            continue

        resize_static_mesh_actor(actor, location, scale, material)
        return True

    unreal.log_warning(f"Hall actor not found for resize: {label}")
    return False


def destroy_generated_hall_shell():
    removed = 0
    for actor in list(unreal.EditorLevelLibrary.get_all_level_actors()):
        label = actor.get_actor_label()
        if label.startswith("Hall_") or label.startswith("WallDamage_"):
            unreal.EditorLevelLibrary.destroy_actor(actor)
            removed += 1
    unreal.log(f"Removed generated hall shell actors: {removed}")


def resize_static_mesh_actor(actor, location, scale, material=None):
    actor.set_actor_location(unreal.Vector(*location), False, False)
    actor.set_actor_scale3d(unreal.Vector(*scale))

    if material:
        component = actor.get_component_by_class(unreal.StaticMeshComponent)
        if component:
            component.set_material(0, material)


def get_component_material_names(component):
    names = []
    if not component:
        return names

    for index in range(component.get_num_materials()):
        material = component.get_material(index)
        if material:
            names.append(material.get_path_name().lower())
    return names


def find_floor_actor():
    actors = unreal.EditorLevelLibrary.get_all_level_actors()
    label_matches = ("hall_rubber_floor", "hall_floor", "floor")

    for actor in actors:
        if actor.get_actor_label().lower() in label_matches:
            return actor

    for actor in actors:
        label = actor.get_actor_label().lower()
        component = actor.get_component_by_class(unreal.StaticMeshComponent)
        material_names = get_component_material_names(component)
        if "floor" in label or any("hall_rubber_floor" in name for name in material_names):
            return actor

    for actor in actors:
        component = actor.get_component_by_class(unreal.StaticMeshComponent)
        if not component:
            continue

        scale = actor.get_actor_scale3d()
        if scale.x >= 20.0 and scale.y >= 15.0 and scale.z <= 0.5:
            return actor

    return None


def resize_floor():
    floor = find_floor_actor()
    if not floor:
        unreal.log_warning("Hall floor actor not found for resize")
        return False

    resize_static_mesh_actor(
        floor,
        (0.0, 0.0, -FLOOR_THICKNESS / 2.0),
        (HALL_LENGTH / 100.0, HALL_WIDTH / 100.0, FLOOR_THICKNESS / 100.0),
    )
    unreal.log(f"Resized hall floor actor: {floor.get_actor_label()}")
    return True


def material_for_wall_length(name, texture, length):
    return make_textured_material(
        name,
        texture,
        0.92,
        length / WALL_HEIGHT,
        1.0,
    )


def load_material(path):
    material = unreal.EditorAssetLibrary.load_asset(path)
    if not material:
        unreal.log_warning(f"Missing material: {path}")
    return material


def spawn_cube(label, location, scale, material=None):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(*location),
        unreal.Rotator(0.0, 0.0, 0.0),
    )
    actor.set_actor_label(label)
    actor.set_actor_scale3d(unreal.Vector(*scale))

    component = actor.get_component_by_class(unreal.StaticMeshComponent)
    component.set_static_mesh(load_mesh(CUBE))
    component.set_editor_property("mobility", unreal.ComponentMobility.STATIC)
    component.set_collision_profile_name("BlockAll")
    actor.set_actor_enable_collision(True)
    if material:
        component.set_material(0, material)
    return actor


def spawn_floor_piece(label, center_x, center_y, length, width, material):
    return spawn_cube(
        label,
        (center_x, center_y, -FLOOR_THICKNESS / 2.0),
        (length / 100.0, width / 100.0, FLOOR_THICKNESS / 100.0),
        material,
    )


def spawn_horizontal_wall(label, center_x, y_inner, outward_sign, length, material):
    half_wall = WALL_THICKNESS / 2.0
    return spawn_cube(
        label,
        (center_x, y_inner + outward_sign * half_wall, WALL_HEIGHT / 2.0),
        (length / 100.0, WALL_THICKNESS / 100.0, WALL_HEIGHT / 100.0),
        material,
    )


def spawn_vertical_wall(label, x_inner, outward_sign, center_y, length, material):
    half_wall = WALL_THICKNESS / 2.0
    return spawn_cube(
        label,
        (x_inner + outward_sign * half_wall, center_y, WALL_HEIGHT / 2.0),
        (WALL_THICKNESS / 100.0, length / 100.0, WALL_HEIGHT / 100.0),
        material,
    )


def resize_hall_shell():
    texture = import_source_texture("T_Hall_Brick_Wall", BRICK_WALL_SOURCE)
    floor_material = load_material("/Game/Art/Materials/M_Hall_Rubber_Floor")
    wall_materials = {
        2350.0: material_for_wall_length("M_Hall_Brick_Wall_2350", texture, 2350.0),
        1500.0: material_for_wall_length("M_Hall_Brick_Wall_1500", texture, 1500.0),
        1000.0: material_for_wall_length("M_Hall_Brick_Wall_1000", texture, 1000.0),
        750.0: material_for_wall_length("M_Hall_Brick_Wall_750", texture, 750.0),
        500.0: material_for_wall_length("M_Hall_Brick_Wall_500", texture, 500.0),
    }

    destroy_generated_hall_shell()

    spawn_floor_piece(
        "Hall_Floor_main_2350x1500",
        (MAIN_X_MIN + MAIN_X_MAX) / 2.0,
        (Y_BOTTOM + Y_TOP) / 2.0,
        MAIN_LENGTH,
        MAIN_WIDTH,
        floor_material,
    )
    spawn_floor_piece(
        "Hall_Floor_side_750x1000",
        (SIDE_X_MIN + MAIN_X_MIN) / 2.0,
        (Y_SIDE_BOTTOM + Y_TOP) / 2.0,
        SIDE_LENGTH,
        SIDE_WIDTH,
        floor_material,
    )

    spawn_horizontal_wall(
        "Hall_Wall_bottom_green_750",
        (SIDE_X_MIN + MAIN_X_MIN) / 2.0,
        Y_SIDE_BOTTOM,
        -1.0,
        SIDE_LENGTH,
        wall_materials[750.0],
    )
    spawn_horizontal_wall(
        "Hall_Wall_bottom_red_2350",
        (MAIN_X_MIN + MAIN_X_MAX) / 2.0,
        Y_BOTTOM,
        -1.0,
        MAIN_LENGTH,
        wall_materials[2350.0],
    )
    spawn_vertical_wall(
        "Hall_Wall_right_brown_1500",
        MAIN_X_MAX,
        1.0,
        (Y_BOTTOM + Y_TOP) / 2.0,
        MAIN_WIDTH,
        wall_materials[1500.0],
    )
    spawn_horizontal_wall(
        "Hall_Wall_top_red_2350",
        (MAIN_X_MIN + MAIN_X_MAX) / 2.0,
        Y_TOP,
        1.0,
        MAIN_LENGTH,
        wall_materials[2350.0],
    )
    spawn_vertical_wall(
        "Hall_Wall_cyan_500",
        MAIN_X_MIN,
        -1.0,
        (Y_BOTTOM + Y_SIDE_BOTTOM) / 2.0,
        Y_SIDE_BOTTOM - Y_BOTTOM,
        wall_materials[500.0],
    )
    spawn_horizontal_wall(
        "Hall_Wall_top_green_750",
        (SIDE_X_MIN + MAIN_X_MIN) / 2.0,
        Y_TOP,
        1.0,
        SIDE_LENGTH,
        wall_materials[750.0],
    )
    spawn_vertical_wall(
        "Hall_Wall_left_blue_1000",
        SIDE_X_MIN,
        -1.0,
        (Y_SIDE_BOTTOM + Y_TOP) / 2.0,
        SIDE_WIDTH,
        wall_materials[1000.0],
    )

    unreal.log("Rebuilt hall shell from floor plan: main=23.5mx15m side=7.5mx10m cyan=5m")


def yaw_rotation(yaw):
    rotation = unreal.Rotator()
    rotation.set_editor_property("pitch", 0.0)
    rotation.set_editor_property("yaw", yaw)
    rotation.set_editor_property("roll", 0.0)
    return rotation


def actor_rotation(spec):
    pitch, yaw, roll = spec.get("rotation", (0.0, spec.get("yaw", 0.0), 0.0))
    rotation = unreal.Rotator()
    rotation.set_editor_property("pitch", pitch)
    rotation.set_editor_property("yaw", yaw)
    rotation.set_editor_property("roll", roll)
    return rotation


def remove_previous_generated_furniture():
    removed = 0
    for actor in list(unreal.EditorLevelLibrary.get_all_level_actors()):
        if actor.get_actor_label().startswith(ACTOR_PREFIX) and actor.get_actor_label() != BED_LABEL:
            unreal.EditorLevelLibrary.destroy_actor(actor)
            removed += 1
    return removed


def place_actor(spec):
    mesh = load_mesh(spec["asset"])
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(*spec["location"]),
        actor_rotation(spec),
    )
    actor.set_actor_label(ACTOR_PREFIX + spec["label"])

    scale3d = spec.get("scale3d")
    if scale3d:
        actor.set_actor_scale3d(unreal.Vector(*scale3d))
        scale_log = ",".join(f"{value:.5f}" for value in scale3d)
    else:
        scale = scale_for_spec(mesh, spec)
        actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
        scale_log = f"{scale:.5f}"

    component = actor.get_component_by_class(unreal.StaticMeshComponent)
    component.set_static_mesh(mesh)
    component.set_editor_property("mobility", unreal.ComponentMobility.STATIC)
    component.set_collision_profile_name("BlockAll")
    actor.set_actor_enable_collision(True)

    if spec.get("snap_to_floor", True):
        snap_actor_to_floor(actor)

    origin, extent = actor.get_actor_bounds(False)
    unreal.log(
        f"PLACED_FURNITURE {actor.get_actor_label()} scale={scale_log} "
        f"bounds_cm=({extent.x * 2.0:.1f},{extent.y * 2.0:.1f},{extent.z * 2.0:.1f}) "
        f"loc=({actor.get_actor_location().x:.1f},{actor.get_actor_location().y:.1f},{actor.get_actor_location().z:.1f})"
    )
    return actor


def ensure_player_start():
    for actor in list(unreal.EditorLevelLibrary.get_all_level_actors()):
        if actor.get_actor_label() == "PlayerStart_Main_Hall" or isinstance(actor, unreal.PlayerStart):
            unreal.EditorLevelLibrary.destroy_actor(actor)

    start = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(-900.0, 650.0, 100.0),
        unreal.Rotator(0.0, -90.0, 0.0),
    )
    start.set_actor_label("PlayerStart_Main_Hall")
    unreal.log("Added PlayerStart_Main_Hall at (-900, 650, 100)")


def main():
    unreal.EditorLevelLibrary.load_level(PROJECT_MAP)

    resize_hall_shell()
    removed = remove_previous_generated_furniture()

    placed = 0
    for spec in FURNITURE:
        place_actor(spec)
        placed += 1

    ensure_player_start()

    actor_count = len(unreal.EditorLevelLibrary.get_all_level_actors())
    unreal.log(f"Removed previous generated furniture actors: {removed}")
    unreal.log(f"Placed furniture actors: {placed}")
    unreal.log(f"Map actor count after furniture placement: {actor_count}")

    unreal.EditorLevelLibrary.save_current_level()
    unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")


if __name__ == "__main__":
    main()
