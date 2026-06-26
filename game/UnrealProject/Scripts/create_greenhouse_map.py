import math
import os
import random
import struct
import zlib

import unreal


PROJECT_MAP = "/Game/Maps/L_Greenhouse_MVP"
CUBE = "/Engine/BasicShapes/Cube.Cube"
GENERATED_TEXTURE_DIR = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    "Intermediate",
    "GeneratedTextures",
)
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OBJ_SOURCE_DIR = os.path.join(PROJECT_ROOT, "OBJ_Models")
BRICK_WALL_SOURCE = os.path.join(PROJECT_ROOT, "SourceTextures", "Walls", "brick_wall.png")
IMPORTED_MODEL_DIR = "/Game/ImportedModels"


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def ensure_folder(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def set_editor_property_if_available(obj, property_name, value):
    try:
        obj.set_editor_property(property_name, value)
    except Exception:
        pass


def write_png(path, width, height, pixels):
    def chunk(tag, data):
        return (
            struct.pack(">I", len(data))
            + tag
            + data
            + struct.pack(">I", zlib.crc32(tag + data) & 0xFFFFFFFF)
        )

    raw_rows = []
    for y in range(height):
        row = bytearray([0])
        for x in range(width):
            row.extend(pixels[y * width + x])
        raw_rows.append(bytes(row))

    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "wb") as handle:
        handle.write(b"\x89PNG\r\n\x1a\n")
        handle.write(chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0)))
        handle.write(chunk(b"IDAT", zlib.compress(b"".join(raw_rows), 9)))
        handle.write(chunk(b"IEND", b""))


def clamp_channel(value):
    return max(0, min(255, int(value)))


def make_rubber_floor_png(path, size=768):
    rng = random.Random(42)
    pixels = []
    for y in range(size):
        for x in range(size):
            seam = min(x % 192, y % 192)
            seam_line = 22 if seam < 3 else 0
            scuff = 10 * math.sin((x * 0.071) + (y * 0.037)) + 6 * math.sin((x - y) * 0.019)
            speck = rng.randint(-10, 12)
            base = 55 + scuff + speck - seam_line
            pixels.append(
                (
                    clamp_channel(base),
                    clamp_channel(base + 4),
                    clamp_channel(base + 3),
                    255,
                )
            )
    write_png(path, size, size, pixels)


def pseudo_noise(x, y, seed):
    value = (x * 73856093) ^ (y * 19349663) ^ (seed * 83492791)
    return ((value & 1023) / 1023.0) * 2.0 - 1.0


def smooth_noise(x, y, seed, scale):
    x /= scale
    y /= scale
    xi = math.floor(x)
    yi = math.floor(y)
    tx = x - xi
    ty = y - yi
    tx = tx * tx * (3 - 2 * tx)
    ty = ty * ty * (3 - 2 * ty)
    a = pseudo_noise(xi, yi, seed)
    b = pseudo_noise(xi + 1, yi, seed)
    c = pseudo_noise(xi, yi + 1, seed)
    d = pseudo_noise(xi + 1, yi + 1, seed)
    return (a * (1 - tx) + b * tx) * (1 - ty) + (c * (1 - tx) + d * tx) * ty


