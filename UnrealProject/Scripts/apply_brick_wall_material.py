import os

import unreal


PROJECT_MAP = "/Game/Maps/L_Greenhouse_MVP"
PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
BRICK_WALL_SOURCE = os.path.join(PROJECT_ROOT, "SourceTextures", "Walls", "brick_wall.png")
HALL_FOOTPRINT_SCALE = 1.5
HALL_LENGTH = int(2400 * HALL_FOOTPRINT_SCALE)
HALL_WIDTH = int(1600 * HALL_FOOTPRINT_SCALE)
WALL_HEIGHT = 600

WALL_MATERIALS = {
    "Hall_Back_wall_damaged_brick": "short",
    "Hall_Front_wall_damaged_brick": "short",
    "Hall_Left_wall_damaged_brick": "long",
    "Hall_Right_wall_damaged_brick": "long",
}


def ensure_folder(path):
    if not unreal.EditorAssetLibrary.does_directory_exist(path):
        unreal.EditorAssetLibrary.make_directory(path)


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
    texture.set_editor_property("address_x", unreal.TextureAddress.TA_MIRROR)
    texture.set_editor_property("address_y", unreal.TextureAddress.TA_CLAMP)
    unreal.EditorAssetLibrary.save_loaded_asset(texture)
    return texture


def make_textured_material(name, texture, roughness=0.92, u_tiling=10.0, v_tiling=3.0):
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
    unreal.MaterialEditingLibrary.connect_material_expressions(coordinate_expr, "", texture_expr, "UVs")
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


def main():
    texture = import_source_texture("T_Hall_Brick_Wall", BRICK_WALL_SOURCE)
    short_material = make_textured_material(
        "M_Hall_Brick_Wall_Short",
        texture,
        u_tiling=HALL_WIDTH / WALL_HEIGHT,
        v_tiling=1.0,
    )
    long_material = make_textured_material(
        "M_Hall_Brick_Wall_Long",
        texture,
        u_tiling=HALL_LENGTH / WALL_HEIGHT,
        v_tiling=1.0,
    )
    materials = {
        "short": short_material,
        "long": long_material,
    }

    unreal.EditorLevelLibrary.load_level(PROJECT_MAP)

    applied = 0
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        material_key = WALL_MATERIALS.get(actor.get_actor_label())
        if not material_key:
            continue

        component = actor.get_component_by_class(unreal.StaticMeshComponent)
        if not component:
            continue

        component.set_material(0, materials[material_key])
        applied += 1

    actor_count = len(unreal.EditorLevelLibrary.get_all_level_actors())
    unreal.log(f"Applied brick wall material to wall actors: {applied}")
    unreal.log(f"Map actor count after brick material apply: {actor_count}")

    unreal.EditorLevelLibrary.save_current_level()
    unreal.EditorAssetLibrary.save_directory("/Game/Art", only_if_is_dirty=False, recursive=True)
    unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")


if __name__ == "__main__":
    main()
