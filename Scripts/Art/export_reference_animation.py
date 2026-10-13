import unreal
from pathlib import Path
out=Path(unreal.Paths.project_dir()).resolve()/'ArtSource/Mewtwo/reference_run.fbx'
t=unreal.AssetExportTask();t.object=unreal.load_asset('/Game/Mannequin/Animations/ThirdPersonRun');t.filename=str(out);t.automated=True;t.prompt=False;t.replace_identical=True;t.exporter=unreal.AnimSequenceExporterFBX();t.options=unreal.FbxExportOption();t.options.set_editor_property('export_preview_mesh',False)
assert unreal.Exporter.run_asset_export_task(t)
unreal.log('MEWTWO_REFERENCE_ANIMATION_EXPORTED')