def make_damaged_brick_png(path, size=2048, seed=73):
    brick_w = 238
    brick_h = 104
    mortar = 11
    pixels = []

    for y in range(size):
        row = y // brick_h
        row_wobble = int(22 * math.sin(row * 1.7 + seed) + 11 * math.sin(row * 0.37))
        offset = ((brick_w // 2) + row_wobble) if row % 2 else row_wobble
        for x in range(size):
            rough_x = int(3 * math.sin(y * 0.047 + seed) + 3 * smooth_noise(x, y, seed, 38))
            rough_y = int(3 * math.sin(x * 0.039 + seed * 0.3) + 3 * smooth_noise(x, y, seed + 5, 44))
            local_x = (x + offset + rough_x) % brick_w
            local_y = (y + rough_y) % brick_h
            edge_x = min(local_x, brick_w - local_x)
            edge_y = min(local_y, brick_h - local_y)
            mortar_distance = min(edge_x, edge_y)
            mortar_mix = max(0.0, min(1.0, (mortar + 5 - mortar_distance) / 10.0))
            is_mortar = mortar_distance < mortar

            brick_index = (x + offset) // brick_w + row * 19
            brick_tint = 18 * pseudo_noise(brick_index, row, seed)
            broad_stain = 9 * math.sin(x * 0.003 + seed * 0.3) + 8 * math.sin(y * 0.004)
            grain = 10 * smooth_noise(x, y, seed + 11, 18) + 7 * smooth_noise(x, y, seed + 17, 7)
            fine_noise = 4 * math.sin(x * 0.23 + y * 0.11 + seed)
            soot = -12 * max(0.0, smooth_noise(x, y, seed + 31, 84))
            variation = broad_stain + grain + brick_tint + fine_noise + soot
            brick_color = (128 + variation, 55 + variation * 0.40, 42 + variation * 0.25)

            mortar_base = 86 + 8 * smooth_noise(x, y, seed + 41, 16) + 4 * math.sin((x + y) * 0.006)
            mortar_color = (mortar_base, mortar_base - 3, mortar_base - 8)
            if is_mortar or mortar_mix > 0.0:
                color = (
                    brick_color[0] * (1.0 - mortar_mix) + mortar_color[0] * mortar_mix,
                    brick_color[1] * (1.0 - mortar_mix) + mortar_color[1] * mortar_mix,
                    brick_color[2] * (1.0 - mortar_mix) + mortar_color[2] * mortar_mix,
                )
            else:
                color = brick_color

            pixels.append(
                (
                    clamp_channel(color[0]),
                    clamp_channel(color[1]),
                    clamp_channel(color[2]),
                    255,
                )
            )

    write_png(path, size, size, pixels)


def import_texture(name, generator):
    ensure_folder("/Game/Art/Textures")
    source_path = os.path.join(GENERATED_TEXTURE_DIR, f"{name}.png")
    generator(source_path)

    destination_path = f"/Game/Art/Textures/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(destination_path):
        unreal.EditorAssetLibrary.delete_asset(destination_path)

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
    return texture


def import_source_texture(name, source_path):
    ensure_folder("/Game/Art/Textures")
    if not os.path.exists(source_path):
        raise RuntimeError(f"Missing source texture: {source_path}")

    destination_path = f"/Game/Art/Textures/{name}"
    if unreal.EditorAssetLibrary.does_asset_exist(destination_path):
        unreal.EditorAssetLibrary.delete_asset(destination_path)

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


def make_textured_material(name, texture, roughness=0.85, u_tiling=1.0, v_tiling=1.0):
    ensure_folder("/Game/Art/Materials")
    path = f"/Game/Art/Materials/{name}"
    existing = unreal.EditorAssetLibrary.load_asset(path)
    if existing:
        unreal.EditorAssetLibrary.delete_loaded_asset(existing)

    tools = unreal.AssetToolsHelpers.get_asset_tools()
    material = tools.create_asset(name, "/Game/Art/Materials", unreal.Material, unreal.MaterialFactoryNew())

    coordinate_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionTextureCoordinate, -680, 0
    )
    coordinate_expr.set_editor_property("u_tiling", u_tiling)
    coordinate_expr.set_editor_property("v_tiling", v_tiling)

    texture_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionTextureSample, -450, 0
    )
    texture_expr.set_editor_property("texture", texture)
    unreal.MaterialEditingLibrary.connect_material_expressions(
        coordinate_expr, "", texture_expr, "UVs"
    )
    unreal.MaterialEditingLibrary.connect_material_property(
        texture_expr, "", unreal.MaterialProperty.MP_BASE_COLOR
    )

    roughness_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -400, 160
    )
    roughness_expr.set_editor_property("r", roughness)
    unreal.MaterialEditingLibrary.connect_material_property(
        roughness_expr, "", unreal.MaterialProperty.MP_ROUGHNESS
    )

    unreal.MaterialEditingLibrary.layout_material_expressions(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


def make_solid_material(name, color, roughness=0.96):
    ensure_folder("/Game/Art/Materials")
    path = f"/Game/Art/Materials/{name}"
    existing = unreal.EditorAssetLibrary.load_asset(path)
    if existing:
        unreal.EditorAssetLibrary.delete_loaded_asset(existing)

    tools = unreal.AssetToolsHelpers.get_asset_tools()
    material = tools.create_asset(name, "/Game/Art/Materials", unreal.Material, unreal.MaterialFactoryNew())

    color_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant3Vector, -420, 0
    )
    color_expr.set_editor_property("constant", unreal.LinearColor(*color))
    unreal.MaterialEditingLibrary.connect_material_property(
        color_expr, "", unreal.MaterialProperty.MP_BASE_COLOR
    )

    roughness_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -420, 170
    )
    roughness_expr.set_editor_property("r", roughness)
    unreal.MaterialEditingLibrary.connect_material_property(
        roughness_expr, "", unreal.MaterialProperty.MP_ROUGHNESS
    )

    unreal.MaterialEditingLibrary.layout_material_expressions(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


MATERIALS = {}
HALL_FOOTPRINT_SCALE = 1.25
HALL_LENGTH = int(2400 * HALL_FOOTPRINT_SCALE)
HALL_WIDTH = int(1600 * HALL_FOOTPRINT_SCALE)
WALL_THICKNESS = 120
WALL_HEIGHT = 600
FLOOR_THICKNESS = 20
WALL_SURFACE_PANEL_DEPTH = 8
WALL_PANEL_WIDTH = 240
WALL_PANEL_HEIGHT = 150


def scaled_damage_cluster(horizontal, z, width, height, seed):
    height_scale = WALL_HEIGHT / 520
    damage_scale = 1.35
    return (
        horizontal * HALL_FOOTPRINT_SCALE,
        z * height_scale,
        width * damage_scale,
        height * height_scale,
        seed,
    )


BASE_DAMAGE_CLUSTERS = {
    "back": ((-515, 270, 300, 155, 1201), (315, 395, 235, 130, 1202)),
    "front": ((-240, 310, 270, 160, 1301), (570, 190, 210, 115, 1302)),
    "left": ((-770, 345, 310, 170, 1401), (430, 225, 240, 135, 1402)),
    "right": ((-420, 410, 260, 150, 1501), (830, 285, 330, 170, 1502)),
}
DAMAGE_CLUSTERS = {
    wall: tuple(scaled_damage_cluster(*cluster) for cluster in clusters)
    for wall, clusters in BASE_DAMAGE_CLUSTERS.items()
}
MODEL_SPECS = {
    "bed": ("Props/Bed/Bed.obj", 100.0),
    "desk_setup": ("Props/Office_Desk_Setup/Office_Desk_Setup.obj", 100.0),
    "fridge": ("Props/Refrigerator/SM_Prop_Refrigerator_01.obj", 1.0),
    "oven": ("Props/Oven/Oven.obj", 100.0),
    "stovetop": ("Props/Stovetop/SM_Prop_Stovetop_01.obj", 1.0),
    "microwave": ("Props/Microwave/Microwave.obj", 100.0),
}
MODEL_UPRIGHT_ROLL = 90.0


def material(name):
    return MATERIALS[name]


def cube(label, location, scale, mat_name):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(*location),
        unreal.Rotator(0, 0, 0),
    )
    actor.set_actor_label(label)
    actor.set_actor_scale3d(unreal.Vector(*scale))
    comp = actor.get_component_by_class(unreal.StaticMeshComponent)
    comp.set_static_mesh(load_asset(CUBE))
    comp.set_editor_property("mobility", unreal.ComponentMobility.STATIC)
    actor.set_actor_enable_collision(True)
    comp.set_collision_profile_name("BlockAll")
    comp.set_material(0, material(mat_name))
    return actor


