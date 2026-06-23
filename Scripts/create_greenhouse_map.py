import math

import unreal


PROJECT_MAP = "/Game/Maps/L_Greenhouse_MVP"
CUBE = "/Engine/BasicShapes/Cube.Cube"
SPHERE = "/Engine/BasicShapes/Sphere.Sphere"
CYLINDER = "/Engine/BasicShapes/Cylinder.Cylinder"


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
    material.set_editor_property("use_material_attributes", False)
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
    roughness_expr.set_editor_property("r", 0.72)
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


def spawn_mesh(label, mesh_path, location, scale, mat_name=None, rotation=(0, 0, 0)):
    mesh = load_asset(mesh_path)
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.StaticMeshActor,
        unreal.Vector(*location),
        unreal.Rotator(rotation[0], rotation[1], rotation[2]),
    )
    actor.set_actor_label(label)
    actor.set_actor_scale3d(unreal.Vector(*scale))
    comp = actor.get_component_by_class(unreal.StaticMeshComponent)
    comp.set_static_mesh(mesh)
    comp.set_editor_property("mobility", unreal.ComponentMobility.STATIC)
    if mat_name:
        comp.set_material(0, material(mat_name))
    return actor


def cube(label, location, scale, mat_name=None, rotation=(0, 0, 0)):
    return spawn_mesh(label, CUBE, location, scale, mat_name, rotation)


def cylinder(label, location, scale, mat_name=None, rotation=(0, 0, 0)):
    return spawn_mesh(label, CYLINDER, location, scale, mat_name, rotation)


def sphere(label, location, scale, mat_name=None, rotation=(0, 0, 0)):
    return spawn_mesh(label, SPHERE, location, scale, mat_name, rotation)


def text(label, body, location, size=42, rotation=(0, 0, 0), color=(0.85, 0.95, 0.82, 1)):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.TextRenderActor,
        unreal.Vector(*location),
        unreal.Rotator(rotation[0], rotation[1], rotation[2]),
    )
    actor.set_actor_label(label)
    comp = actor.get_component_by_class(unreal.TextRenderComponent)
    comp.set_text(body)
    comp.set_horizontal_alignment(unreal.HorizTextAligment.EHTA_CENTER)
    comp.set_world_size(size)
    comp.set_text_render_color(unreal.Color(
        int(color[0] * 255), int(color[1] * 255), int(color[2] * 255), int(color[3] * 255)
    ))
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
    comp.set_editor_property("light_color", unreal.Color(
        int(color[0] * 255), int(color[1] * 255), int(color[2] * 255), 255
    ))
    comp.set_editor_property("source_width", width)
    comp.set_editor_property("source_height", height)
    return actor


def point_light(label, location, intensity, radius, color):
    actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PointLight,
        unreal.Vector(*location),
        unreal.Rotator(0, 0, 0),
    )
    actor.set_actor_label(label)
    comp = actor.get_component_by_class(unreal.PointLightComponent)
    comp.set_editor_property("intensity", intensity)
    comp.set_editor_property("attenuation_radius", radius)
    comp.set_editor_property("light_color", unreal.Color(
        int(color[0] * 255), int(color[1] * 255), int(color[2] * 255), 255
    ))
    return actor


