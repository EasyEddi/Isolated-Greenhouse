import math
import unreal


MAP_PATH = "/Game/Maps/L_Greenhouse_MVP"


def actor_label(actor):
    try:
        return actor.get_actor_label()
    except Exception:
        return actor.get_name()


def is_storage_shelf(actor):
    label = actor_label(actor).lower()
    path = actor.get_path_name().lower()
    return "storage" in label or "storage" in path or "storge" in label or "storge" in path or "shelf" in label or "shelf" in path


def horizontal_distance_squared(a, b):
    dx = a.x - b.x
    dy = a.y - b.y
    return dx * dx + dy * dy


def main():
    unreal.EditorLevelLibrary.load_level(MAP_PATH)
    world = unreal.EditorLevelLibrary.get_editor_world()
    actors = unreal.EditorLevelLibrary.get_all_level_actors()

    player_starts = [actor for actor in actors if actor.get_class().get_name() == "PlayerStart"]
    shelves = [actor for actor in actors if is_storage_shelf(actor)]

    if not player_starts:
        raise RuntimeError("No PlayerStart actor found in L_Greenhouse_MVP")

    if not shelves:
        raise RuntimeError("No storage shelf actor found in L_Greenhouse_MVP")

    player_start = player_starts[0]
    start_location = player_start.get_actor_location()
    target = min(shelves, key=lambda actor: horizontal_distance_squared(start_location, actor.get_actor_location()))
    target_location = target.get_actor_location()

    direction_x = target_location.x - start_location.x
    direction_y = target_location.y - start_location.y
    yaw_degrees = math.degrees(math.atan2(direction_y, direction_x))

    player_start.set_actor_rotation(unreal.Rotator(0.0, yaw_degrees, 0.0), False)
    unreal.EditorLoadingAndSavingUtils.save_dirty_packages(True, True)

    unreal.log(
        "Aimed PlayerStart at storage shelf: "
        f"{actor_label(player_start)} -> {actor_label(target)} yaw={yaw_degrees:.2f}"
    )


main()