def damage_cube(label, location, scale, mat_name):
    actor = cube(label, location, scale, mat_name)
    actor.set_actor_enable_collision(False)
    comp = actor.get_component_by_class(unreal.StaticMeshComponent)
    comp.set_collision_profile_name("NoCollision")
    return actor


def imported_model_asset_path(key):
    return f"{IMPORTED_MODEL_DIR}/{key}/{key}"


def import_static_model(key, relative_obj_path, import_scale):
    destination_path = f"{IMPORTED_MODEL_DIR}/{key}"
    asset_path = imported_model_asset_path(key)
    if unreal.EditorAssetLibrary.does_directory_exist(destination_path):
        unreal.EditorAssetLibrary.delete_directory(destination_path)

    ensure_folder(IMPORTED_MODEL_DIR)
    ensure_folder(destination_path)

    source_path = os.path.join(OBJ_SOURCE_DIR, relative_obj_path)
    task = unreal.AssetImportTask()
    task.set_editor_property("filename", source_path)
    task.set_editor_property("destination_path", destination_path)
    task.set_editor_property("destination_name", key)
    task.set_editor_property("automated", True)
    task.set_editor_property("replace_existing", True)
    task.set_editor_property("save", True)

    import_ui = unreal.FbxImportUI()
    set_editor_property_if_available(import_ui, "import_mesh", True)
    set_editor_property_if_available(import_ui, "import_as_skeletal", False)
    set_editor_property_if_available(import_ui, "import_materials", True)
    set_editor_property_if_available(import_ui, "import_textures", True)
    set_editor_property_if_available(import_ui, "automated_import_should_detect_type", False)
    set_editor_property_if_available(import_ui, "mesh_type_to_import", unreal.FBXImportType.FBXIT_STATIC_MESH)
    if import_ui.static_mesh_import_data:
        set_editor_property_if_available(import_ui.static_mesh_import_data, "import_uniform_scale", import_scale)
        set_editor_property_if_available(import_ui.static_mesh_import_data, "combine_meshes", True)
        set_editor_property_if_available(import_ui.static_mesh_import_data, "generate_lightmap_u_vs", True)
    task.set_editor_property("options", import_ui)

    unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
    make_imported_materials_matte(destination_path)
    imported = unreal.EditorAssetLibrary.load_asset(asset_path)
    if imported:
        return imported

    for object_path in task.imported_object_paths:
        asset = unreal.EditorAssetLibrary.load_asset(object_path)
        if isinstance(asset, unreal.StaticMesh):
            return asset

    raise RuntimeError(f"Failed to import model: {source_path}")