def add_greenhouse_shell():
    cube("GH_Floor_warm_concrete", (0, 0, -10), (24, 16, 0.2), "floor")
    cube("GH_Back_wall_plaster", (-1150, 0, 250), (0.25, 16, 5), "wall")
    cube("GH_Left_low_wall", (0, -780, 165), (24, 0.25, 3.3), "wall")
    cube("GH_Right_low_wall", (0, 780, 165), (24, 0.25, 3.3), "wall")
    cube("GH_Front_entry_wall_left", (900, -500, 200), (5, 0.25, 4), "wall", (0, 0, 90))
    cube("GH_Front_entry_wall_right", (900, 500, 200), (5, 0.25, 4), "wall", (0, 0, 90))

    for y in [-520, 0, 520]:
        cube(f"GH_Glass_roof_panel_{y}", (0, y, 555), (22, 3.8, 0.08), "glass", (0, 18, 0))
        cube(f"GH_Roof_metal_rib_{y}", (0, y, 580), (22.5, 0.08, 0.12), "metal", (0, 18, 0))

    for x in [-900, -450, 0, 450, 900]:
        cube(f"GH_Left_window_frame_{x}", (x, -780, 410), (0.08, 0.18, 3), "metal")
        cube(f"GH_Right_window_frame_{x}", (x, 780, 410), (0.08, 0.18, 3), "metal")
        cube(f"GH_Glass_wall_left_{x}", (x, -790, 390), (3.6, 0.05, 2.6), "glass")
        cube(f"GH_Glass_wall_right_{x}", (x, 790, 390), (3.6, 0.05, 2.6), "glass")

    cube("GH_Double_door_frame", (1120, 0, 180), (0.15, 3.4, 3.6), "metal")
    cube("GH_Double_door_glass_left", (1130, -95, 170), (0.08, 1.35, 3.0), "glass")
    cube("GH_Double_door_glass_right", (1130, 95, 170), (0.08, 1.35, 3.0), "glass")


def add_work_zones():
    cube("Worktable_main_wood_top", (-300, -250, 110), (5.4, 1.65, 0.18), "wood")
    for x in [-530, -70]:
        for y in [-315, -185]:
            cube(f"Worktable_leg_{x}_{y}", (x, y, 55), (0.16, 0.16, 1.05), "wood")

    cube("Computer_desk_top_order_station", (-620, 420, 105), (3.4, 1.35, 0.18), "wood")
    for x in [-760, -480]:
        for y in [365, 475]:
            cube(f"Computer_desk_leg_{x}_{y}", (x, y, 55), (0.14, 0.14, 1), "wood")
    cube("Computer_monitor_active_order", (-620, 375, 175), (1.25, 0.08, 0.75), "screen")
    cube("Computer_keyboard", (-620, 305, 124), (1.45, 0.45, 0.06), "dark")
    text("Label_Computer_OrderUI", "ORDER COMPUTER\n3 MVP ORDERS", (-620, 292, 225), 30, (0, 0, 0))

    cube("Packstation_counter", (600, 420, 100), (3.8, 1.55, 0.18), "wood")
    cube("Packstation_box_open", (520, 400, 150), (1.15, 0.9, 0.38), "cardboard")
    cube("Packstation_box_lid_left", (460, 400, 185), (0.08, 0.9, 0.32), "cardboard", (0, 0, 22))
    cube("Packstation_label_panel", (655, 340, 150), (1.4, 0.08, 0.55), "screen")
    text("Label_Packstation", "PACK + SHIP\nOFFSHOOTS", (600, 325, 215), 32, (0, 0, 0))

    for i, x in enumerate([-850, -600, -350, -100, 150, 400, 650]):
        cube(f"Floor_Path_Marker_{i}", (x, 0, 2), (1.15, 0.18, 0.025), "path")


def add_plants():
    cylinder("Monstera_main_pot", (-320, -250, 150), (0.58, 0.58, 0.58), "pot")
    cylinder("Monstera_soil", (-320, -250, 183), (0.5, 0.5, 0.06), "soil")
    cylinder("Monstera_stem_cluster", (-320, -250, 235), (0.08, 0.08, 1.05), "stem")

    leaf_angles = [-75, -40, 0, 35, 70, 115]
    for i, yaw in enumerate(leaf_angles):
        radius = 72 + (i % 2) * 22
        x = -320 + math.cos(math.radians(yaw)) * radius
        y = -250 + math.sin(math.radians(yaw)) * radius
        z = 260 + (i % 3) * 22
        sphere(f"Monstera_leaf_ready_{i}", (x, y, z), (0.72, 0.26, 0.055), "leaf", (18, yaw, 0))
    text("Label_Monstera", "MONSTERA\nMOTHER PLANT", (-320, -505, 250), 34, (0, 0, 0))

    shelf_xs = [-760, -520, -280, -40, 200, 440]
    for row, y in enumerate([-610, 610]):
        cube(f"Plant_shelf_{row}_bottom", (-270, y, 95), (10.8, 0.55, 0.12), "metal")
        cube(f"Plant_shelf_{row}_top", (-270, y, 205), (10.8, 0.55, 0.12), "metal")
        for x in shelf_xs:
            cylinder(f"Small_pot_{row}_{x}", (x, y, 130), (0.32, 0.32, 0.32), "pot")
            sphere(f"Small_plant_{row}_{x}", (x, y, 180), (0.38, 0.24, 0.2), "leaf")


