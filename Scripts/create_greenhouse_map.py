import unreal


PROJECT_MAP = "/Game/Maps/L_Greenhouse_MVP"
CUBE = "/Engine/BasicShapes/Cube.Cube"


def load_asset(path):
    asset = unreal.EditorAssetLibrary.load_asset(path)
    if not asset:
        raise RuntimeError(f"Missing asset: {path}")
    return asset


def ensure_folder(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


def make_material(name, color, opacity=1.0):
    ensure_folder("/Game/Art/Materials")
    path = f"/Game/Art/Materials/{name}"
    existing = unreal.EditorAssetLibrary.load_asset(path)
    if existing:
        return existing

    tools = unreal.AssetToolsHelpers.get_asset_tools()
    material = tools.create_asset(name, "/Game/Art/Materials", unreal.Material, unreal.MaterialFactoryNew())
    material.set_editor_property("two_sided", True)

    color_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant3Vector, -400, 0
    )
    color_expr.set_editor_property("constant", unreal.LinearColor(*color))
    unreal.MaterialEditingLibrary.connect_material_property(
        color_expr, "", unreal.MaterialProperty.MP_BASE_COLOR
    )

    roughness_expr = unreal.MaterialEditingLibrary.create_material_expression(
        material, unreal.MaterialExpressionConstant, -400, 160
    )
    roughness_expr.set_editor_property("r", 0.68)
    unreal.MaterialEditingLibrary.connect_material_property(
        roughness_expr, "", unreal.MaterialProperty.MP_ROUGHNESS
    )

    if opacity < 1.0:
        material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
        material.set_editor_property("translucency_lighting_mode", unreal.TranslucencyLightingMode.TLM_SURFACE)
        opacity_expr = unreal.MaterialEditingLibrary.create_material_expression(
            material, unreal.MaterialExpressionConstant, -400, 320
        )
        opacity_expr.set_editor_property("r", opacity)
        unreal.MaterialEditingLibrary.connect_material_property(
            opacity_expr, "", unreal.MaterialProperty.MP_OPACITY
        )

    unreal.MaterialEditingLibrary.layout_material_expressions(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


MATERIALS = {}


def material(name):
    return MATERIALS[name]


def cube(label, location, scale, mat_name=None, rotation=(0, 0, 0)):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(*location),
        unreal.Rotator(rotation[0], rotation[1], rotation[2]),
    )
    actor.set_actor_label(label)
    actor.set_actor_scale3d(unreal.Vector(*scale))
    comp = actor.get_component_by_class(unreal.StaticMeshComponent)
    comp.set_static_mesh(load_asset(CUBE))
    comp.set_editor_property("mobility", unreal.ComponentMobility.STATIC)
    if mat_name:
        comp.set_material(0, material(mat_name))
    return actor


def rect_light(label, location, rotation, intensity, color, width=600, height=400):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.RectLight,
        unreal.Vector(*location),
        unreal.Rotator(rotation[0], rotation[1], rotation[2]),
    )
    actor.set_actor_label(label)
    comp = actor.get_component_by_class(unreal.RectLightComponent)
    comp.set_editor_property("intensity", intensity)
    comp.set_editor_property("source_width", width)
    comp.set_editor_property("source_height", height)
    comp.set_editor_property(
        "light_color",
        unreal.Color(int(color[0] * 255), int(color[1] * 255), int(color[2] * 255), 255),
    )
    return actor


def add_greenhouse_shell():
    # Closed 24m x 16m greenhouse footprint. Cubes use 100cm base mesh dimensions.
    cube("GH_Floor_single_slab", (0, 0, -10), (24.0, 16.0, 0.2), "floor")

    cube("GH_Back_wall_solid", (-1200, 0, 220), (0.35, 16.0, 4.6), "wall")
    cube("GH_Front_wall_left", (1200, -555, 220), (0.35, 4.9, 4.6), "wall")
    cube("GH_Front_wall_right", (1200, 555, 220), (0.35, 4.9, 4.6), "wall")
    cube("GH_Front_door_header", (1200, 0, 405), (0.36, 6.2, 0.9), "wall")

    cube("GH_Left_knee_wall", (0, -800, 115), (24.0, 0.35, 2.3), "wall")
    cube("GH_Right_knee_wall", (0, 800, 115), (24.0, 0.35, 2.3), "wall")
    cube("GH_Left_glass_wall", (0, -805, 360), (24.0, 0.08, 2.65), "glass")
    cube("GH_Right_glass_wall", (0, 805, 360), (24.0, 0.08, 2.65), "glass")

    for x in [-1000, -600, -200, 200, 600, 1000]:
        cube(f"GH_Left_vertical_frame_{x}", (x, -815, 360), (0.10, 0.14, 3.15), "metal")
        cube(f"GH_Right_vertical_frame_{x}", (x, 815, 360), (0.10, 0.14, 3.15), "metal")

    for y in [-805, 805]:
        cube(f"GH_Side_lower_rail_{y}", (0, y, 235), (24.2, 0.16, 0.12), "metal")
        cube(f"GH_Side_upper_rail_{y}", (0, y, 492), (24.2, 0.16, 0.12), "metal")

    # Overlapping roof slabs remove the visible cracks from the earlier panelized roof.
    cube("GH_Roof_left_glass_plane", (0, -430, 585), (24.2, 8.85, 0.12), "glass", (0, -15, 0))
    cube("GH_Roof_right_glass_plane", (0, 430, 585), (24.2, 8.85, 0.12), "glass", (0, 15, 0))
    cube("GH_Roof_center_cap", (0, 0, 690), (24.4, 0.35, 0.22), "metal")

    for x in [-1000, -600, -200, 200, 600, 1000]:
        cube(f"GH_Roof_left_rafter_{x}", (x, -430, 605), (0.12, 9.1, 0.18), "metal", (0, -15, 0))
        cube(f"GH_Roof_right_rafter_{x}", (x, 430, 605), (0.12, 9.1, 0.18), "metal", (0, 15, 0))

    cube("GH_Back_roof_gap_fill", (-1210, 0, 555), (0.35, 16.2, 2.1), "glass")
    cube("GH_Front_roof_gap_fill", (1210, 0, 555), (0.35, 16.2, 2.1), "glass")

    cube("GH_Double_door_frame_left", (1215, -175, 185), (0.14, 0.18, 3.7), "metal")
    cube("GH_Double_door_frame_right", (1215, 175, 185), (0.14, 0.18, 3.7), "metal")
    cube("GH_Double_door_frame_top", (1215, 0, 365), (0.14, 3.7, 0.14), "metal")
    cube("GH_Double_door_glass_left", (1220, -90, 180), (0.08, 1.55, 3.35), "glass")
    cube("GH_Double_door_glass_right", (1220, 90, 180), (0.08, 1.55, 3.35), "glass")