def make_imported_materials_matte(destination_path):
    for asset_path in unreal.EditorAssetLibrary.list_assets(destination_path, recursive=False, include_folder=False):
        asset = unreal.EditorAssetLibrary.load_asset(asset_path)
        if isinstance(asset, unreal.MaterialInstanceConstant):
            overrides = asset.get_editor_property("base_property_overrides")
            set_editor_property_if_available(overrides, "override_roughness", True)
            set_editor_property_if_available(overrides, "roughness", 0.96)
            set_editor_property_if_available(overrides, "override_specular", True)
            set_editor_property_if_available(overrides, "specular", 0.12)
            set_editor_property_if_available(overrides, "override_metallic", True)
            set_editor_property_if_available(overrides, "metallic", 0.0)
            asset.set_editor_property("base_property_overrides", overrides)
            unreal.EditorAssetLibrary.save_loaded_asset(asset)
            continue

        if not isinstance(asset, unreal.Material):
            continue

        roughness_expr = unreal.MaterialEditingLibrary.create_material_expression(
            asset, unreal.MaterialExpressionConstant, -360, 260
        )
        roughness_expr.set_editor_property("r", 0.96)
        unreal.MaterialEditingLibrary.connect_material_property(
            roughness_expr, "", unreal.MaterialProperty.MP_ROUGHNESS
        )

        specular_expr = unreal.MaterialEditingLibrary.create_material_expression(
            asset, unreal.MaterialExpressionConstant, -360, 360
        )
        specular_expr.set_editor_property("r", 0.12)
        unreal.MaterialEditingLibrary.connect_material_property(
            specular_expr, "", unreal.MaterialProperty.MP_SPECULAR
        )

        metallic_expr = unreal.MaterialEditingLibrary.create_material_expression(
            asset, unreal.MaterialExpressionConstant, -360, 460
        )
        metallic_expr.set_editor_property("r", 0.0)
        unreal.MaterialEditingLibrary.connect_material_property(
            metallic_expr, "", unreal.MaterialProperty.MP_METALLIC
        )

        unreal.MaterialEditingLibrary.recompile_material(asset)
        unreal.EditorAssetLibrary.save_loaded_asset(asset)


def import_layout_models():
    return {
        key: import_static_model(key, relative_obj_path, import_scale)
        for key, (relative_obj_path, import_scale) in MODEL_SPECS.items()
    }


def snap_actor_to_floor(actor, floor_z=0.0):
    origin, extent = actor.get_actor_bounds(False)
    bottom_z = origin.z - extent.z
    actor.add_actor_world_offset(unreal.Vector(0.0, 0.0, floor_z - bottom_z), False, False)


