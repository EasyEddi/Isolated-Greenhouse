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


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def ensure_folder(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


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
    brick_w = 178
    brick_h = 78
    mortar = 9
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
HALL_LENGTH = 2400
HALL_WIDTH = 1600
WALL_THICKNESS = 120
WALL_HEIGHT = 520
FLOOR_THICKNESS = 20
WALL_SURFACE_PANEL_DEPTH = 8
WALL_PANEL_WIDTH = 140
WALL_PANEL_HEIGHT = 95
DAMAGE_CLUSTERS = {
    "back": ((-515, 270, 300, 155, 1201), (315, 395, 235, 130, 1202)),
    "front": ((-240, 310, 270, 160, 1301), (570, 190, 210, 115, 1302)),
    "left": ((-770, 345, 310, 170, 1401), (430, 225, 240, 135, 1402)),
    "right": ((-420, 410, 260, 150, 1501), (830, 285, 330, 170, 1502)),
}


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
        wobble = 0.13 * smooth_noise(horizontal, z, seed, 95)
        normalized = ((horizontal - center) / (width / 2)) ** 2 + ((z - dz) / (height / 2)) ** 2
        strength = max(strength, 1.0 - normalized + wobble)
    return strength


def add_wall_surface(wall, length, mat_name):
    for horizontal, width in interval_cells(-length / 2, length / 2, WALL_PANEL_WIDTH):
        for z, height in interval_cells(0, WALL_HEIGHT, WALL_PANEL_HEIGHT):
            strength = damage_strength(wall, horizontal, z)
            if strength > 0.12:
                continue

            panel_inset = max(0.0, 5.0 * strength)
            oriented_wall_piece(
                f"Hall_{wall}_brick_segment_{horizontal:.0f}_{z:.0f}",
                wall,
                horizontal,
                z,
                width + 2,
                height + 2,
                WALL_SURFACE_PANEL_DEPTH,
                mat_name,
                panel_inset,
            )


def add_damage_cluster(wall, center, z, width, height, seed):
    rng = random.Random(seed)
    oriented_wall_piece(
        f"WallDamage_{wall}_{seed}_deep_shadow",
        wall,
        center,
        z,
        width * 0.72,
        height * 0.54,
        8.0,
        "damage_dark",
        15.0,
    )

    for index in range(14):
        piece_w = rng.uniform(width * 0.16, width * 0.42)
        piece_h = rng.uniform(height * 0.12, height * 0.28)
        px = center + rng.uniform(-width * 0.42, width * 0.42)
        pz = z + rng.uniform(-height * 0.35, height * 0.35)
        mat_name = "damage_shadow" if index < 3 else "damage_plaster"
        oriented_wall_piece(
            f"WallDamage_{wall}_{seed}_chip_{index:02d}",
            wall,
            px,
            pz,
            piece_w,
            piece_h,
            rng.uniform(4.0, 7.0),
            mat_name,
            rng.uniform(7.0, 14.0),
        )

    for index in range(10):
        piece_w = rng.uniform(width * 0.10, width * 0.22)
        piece_h = rng.uniform(height * 0.06, height * 0.15)
        px = center + rng.choice((-1, 1)) * rng.uniform(width * 0.34, width * 0.56)
        pz = z + rng.uniform(-height * 0.38, height * 0.38)
        oriented_wall_piece(
            f"WallDamage_{wall}_{seed}_broken_edge_{index:02d}",
            wall,
            px,
            pz,
            piece_w,
            piece_h,
            rng.uniform(5.0, 8.0),
            "damage_broken_brick",
            rng.uniform(2.0, 6.5),
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
    # Unreal cube base size is 100cm. This hall is 24m x 16m with tall open walls.
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
        "wall_core",
    )
    cube(
        "Hall_Front_wall_damaged_brick",
        (half_length + half_wall, 0, wall_height / 2),
        (wall_thickness / 100, hall_width / 100, wall_height / 100),
        "wall_core",
    )
    cube(
        "Hall_Left_wall_damaged_brick",
        (0, -half_width - half_wall, wall_height / 2),
        (hall_length / 100, wall_thickness / 100, wall_height / 100),
        "wall_core",
    )
    cube(
        "Hall_Right_wall_damaged_brick",
        (0, half_width + half_wall, wall_height / 2),
        (hall_length / 100, wall_thickness / 100, wall_height / 100),
        "wall_core",
    )

    add_wall_surface("back", hall_width, "wall_back")
    add_wall_surface("front", hall_width, "wall_front")
    add_wall_surface("left", hall_length, "wall_left")
    add_wall_surface("right", hall_length, "wall_right")

    for wall, clusters in DAMAGE_CLUSTERS.items():
        for cluster in clusters:
            add_damage_cluster(wall, *cluster)


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
        unreal.Vector(-520, 65, 95),
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
            5.0,
            2.0,
        ),
        "wall_front": make_textured_material(
            "M_Hall_Damaged_Brick_Wall_Front",
            import_texture("T_Hall_Damaged_Brick_Wall_Front", lambda path: make_damaged_brick_png(path, seed=181)),
            0.91,
            5.0,
            2.0,
        ),
        "wall_left": make_textured_material(
            "M_Hall_Damaged_Brick_Wall_Left",
            import_texture("T_Hall_Damaged_Brick_Wall_Left", lambda path: make_damaged_brick_png(path, seed=409)),
            0.91,
            7.0,
            2.0,
        ),
        "wall_right": make_textured_material(
            "M_Hall_Damaged_Brick_Wall_Right",
            import_texture("T_Hall_Damaged_Brick_Wall_Right", lambda path: make_damaged_brick_png(path, seed=827)),
            0.91,
            7.0,
            2.0,
        ),
        "wall_core": make_solid_material(
            "M_Hall_Wall_Dark_Core",
            (0.045, 0.040, 0.037, 1.0),
            0.99,
        ),
        "damage_dark": make_solid_material(
            "M_Hall_Wall_Damage_Dark_Recess",
            (0.055, 0.048, 0.044, 1.0),
            0.98,
        ),
        "damage_shadow": make_solid_material(
            "M_Hall_Wall_Damage_Shadow_Plaster",
            (0.22, 0.20, 0.18, 1.0),
            0.98,
        ),
        "damage_plaster": make_solid_material(
            "M_Hall_Wall_Damage_Exposed_Plaster",
            (0.50, 0.47, 0.41, 1.0),
            0.98,
        ),
        "damage_broken_brick": make_solid_material(
            "M_Hall_Wall_Damage_Broken_Brick_Edge",
            (0.32, 0.13, 0.08, 1.0),
            0.96,
        ),
    }

    add_rectangular_hall()
    add_daylight()
    set_map_game_mode()

    unreal.EditorLevelLibrary.save_current_level()
    unreal.EditorAssetLibrary.save_directory("/Game/Art", only_if_is_dirty=False, recursive=True)
    unreal.log("Created playable hall map: textured floor/walls, daylight, spawn, and walking GameMode.")
    unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")


if __name__ == "__main__":
    main()
