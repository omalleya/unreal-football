import unreal
from pathlib import Path

out = Path(unreal.Paths.project_dir()).resolve() / 'ArtSource/Mewtwo/reference_mannequin.fbx'
task = unreal.AssetExportTask()
task.object = unreal.load_asset('/Game/Mannequin/Character/Mesh/SK_Mannequin')
task.filename = str(out)
task.automated = True
task.prompt = False
task.replace_identical = True
task.exporter = unreal.SkeletalMeshExporterFBX()
task.options = unreal.FbxExportOption()
task.options.set_editor_property('ascii', False)
task.options.set_editor_property('level_of_detail', False)
assert unreal.Exporter.run_asset_export_task(task), 'Reference rig export failed'
unreal.log('MEWTWO_REFERENCE_EXPORTED ' + str(out))