def place_model(models, key, label, location, yaw=0.0, scale=1.0, snap_to_floor=True):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(*location),
        unreal.Rotator(0.0, yaw, MODEL_UPRIGHT_ROLL),
    )
    actor.set_actor_label(label)
    actor.set_actor_scale3d(unreal.Vector(scale, scale, scale))
    comp = actor.get_component_by_class(unreal.StaticMeshComponent)
    comp.set_static_mesh(models[key])
    comp.set_editor_property("mobility", unreal.ComponentMobility.STATIC)
    actor.set_actor_enable_collision(True)
    comp.set_collision_profile_name("BlockAll")
    if snap_to_floor:
        snap_actor_to_floor(actor)
    return actor


def wall_inner_plane(wall):
    half_length = HALL_LENGTH / 2
    half_width = HALL_WIDTH / 2
    if wall == "back":
        return -half_length, unreal.Vector(1.0, 0.0, 0.0)
    if wall == "front":
        return half_length, unreal.Vector(-1.0, 0.0, 0.0)
    if wall == "left":
        return -half_width, unreal.Vector(0.0, 1.0, 0.0)
    if wall == "right":
        return half_width, unreal.Vector(0.0, -1.0, 0.0)
    raise ValueError(f"Unknown wall: {wall}")


def oriented_wall_piece(label, wall, horizontal, z, width, height, depth, mat_name, inset=0.0):
    plane, normal = wall_inner_plane(wall)
    center_offset = (depth / 2) - inset
    if wall == "back":
        return damage_cube(
            label,
            (plane + normal.x * center_offset, horizontal, z),
            (depth / 100, width / 100, height / 100),
            mat_name,
        )
    if wall == "front":
        return damage_cube(
            label,
            (plane + normal.x * center_offset, horizontal, z),
            (depth / 100, width / 100, height / 100),
            mat_name,
        )
    if wall == "left":
        return damage_cube(
            label,
            (horizontal, plane + normal.y * center_offset, z),
            (width / 100, depth / 100, height / 100),
            mat_name,
        )
    if wall == "right":
        return damage_cube(
            label,
            (horizontal, plane + normal.y * center_offset, z),
            (width / 100, depth / 100, height / 100),
            mat_name,
        )


def interval_cells(start, end, step):
    cells = []
    cursor = start
    while cursor < end - 0.01:
        next_cursor = min(cursor + step, end)
        cells.append(((cursor + next_cursor) / 2, next_cursor - cursor))
        cursor = next_cursor
    return cells


def damage_strength(wall, horizontal, z):
    strength = 0.0
    for center, dz, width, height, seed in DAMAGE_CLUSTERS[wall]:
        wobble = 0.20 * smooth_noise(horizontal, z, seed, 72) + 0.09 * smooth_noise(horizontal, z, seed + 9, 31)
        normalized = ((horizontal - center) / (width / 2)) ** 2 + ((z - dz) / (height / 2)) ** 2
        strength = max(strength, 1.0 - normalized + wobble)
    return strength


def add_wall_surface(wall, length, mat_name):
    for horizontal, width in interval_cells(-length / 2, length / 2, WALL_PANEL_WIDTH):
        for z, height in interval_cells(0, WALL_HEIGHT, WALL_PANEL_HEIGHT):
            strength = damage_strength(wall, horizontal, z)
            chip_noise = 0.18 * smooth_noise(horizontal, z, 9001, 26)
            if strength + chip_noise > 0.38:
                continue

            panel_inset = max(0.0, 8.0 * max(strength, 0.0))
            oriented_wall_piece(
                f"Hall_{wall}_brick_segment_{horizontal:.0f}_{z:.0f}",
                wall,
                horizontal,
                z,
                width,
                height,
                WALL_SURFACE_PANEL_DEPTH,
                mat_name,
                panel_inset,
            )