def add_static_work_areas():
    # Empty built-in surfaces only. Plants, tools, computer, shop and shipping gameplay props are separate teammate work.
    cube("GH_Empty_north_work_bench", (-500, -500, 90), (6.0, 1.15, 0.22), "wood")
    cube("GH_Empty_south_work_bench", (430, 500, 90), (5.2, 1.15, 0.22), "wood")
    for label, x, y, length in [
        ("north", -500, -500, 6.0),
        ("south", 430, 500, 5.2),
    ]:
        half_x = (length * 100) / 2 - 24
        for dx in [-half_x, half_x]:
            for dy in [-42, 42]:
                cube(f"GH_{label}_bench_leg_{int(dx)}_{dy}", (x + dx, y + dy, 42), (0.18, 0.18, 0.84), "wood")

    cube("GH_Left_empty_wall_shelf", (-450, -720, 185), (8.0, 0.65, 0.16), "metal")
    cube("GH_Right_empty_wall_shelf", (450, 720, 185), (8.0, 0.65, 0.16), "metal")
    cube("GH_Clear_center_walkway", (0, 0, 1.5), (18.0, 1.05, 0.03), "path")


def add_lighting_and_start():
    sky = unreal.EditorLevelLibrary.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 650), unreal.Rotator(0, 0, 0))
    sky.set_actor_label("Lighting_Skylight_soft_greenhouse")
    sky_comp = sky.get_component_by_class(unreal.SkyLightComponent)
    sky_comp.set_editor_property("intensity", 1.1)

    sun = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0, 0, 900),
        unreal.Rotator(-42, -34, 0),
    )
    sun.set_actor_label("Lighting_Warm_morning_sun")
    sun_comp = sun.get_component_by_class(unreal.DirectionalLightComponent)
    sun_comp.set_editor_property("intensity", 3.2)
    sun_comp.set_editor_property("light_color", unreal.Color(255, 229, 190, 255))

    rect_light("Lighting_Left_roof_soft_panel", (-250, -270, 555), (-75, 0, 0), 180, (1.0, 0.82, 0.62), 800, 260)
    rect_light("Lighting_Right_roof_soft_panel", (250, 270, 555), (-75, 0, 0), 180, (0.72, 0.90, 0.82), 800, 260)

    start = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(980, 0, 95),
        unreal.Rotator(0, 180, 0),
    )
    start.set_actor_label("PlayerStart_Greenhouse_Entrance")


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
        "floor": make_material("M_Warm_Concrete", (0.46, 0.42, 0.35, 1)),
        "wall": make_material("M_Soft_Off_White", (0.78, 0.77, 0.68, 1)),
        "glass": make_material("M_Greenhouse_Glass", (0.55, 0.83, 0.78, 1), 0.34),
        "metal": make_material("M_Dark_Green_Metal", (0.08, 0.18, 0.14, 1)),
        "wood": make_material("M_Workbench_Wood", (0.45, 0.30, 0.18, 1)),
        "path": make_material("M_Subtle_Path_Marker", (0.50, 0.68, 0.43, 1)),
    }

    add_greenhouse_shell()
    add_static_work_areas()
    add_lighting_and_start()

    unreal.EditorLevelLibrary.save_current_level()
    unreal.EditorAssetLibrary.save_directory("/Game/Art", only_if_is_dirty=False, recursive=True)
    unreal.log("Created static greenhouse map: closed shell, empty work areas, no plants/tools/gameplay props.")


if __name__ == "__main__":
    main()
