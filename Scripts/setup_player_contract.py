"""Create the baseline swappable appearance without modifying the original template mesh."""
import unreal

ROOT = "/Game/Football/Characters/Shared"
MESH_PATH = ROOT + "/SK_PracticeMannequin"
ASSET_PATH = ROOT + "/DA_PracticeMannequin"
unreal.EditorAssetLibrary.make_directory(ROOT)
mesh = unreal.load_asset(MESH_PATH) if unreal.EditorAssetLibrary.does_asset_exist(MESH_PATH) else unreal.EditorAssetLibrary.duplicate_asset(
    "/Game/Mannequin/Character/Mesh/SK_Mannequin", MESH_PATH)
if not mesh:
    raise RuntimeError("Could not create shared practice mesh")
if not unreal.PlayerAppearance.add_football_grip_socket(mesh, unreal.Vector(8, 0, -7), unreal.Rotator(0, 0, 0)):
    raise RuntimeError("Could not create a valid FootballGrip socket")
unreal.EditorAssetLibrary.save_loaded_asset(mesh)

appearance_class = unreal.load_class(None, "/Script/Football.PlayerAppearance")
if unreal.EditorAssetLibrary.does_asset_exist(ASSET_PATH):
    appearance = unreal.load_asset(ASSET_PATH)
else:
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", appearance_class)
    appearance = unreal.AssetToolsHelpers.get_asset_tools().create_asset("DA_PracticeMannequin", ROOT, appearance_class, factory)
    appearance.set_editor_property("mesh", mesh)
    appearance.set_editor_property("animation_class", unreal.load_class(None, "/Game/Mannequin/Animations/ThirdPerson_AnimBP.ThirdPerson_AnimBP_C"))
    unreal.EditorAssetLibrary.save_loaded_asset(appearance)
unreal.log("FOOTBALL_PLAYER_CONTRACT_READY " + ASSET_PATH)
