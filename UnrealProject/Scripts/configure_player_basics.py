import unreal


MOUSE_LOOK_CONTEXT = "/Game/Input/IMC_MouseLook"
FIRST_PERSON_CHARACTER = "/Game/FirstPerson/Blueprints/BP_FirstPersonCharacter"
MANNY_PLACEHOLDER_MESH = "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple"
INVISIBLE_MATERIAL = "/Game/Characters/Mannequins/Materials/M_Invisible_FirstPersonPlaceholder"
MOUSE_SENSITIVITY = 0.12
CAMERA_HEIGHT_CM = 170.0
CAPSULE_RADIUS_CM = 70.0
CAPSULE_HALF_HEIGHT_CM = 180.0
FIRST_PERSON_MESH_PROPERTIES = (
    "FirstPersonMesh",
    "CharacterMesh",
    "CharacterMesh0",
    "Mesh",
)


def configure_mouse_sensitivity():
    context = unreal.EditorAssetLibrary.load_asset(MOUSE_LOOK_CONTEXT)
    if not context:
        raise RuntimeError(f"Missing input mapping context: {MOUSE_LOOK_CONTEXT}")

    mapping_data = context.get_editor_property("default_key_mappings")
    mappings = list(mapping_data.get_editor_property("mappings"))
    if not mappings:
        raise RuntimeError("IMC_MouseLook has no key mappings.")

    mapping = mappings[0]
    modifiers = list(mapping.get_editor_property("modifiers"))

    scalar_modifier = None
    for modifier in modifiers:
        if modifier.get_class().get_name() == "InputModifierScalar":
            scalar_modifier = modifier
            break

    if not scalar_modifier:
        scalar_modifier = unreal.new_object(
            unreal.InputModifierScalar,
            outer=context,
            name="InputModifierScalar_MouseSensitivity",
        )
        modifiers.append(scalar_modifier)

    scalar_modifier.set_editor_property(
        "scalar",
        unreal.Vector(MOUSE_SENSITIVITY, MOUSE_SENSITIVITY, 1.0),
    )
    mapping.set_editor_property("modifiers", modifiers)
    mappings[0] = mapping
    mapping_data.set_editor_property("mappings", mappings)
    context.set_editor_property("default_key_mappings", mapping_data)

    unreal.EditorAssetLibrary.save_loaded_asset(context)
    unreal.log(f"Mouse look sensitivity scaled to {MOUSE_SENSITIVITY}.")


def set_skeletal_mesh(component, mesh):
    for mesh_property in ("skeletal_mesh_asset", "skeletal_mesh"):
        try:
            component.set_editor_property(mesh_property, mesh)
            return True
        except Exception:
            continue
    return False


def hide_skeletal_component(component, hidden_mesh):
    set_skeletal_mesh(component, hidden_mesh)
    component.set_editor_property("visible", False)
    component.set_editor_property("hidden_in_game", True)
    component.set_editor_property("owner_no_see", True)
    component.set_editor_property("only_owner_see", False)
    component.set_editor_property("cast_shadow", False)
    component.set_editor_property("cast_dynamic_shadow", False)
    component.set_editor_property("cast_static_shadow", False)


def configure_camera_component(component, parent_component=None):
    if parent_component:
        for attach_call in (
            lambda: component.setup_attachment(parent_component, ""),
            lambda: component.set_editor_property("attach_parent", parent_component),
        ):
            try:
                attach_call()
                break
            except Exception:
                continue

    component.set_editor_property("relative_location", unreal.Vector(0.0, 0.0, CAMERA_HEIGHT_CM))
    component.set_editor_property("relative_rotation", unreal.Rotator(0.0, 0.0, 0.0))
    component.set_editor_property("use_pawn_control_rotation", True)


