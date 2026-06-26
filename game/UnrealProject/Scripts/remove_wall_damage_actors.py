import unreal


PROJECT_MAP = "/Game/Maps/L_Greenhouse_MVP"

REMOVE_PREFIXES = (
    "WallDamage_",
)

REMOVE_CONTAINS = (
    "_brick_segment_",
)


def should_remove(actor):
    label = actor.get_actor_label()
    return any(label.startswith(prefix) for prefix in REMOVE_PREFIXES) or any(
        marker in label for marker in REMOVE_CONTAINS
    )


def main():
    unreal.EditorLevelLibrary.load_level(PROJECT_MAP)

    removed = 0
    kept_wall_actors = []
    for actor in unreal.EditorLevelLibrary.get_all_level_actors():
        label = actor.get_actor_label()
        if label in {
            "Hall_Back_wall_damaged_brick",
            "Hall_Front_wall_damaged_brick",
            "Hall_Left_wall_damaged_brick",
            "Hall_Right_wall_damaged_brick",
        }:
            kept_wall_actors.append(label)

        if should_remove(actor):
            unreal.EditorLevelLibrary.destroy_actor(actor)
            removed += 1

    actor_count = len(unreal.EditorLevelLibrary.get_all_level_actors())
    unreal.log(f"Removed wall detail actors: {removed}")
    unreal.log(f"Kept main wall actors: {sorted(kept_wall_actors)}")
    unreal.log(f"Map actor count after wall cleanup: {actor_count}")

    unreal.EditorLevelLibrary.save_current_level()
    unreal.SystemLibrary.execute_console_command(None, "QUIT_EDITOR")


if __name__ == "__main__":
    main()
