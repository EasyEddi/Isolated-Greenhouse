import unreal


PROJECT_MAP = "/Game/Maps/L_Greenhouse_MVP"
IMPORTED_MODEL_PREFIX = "/Game/ImportedModels/"


def actor_uses_imported_model(actor):
    for component in actor.get_components_by_class(unreal.StaticMeshComponent):
        mesh = component.get_editor_property("static_mesh")
        if not mesh:
            continue

        if mesh.get_path_name().startswith(IMPORTED_MODEL_PREFIX):
            return True

    return False


def main():
    unreal.EditorLevelLibrary.load_level(PROJECT_MAP)

    removed_labels = []
    for actor in list(unreal.EditorLevelLibrary.get_all_level_actors()):
        if not actor_uses_imported_model(actor):
            continue

        removed_labels.append(actor.get_actor_label())
        unreal.EditorLevelLibrary.destroy_actor(actor)

    actor_count = len(unreal.EditorLevelLibrary.get_all_level_actors())
    unreal.log(f"Removed imported model actors: {len(removed_labels)}")
    for label in removed_labels:
        unreal.log(f"Removed imported model actor: {label}")
    unreal.log(f"Map actor count after imported model cleanup: {actor_count}")

    unreal.EditorLevelLibrary.save_current_level()
    unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")


if __name__ == "__main__":
    main()
