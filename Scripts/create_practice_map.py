"""Run in Unreal Editor's Python commandlet once to create project-owned starter assets."""
import unreal

MATERIAL = "/Game/Football/Materials/M_Prototype"
MAP = "/Game/Football/Maps/PracticeField"
tools = unreal.AssetToolsHelpers.get_asset_tools()
if not unreal.EditorAssetLibrary.does_asset_exist(MATERIAL):
    material = tools.create_asset("M_Prototype", "/Game/Football/Materials", unreal.Material, unreal.MaterialFactoryNew())
    color = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionVectorParameter)
    color.set_editor_property("parameter_name", "Tint")
    color.set_editor_property("default_value", unreal.LinearColor(0.8, 0.8, 0.8, 1))
    material.set_editor_property("used_with_instanced_static_meshes", True)
    unreal.MaterialEditingLibrary.connect_material_property(color, "", unreal.MaterialProperty.MP_BASE_COLOR)
    rough = unreal.MaterialEditingLibrary.create_material_expression(material, unreal.MaterialExpressionConstant)
    rough.set_editor_property("r", 0.82)
    unreal.MaterialEditingLibrary.connect_material_property(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    unreal.MaterialEditingLibrary.recompile_material(material)
    unreal.EditorAssetLibrary.save_asset(MATERIAL)

base = unreal.load_asset(MATERIAL)
colors = {
    "Surround": (0.025, 0.065, 0.045), "Turf": (0.045, 0.19, 0.075),
    "AlternateTurf": (0.055, 0.23, 0.09), "Markings": (0.92, 0.96, 0.90),
    "HomeEndzone": (0.025, 0.10, 0.24), "AwayEndzone": (0.65, 0.16, 0.025),
    "Goalposts": (0.95, 0.68, 0.04),
}
for name, rgb in colors.items():
    path = "/Game/Football/Materials/MI_" + name
    if not unreal.EditorAssetLibrary.does_asset_exist(path):
        instance = tools.create_asset("MI_" + name, "/Game/Football/Materials", unreal.MaterialInstanceConstant, unreal.MaterialInstanceConstantFactoryNew())
        unreal.MaterialEditingLibrary.set_material_instance_parent(instance, base)
        unreal.MaterialEditingLibrary.set_material_instance_vector_parameter_value(instance, "Tint", unreal.LinearColor(*rgb, 1))
        unreal.EditorAssetLibrary.save_asset(path)

if unreal.EditorAssetLibrary.does_asset_exist(MAP):
    raise RuntimeError("PracticeField already exists; refusing to overwrite an edited map.")
level = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
level.new_level(MAP)
actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
field_class = unreal.load_class(None, "/Script/Football.PracticeField")
field = actors.spawn_actor_from_class(field_class, unreal.Vector(0, 0, 0))
field.set_actor_label("Practice field - 120 yards")
start = actors.spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(-3657.6, 0, 100))
start.set_actor_label("Start - home 10 yard line")
sun = actors.spawn_actor_from_class(unreal.DirectionalLight, unreal.Vector(0, 0, 1500), unreal.Rotator(-42, -30, 0))
sun.light_component.set_editor_property("intensity", 3.5)
sun.light_component.set_editor_property("atmosphere_sun_light", True)
sun.light_component.set_editor_property("dynamic_shadow_distance_movable_light", 15000)
sun.light_component.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
actors.spawn_actor_from_class(unreal.SkyAtmosphere, unreal.Vector(0, 0, 0))
sky = actors.spawn_actor_from_class(unreal.SkyLight, unreal.Vector(0, 0, 1500))
sky.light_component.set_editor_property("mobility", unreal.ComponentMobility.MOVABLE)
sky.light_component.set_editor_property("real_time_capture", True)
sky.light_component.set_editor_property("intensity", 1.0)
level.save_current_level()
unreal.log("FOOTBALL_MAP_CREATED " + MAP)
