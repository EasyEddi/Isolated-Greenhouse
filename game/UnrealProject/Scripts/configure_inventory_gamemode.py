import unreal


MAP_PATH = "/Game/Maps/L_Greenhouse_MVP"
GAME_MODE_PATH = "/Script/TheIsolatedGreenhouse.GreenhouseGameMode"


def main():
    unreal.EditorLevelLibrary.load_level(MAP_PATH)
    world = unreal.EditorLevelLibrary.get_editor_world()
    world_settings = world.get_world_settings()
    game_mode_class = unreal.load_class(None, GAME_MODE_PATH)

    if game_mode_class is None:
        raise RuntimeError(f"Could not load game mode class: {GAME_MODE_PATH}")

    world_settings.set_editor_property("default_game_mode", game_mode_class)
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)
    unreal.log(f"Configured {MAP_PATH} to use {GAME_MODE_PATH}")


main()