def configure_capsule_component(component):
    for method_name, value in (
        ("set_capsule_radius", CAPSULE_RADIUS_CM),
        ("set_capsule_half_height", CAPSULE_HALF_HEIGHT_CM),
    ):
        try:
            getattr(component, method_name)(value, True)
        except Exception:
            pass

    for property_name, value in (
        ("capsule_radius", CAPSULE_RADIUS_CM),
        ("capsule_half_height", CAPSULE_HALF_HEIGHT_CM),
    ):
        try:
            component.set_editor_property(property_name, value)
        except Exception:
            pass


def reparent_camera_to_capsule(character_bp, generated_class, subobject_subsystem):
    if not subobject_subsystem:
        return False

    handles = subobject_subsystem.k2_gather_subobject_data_for_blueprint(character_bp)
    capsule_handle = None
    camera_handle = None
    seen_components = set()

    for handle in handles:
        data = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(handle)
        component = unreal.SubobjectDataBlueprintFunctionLibrary.get_object_for_blueprint(
            data,
            character_bp,
        )
        if not component:
            continue

        path = component.get_path_name()
        if path in seen_components:
            continue
        seen_components.add(path)

        if isinstance(component, unreal.CapsuleComponent):
            capsule_handle = handle
        elif isinstance(component, unreal.CameraComponent):
            camera_handle = handle

    if not capsule_handle or not camera_handle:
        return False

    preview_actor = None
    try:
        preview_actor = unreal.EditorLevelLibrary.spawn_actor_from_class(
            generated_class,
            unreal.Vector(0.0, 0.0, -100000.0),
            unreal.Rotator(0.0, 0.0, 0.0),
        )
        preview_actor.set_actor_label("TEMP_BP_FirstPersonCharacter_CameraPreview")
        preview_actor.set_actor_hidden_in_game(True)

        params = unreal.ReparentSubobjectParams()
        params.set_editor_property("new_parent_handle", capsule_handle)
        params.set_editor_property("blueprint_context", character_bp)
        params.set_editor_property("actor_preview_context", preview_actor)
        result = subobject_subsystem.reparent_subobject(params, camera_handle)
        unreal.log(f"Reparented first-person camera to capsule: {result}")
        return bool(result)
    finally:
        if preview_actor:
            unreal.EditorLevelLibrary.destroy_actor(preview_actor)


def invisible_material():
    material = unreal.EditorAssetLibrary.load_asset(INVISIBLE_MATERIAL)
    if not material:
        asset_tools = unreal.AssetToolsHelpers.get_asset_tools()
        material = asset_tools.create_asset(
            asset_name="M_Invisible_FirstPersonPlaceholder",
            package_path="/Game/Characters/Mannequins/Materials",
            asset_class=unreal.Material,
            factory=unreal.MaterialFactoryNew(),
        )

    material.set_editor_property("blend_mode", unreal.BlendMode.BLEND_MASKED)
    material.set_editor_property("two_sided", False)
    material.set_editor_property("opacity_mask_clip_value", 0.5)

    opacity_mask = unreal.MaterialEditingLibrary.create_material_expression(
        material,
        unreal.MaterialExpressionConstant,
        -300,
        0,
    )
    opacity_mask.set_editor_property("r", 0.0)
    unreal.MaterialEditingLibrary.connect_material_property(
        opacity_mask,
        "",
        unreal.MaterialProperty.MP_OPACITY_MASK,
    )
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_loaded_asset(material)
    return material


def hide_manny_placeholder_asset():
    manny_mesh = unreal.EditorAssetLibrary.load_asset(MANNY_PLACEHOLDER_MESH)
    if not manny_mesh:
        return

    material = invisible_material()
    materials = list(manny_mesh.get_editor_property("materials"))
    for skeletal_material in materials:
        skeletal_material.set_editor_property("material_interface", material)
    manny_mesh.set_editor_property("materials", materials)

    unreal.EditorAssetLibrary.save_loaded_asset(manny_mesh)
    unreal.log("Assigned fully masked invisible material to SKM_Manny_Simple placeholder mesh.")