def add_lighting_and_markers():
    sky = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.SkyLight,
        unreal.Vector(0, 0, 650),
        unreal.Rotator(0, 0, 0),
    )
    sky.set_actor_label("Lighting_Skylight_soft_greenhouse")
    sky_comp = sky.get_component_by_class(unreal.SkyLightComponent)
    sky_comp.set_editor_property("intensity", 1.2)

    sun = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.DirectionalLight,
        unreal.Vector(0, 0, 900),
        unreal.Rotator(-38, -32, 0),
    )
    sun.set_actor_label("Lighting_Warm_late_morning_sun")
    sun_comp = sun.get_component_by_class(unreal.DirectionalLightComponent)
    sun_comp.set_editor_property("intensity", 3.0)
    sun_comp.set_editor_property("light_color", unreal.Color(255, 226, 184, 255))

    rect_light("Lighting_Worktable_warm_panel", (-300, -240, 460), (-90, 0, 0), 350, (1.0, 0.78, 0.52), 420, 260)
    rect_light("Lighting_Computer_soft_panel", (-620, 420, 330), (-80, 0, 0), 180, (0.72, 0.95, 1.0), 260, 170)
    point_light("Lighting_Packstation_status_glow", (640, 350, 210), 180, 280, (0.55, 0.9, 0.7))

    pawn = unreal.EditorLevelLibrary.spawn_actor_from_class(
        unreal.PlayerStart,
        unreal.Vector(900, 0, 95),
        unreal.Rotator(0, 180, 0),
    )
    pawn.set_actor_label("PlayerStart_Facing_MVP_Workflow")

    text("Label_MainLoop", "CARE PLANT  ->  CUT OFFSHOOT  ->  PACK ORDER", (20, 0, 335), 42, (0, 180, 0))


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
        "glass": make_material("M_Greenhouse_Glass", (0.55, 0.83, 0.78, 1), 0.32),
        "metal": make_material("M_Dark_Green_Metal", (0.08, 0.18, 0.14, 1)),
        "wood": make_material("M_Workbench_Wood", (0.45, 0.30, 0.18, 1)),
        "leaf": make_material("M_Plant_Leaf", (0.08, 0.38, 0.18, 1)),
        "stem": make_material("M_Plant_Stem", (0.14, 0.42, 0.16, 1)),
        "pot": make_material("M_Terracotta_Pot", (0.55, 0.23, 0.13, 1)),
        "soil": make_material("M_Dark_Soil", (0.09, 0.06, 0.035, 1)),
        "screen": make_material("M_Soft_Blue_Screen", (0.18, 0.48, 0.58, 1)),
        "dark": make_material("M_Warm_Dark_Detail", (0.035, 0.04, 0.04, 1)),
        "cardboard": make_material("M_Cardboard", (0.54, 0.39, 0.22, 1)),
        "path": make_material("M_Subtle_Path_Marker", (0.50, 0.68, 0.43, 1)),
    }

    add_greenhouse_shell()
    add_work_zones()
    add_plants()
    add_lighting_and_markers()

    unreal.EditorLevelLibrary.save_current_level()
    unreal.EditorAssetLibrary.save_directory("/Game/Art", only_if_is_dirty=False, recursive=True)
    unreal.log("Created MVP greenhouse map with shell, work zones, plant props, lights, and labels.")


if __name__ == "__main__":
    main()