def add_damage_cluster(wall, center, z, width, height, seed):
    rng = random.Random(seed)

    oriented_wall_piece(
        f"WallDamage_{wall}_{seed}_beige_recess_base",
        wall,
        center,
        z,
        width * 0.92,
        height * 0.80,
        6.0,
        "damage_plaster",
        22.0,
    )

    for index in range(12):
        angle = rng.uniform(0.0, math.tau)
        radius = math.sqrt(rng.uniform(0.0, 1.0))
        px = center + math.cos(angle) * radius * width * rng.uniform(0.10, 0.48)
        pz = z + math.sin(angle) * radius * height * rng.uniform(0.10, 0.46)
        edge_fade = 1.0 - min(
            1.0,
            ((px - center) / (width * 0.50)) ** 2 + ((pz - z) / (height * 0.48)) ** 2,
        )
        oriented_wall_piece(
            f"WallDamage_{wall}_{seed}_recess_tile_{index:02d}",
            wall,
            px,
            pz,
            rng.uniform(width * 0.055, width * 0.13),
            rng.uniform(height * 0.045, height * 0.12),
            rng.uniform(3.0, 7.0),
            "damage_plaster" if index < 9 else "damage_shadow",
            rng.uniform(14.0, 27.0) * max(0.45, edge_fade),
        )

    for index in range(16):
        angle = rng.uniform(0.0, math.tau)
        radius = math.sqrt(rng.uniform(0.0, 0.95))
        px = center + math.cos(angle) * radius * width * 0.45
        pz = z + math.sin(angle) * radius * height * 0.39
        oriented_wall_piece(
            f"WallDamage_{wall}_{seed}_inner_plaster_chip_{index:02d}",
            wall,
            px,
            pz,
            rng.uniform(width * 0.045, width * 0.135),
            rng.uniform(height * 0.035, height * 0.105),
            rng.uniform(3.0, 6.0),
            "damage_plaster" if index % 4 else "damage_broken_brick",
            rng.uniform(5.0, 18.0),
        )

    for index in range(10):
        angle = rng.uniform(0.0, math.tau)
        radius = rng.uniform(0.38, 1.03)
        px = center + math.cos(angle) * radius * width * 0.50
        pz = z + math.sin(angle) * radius * height * 0.48
        mat_name = "damage_shadow" if index % 3 == 0 else "damage_plaster"
        oriented_wall_piece(
            f"WallDamage_{wall}_{seed}_plaster_lip_{index:02d}",
            wall,
            px,
            pz,
            rng.uniform(width * 0.045, width * 0.14),
            rng.uniform(height * 0.035, height * 0.105),
            rng.uniform(3.0, 6.0),
            mat_name,
            rng.uniform(5.0, 15.0),
        )

    for index in range(8):
        side = rng.choice((-1, 1))
        px = center + side * rng.uniform(width * 0.34, width * 0.58)
        pz = z + rng.uniform(-height * 0.46, height * 0.46)
        oriented_wall_piece(
            f"WallDamage_{wall}_{seed}_broken_edge_{index:02d}",
            wall,
            px,
            pz,
            rng.uniform(width * 0.035, width * 0.12),
            rng.uniform(height * 0.025, height * 0.09),
            rng.uniform(4.0, 8.0),
            "damage_broken_brick",
            rng.uniform(1.5, 7.0),
        )


def set_map_game_mode():
    game_mode_class = unreal.EditorAssetLibrary.load_blueprint_class(
        "/Game/FirstPerson/Blueprints/BP_FirstPersonGameMode"
    )
    if not game_mode_class:
        unreal.log_warning("First-person GameMode not found; map will still open, but PIE may use the default pawn.")
        return

    world = unreal.EditorLevelLibrary.get_editor_world()
    world_settings = world.get_world_settings()
    world_settings.set_editor_property("default_game_mode", game_mode_class)
    world_settings.set_editor_property("force_no_precomputed_lighting", True)


def add_rectangular_hall():
    # Unreal cube base size is 100cm. This hall is 36m x 24m with tall open walls.
    hall_length = HALL_LENGTH
    hall_width = HALL_WIDTH
    wall_thickness = WALL_THICKNESS
    wall_height = WALL_HEIGHT
    floor_thickness = FLOOR_THICKNESS
    half_length = hall_length / 2
    half_width = hall_width / 2
    half_wall = wall_thickness / 2

    cube(
        "Hall_Floor_rubber_surface",
        (0, 0, -floor_thickness / 2),
        (hall_length / 100, hall_width / 100, floor_thickness / 100),
        "floor",
    )

    cube(
        "Hall_Back_wall_damaged_brick",
        (-half_length - half_wall, 0, wall_height / 2),
        (wall_thickness / 100, hall_width / 100, wall_height / 100),
        "wall_brick_short",
    )
    cube(
        "Hall_Front_wall_damaged_brick",
        (half_length + half_wall, 0, wall_height / 2),
        (wall_thickness / 100, hall_width / 100, wall_height / 100),
        "wall_brick_short",
    )
    cube(
        "Hall_Left_wall_damaged_brick",
        (0, -half_width - half_wall, wall_height / 2),
        (hall_length / 100, wall_thickness / 100, wall_height / 100),
        "wall_brick_long",
    )
    cube(
        "Hall_Right_wall_damaged_brick",
        (0, half_width + half_wall, wall_height / 2),
        (hall_length / 100, wall_thickness / 100, wall_height / 100),
        "wall_brick_long",
    )