def configure_character_mesh():
    character_bp = unreal.EditorAssetLibrary.load_asset(FIRST_PERSON_CHARACTER)
    if not character_bp:
        raise RuntimeError(f"Missing first person character blueprint: {FIRST_PERSON_CHARACTER}")

    generated_class = character_bp.generated_class()
    cdo = unreal.get_default_object(generated_class)
    hidden_mesh = unreal.EditorAssetLibrary.load_asset(MANNY_PLACEHOLDER_MESH)
    if not hidden_mesh:
        raise RuntimeError(f"Missing placeholder mesh: {MANNY_PLACEHOLDER_MESH}")
    capsule_component = cdo.get_component_by_class(unreal.CapsuleComponent)
    if capsule_component:
        configure_capsule_component(capsule_component)

    for property_name in ("base_eye_height", "BaseEyeHeight"):
        try:
            cdo.set_editor_property(property_name, CAMERA_HEIGHT_CM)
        except Exception:
            pass

    changed = set()

    for component in cdo.get_components_by_class(unreal.SkeletalMeshComponent):
        hide_skeletal_component(component, hidden_mesh)
        changed.add(component.get_path_name())

    for component in cdo.get_components_by_class(unreal.CameraComponent):
        configure_camera_component(component, capsule_component)
        changed.add(component.get_path_name())

    for property_name in FIRST_PERSON_MESH_PROPERTIES:
        try:
            component = cdo.get_editor_property(property_name)
        except Exception:
            continue

        if isinstance(component, unreal.SkeletalMeshComponent):
            hide_skeletal_component(component, hidden_mesh)
            changed.add(component.get_path_name())

    construction_script = None
    for script_owner in (character_bp, generated_class):
        for property_name in ("simple_construction_script", "SimpleConstructionScript"):
            try:
                construction_script = script_owner.get_editor_property(property_name)
            except Exception:
                construction_script = None
            if construction_script:
                break
        if construction_script:
            break

    if construction_script:
        for node in construction_script.get_all_nodes():
            try:
                component = node.get_editor_property("component_template")
            except Exception:
                component = None

            if isinstance(component, unreal.SkeletalMeshComponent):
                hide_skeletal_component(component, hidden_mesh)
                changed.add(component.get_path_name())
            elif isinstance(component, unreal.CapsuleComponent):
                configure_capsule_component(component)
                changed.add(component.get_path_name())
            elif isinstance(component, unreal.CameraComponent):
                configure_camera_component(component, capsule_component)
                changed.add(component.get_path_name())

    subobject_subsystem = unreal.get_engine_subsystem(unreal.SubobjectDataSubsystem)
    if not subobject_subsystem:
        subobject_subsystem = unreal.get_editor_subsystem(unreal.SubobjectDataSubsystem)

    if subobject_subsystem:
        reparent_camera_to_capsule(character_bp, generated_class, subobject_subsystem)
        handles = subobject_subsystem.k2_gather_subobject_data_for_blueprint(character_bp)
        for handle in handles:
            data = unreal.SubobjectDataBlueprintFunctionLibrary.get_data(handle)
            component = unreal.SubobjectDataBlueprintFunctionLibrary.get_object_for_blueprint(
                data,
                character_bp,
            )
            if isinstance(component, unreal.SkeletalMeshComponent):
                hide_skeletal_component(component, hidden_mesh)
                changed.add(component.get_path_name())
            elif isinstance(component, unreal.CapsuleComponent):
                configure_capsule_component(component)
                changed.add(component.get_path_name())
            elif isinstance(component, unreal.CameraComponent):
                configure_camera_component(component, capsule_component)
                changed.add(component.get_path_name())

    unreal.BlueprintEditorLibrary.compile_blueprint(character_bp)
    unreal.EditorAssetLibrary.save_loaded_asset(character_bp)
    unreal.log(
        "Configured "
        f"{len(changed)} first-person component(s) with invisible character mesh "
        f"and centered camera height {CAMERA_HEIGHT_CM}cm."
    )


if __name__ == "__main__":
    configure_mouse_sensitivity()
    configure_character_mesh()
    hide_manny_placeholder_asset()