def add_greenhouse_work_area(models):
    # Static placeholder benches for the greenhouse zone; plant gameplay stays out of this map pass.
    for index, x in enumerate((-430, 0, 430)):
        cube(
            f"Greenhouse_Workbench_{index + 1}",
            (x, 650, 47),
            (3.6, 0.72, 0.18),
            "damage_plaster",
        )
        cube(
            f"Greenhouse_Workbench_{index + 1}_Left_Leg",
            (x - 150, 650, 20),
            (0.12, 0.12, 0.40),
            "wall_core",
        )
        cube(
            f"Greenhouse_Workbench_{index + 1}_Right_Leg",
            (x + 150, 650, 20),
            (0.12, 0.12, 0.40),
            "wall_core",
        )


def add_residential_area(models):
    place_model(models, "bed", "LivingArea_Bed", (-1260, -440, 0), yaw=90)
    place_model(models, "fridge", "LivingArea_Refrigerator", (-1510, 510, 0), yaw=90)
    cube("LivingArea_Microwave_Stand", (-1390, 330, 42), (1.05, 0.62, 0.84), "damage_plaster")
    place_model(models, "microwave", "LivingArea_Microwave", (-1390, 330, 96), yaw=90, snap_to_floor=False)


def add_desk_area(models):
    place_model(models, "desk_setup", "DeskArea_Office_Desk_Setup", (-520, -760, 0), yaw=0)


def add_storage_area(models):
    shelf_positions = ((260, -210), (545, -210), (405, -465))
    for index, (x, y) in enumerate(shelf_positions):
        cube(f"StorageArea_Empty_Shelf_{index + 1}_Back_Post_L", (x - 92, y - 26, 84), (0.08, 0.08, 1.68), "wall_core")
        cube(f"StorageArea_Empty_Shelf_{index + 1}_Back_Post_R", (x + 92, y - 26, 84), (0.08, 0.08, 1.68), "wall_core")
        cube(f"StorageArea_Empty_Shelf_{index + 1}_Front_Post_L", (x - 92, y + 26, 84), (0.08, 0.08, 1.68), "wall_core")
        cube(f"StorageArea_Empty_Shelf_{index + 1}_Front_Post_R", (x + 92, y + 26, 84), (0.08, 0.08, 1.68), "wall_core")
        for level, z in enumerate((38, 96, 154), start=1):
            cube(
                f"StorageArea_Empty_Shelf_{index + 1}_Board_{level}",
                (x, y, z),
                (2.05, 0.64, 0.09),
                "damage_plaster",
            )


def add_free_utility_area(models):
    place_model(models, "oven", "UtilityArea_Oven", (1120, -575, 0), yaw=-90)
    cube("UtilityArea_Stovetop_Counter", (1120, -395, 42), (1.25, 0.84, 0.84), "damage_plaster")
    place_model(models, "stovetop", "UtilityArea_Stovetop", (1120, -395, 92), yaw=-90, snap_to_floor=False)


def add_planned_room_layout(models):
    # Model placement is handled manually while the asset set is still changing.
    # Keep the generator limited to the playable hall shell and core setup.
    return


def add_daylight():
    atmosphere = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyAtmosphere,
        unreal.Vector(0, 0, 0),
        unreal.Rotator(0, 0, 0),
    )
    atmosphere.set_actor_label("Lighting_Daytime_SkyAtmosphere")

    sky = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0, 0, 600),
        unreal.Rotator(0, 0, 0),
    )
    sky.set_actor_label("Lighting_Daytime_SkyLight")
    sky_comp = sky.get_component_by_class(unreal.SkyLightComponent)
    sky_comp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    sky_comp.set_editor_property("intensity", 1.4)

    sun = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0, 0, 800),
        unreal.Rotator(-45, -35, 0),
    )
    sun.set_actor_label("Lighting_Sun_Daylight")
    sun_comp = sun.get_component_by_class(unreal.DirectionalLightComponent)
    sun_comp.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
    sun_comp.set_editor_property("intensity", 6.0)
    sun_comp.set_editor_property("light_color", unreal.Color(255, 246, 226, 255))

    start = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(-780, 100, 95),
        unreal.Rotator(0, 0, 0),
    )
    start.set_actor_label("PlayerStart_Main_Hall")


def main():
    global MATERIALS

    ensure_folder("/Game/Maps")
    ensure_folder("/Game/Art")

    if unreal.EditorAssetLibrary.does_asset_exist(PROJECT_MAP):
        unreal.EditorLevelLibrary.load_level(PROJECT_MAP)
    else:
        unreal.EditorLevelLibrary.new_level(PROJECT_MAP)

    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        unreal.EditorLevelLibrary.destroy_actor(actor)

    MATERIALS = {
        "floor": make_textured_material(
            "M_Hall_Rubber_Floor",
            import_texture("T_Hall_Rubber_Floor", make_rubber_floor_png),
            0.96,
        ),
        "wall_back": make_textured_material(
            "M_Hall_Damaged_Brick_Wall_Back",
            import_texture("T_Hall_Damaged_Brick_Wall_Back", lambda path: make_damaged_brick_png(path, seed=73)),
            0.91,
            0.24,
            0.24,
        ),
        "wall_front": make_textured_material(
            "M_Hall_Damaged_Brick_Wall_Front",
            import_texture("T_Hall_Damaged_Brick_Wall_Front", lambda path: make_damaged_brick_png(path, seed=181)),
            0.91,
            0.24,
            0.24,
        ),
        "wall_left": make_textured_material(
            "M_Hall_Damaged_Brick_Wall_Left",
            import_texture("T_Hall_Damaged_Brick_Wall_Left", lambda path: make_damaged_brick_png(path, seed=409)),
            0.91,
            0.24,
            0.24,
        ),
        "wall_right": make_textured_material(
            "M_Hall_Damaged_Brick_Wall_Right",
            import_texture("T_Hall_Damaged_Brick_Wall_Right", lambda path: make_damaged_brick_png(path, seed=827)),
            0.91,
            0.24,
            0.24,
        ),
        "wall_brick_short": make_textured_material(
            "M_Hall_Brick_Wall_Short",
            import_source_texture("T_Hall_Brick_Wall", BRICK_WALL_SOURCE),
            0.92,
            HALL_WIDTH / WALL_HEIGHT,
            1.0,
        ),
        "wall_brick_long": make_textured_material(
            "M_Hall_Brick_Wall_Long",
            import_source_texture("T_Hall_Brick_Wall", BRICK_WALL_SOURCE),
            0.92,
            HALL_LENGTH / WALL_HEIGHT,
            1.0,
        ),
        "wall_core": make_solid_material(
            "M_Hall_Wall_Dark_Core",
            (0.045, 0.040, 0.037, 1.0),
            0.99,
        ),
        "damage_dark": make_solid_material(
            "M_Hall_Wall_Damage_Dark_Recess",
            (0.34, 0.31, 0.25, 1.0),
            0.98,
        ),
        "damage_shadow": make_solid_material(
            "M_Hall_Wall_Damage_Shadow_Plaster",
            (0.62, 0.56, 0.45, 1.0),
            0.98,
        ),
        "damage_plaster": make_solid_material(
            "M_Hall_Wall_Damage_Exposed_Plaster",
            (0.72, 0.66, 0.55, 1.0),
            0.98,
        ),
        "damage_broken_brick": make_solid_material(
            "M_Hall_Wall_Damage_Broken_Brick_Edge",
            (0.32, 0.13, 0.08, 1.0),
            0.96,
        ),
    }

    models = {}
    add_rectangular_hall()
    add_planned_room_layout(models)
    add_daylight()
    set_map_game_mode()

    actor_count = len(unreal.EditorLevelLibrary.get_all_level_actors())
    unreal.log(f"Generated map actor count: {actor_count}")

    unreal.EditorLevelLibrary.save_current_level()
    unreal.EditorAssetLibrary.save_directory("/Game/Art", only_if_is_dirty=False, recursive=True)
    unreal.log("Created planned first map: hall shell, daylight, spawn, and walking GameMode.")
    unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")


if __name__ == "__main__":
    main()
