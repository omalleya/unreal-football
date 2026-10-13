"""Import generated FBX to the shared mannequin skeleton, retaining source assets."""
import unreal, json
from pathlib import Path
OUT=Path(unreal.Paths.project_dir()).resolve()/'ArtSource/Mewtwo'
DEST='/Game/Football/Characters/Mewtwo'
unreal.SystemLibrary.execute_console_command(None,'Interchange.FeatureFlags.Import.FBX 0')
original=unreal.load_asset('/Game/Mannequin/Character/Mesh/SK_Mannequin')
skeleton=original.get_editor_property('skeleton')
options=unreal.FbxImportUI()
options.set_editor_property('import_mesh',True)
options.set_editor_property('import_as_skeletal',True)
options.set_editor_property('mesh_type_to_import',unreal.FBXImportType.FBXIT_SKELETAL_MESH)
options.set_editor_property('automated_import_should_detect_type',False)
options.set_editor_property('skeleton',skeleton)
options.set_editor_property('import_animations',False)
options.set_editor_property('import_materials',False)
options.set_editor_property('import_textures',False)
options.set_editor_property('create_physics_asset',False)
skeletal=options.get_editor_property('skeletal_mesh_import_data')
skeletal.set_editor_property('update_skeleton_reference_pose',False)
skeletal.set_editor_property('use_t0_as_ref_pose',False)
skeletal.set_editor_property('import_uniform_scale',1.0)
skeletal.set_editor_property('vertex_color_import_option',unreal.VertexColorImportOption.REPLACE)
skeletal.set_editor_property('normal_import_method',unreal.FBXNormalImportMethod.FBXNIM_IMPORT_NORMALS_AND_TANGENTS)
task=unreal.AssetImportTask();task.filename=str(OUT/'SK_Mewtwo.fbx');task.destination_path=DEST;task.destination_name='SK_Mewtwo';task.automated=True;task.replace_existing=True;task.save=True;task.options=options;task.factory=unreal.FbxFactory()
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
mesh=unreal.load_asset(DEST+'/SK_Mewtwo')
assert mesh, 'Skeletal mesh import failed'
assert mesh.get_editor_property('skeleton')==skeleton, 'Must use shared mannequin skeleton'
colors={'M_Mewtwo_Pale':(.72,.69,.78),'M_Mewtwo_Violet':(.36,.16,.39),'M_Mewtwo_Outline':(.035,.016,.06),'M_Mewtwo_EyeWhite':(.91,.9,.97),'M_Mewtwo_Iris':(.42,.1,.68)}
asset_tools=unreal.AssetToolsHelpers.get_asset_tools()
slots=list(mesh.get_editor_property('materials'))
for slot in slots:
 name=str(slot.get_editor_property('material_slot_name'))
 color=colors.get(name,(.72,.69,.78))
 path=DEST+'/'+name
 material=unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else asset_tools.create_asset(name,DEST,unreal.Material,unreal.MaterialFactoryNew())
 material.set_editor_property('used_with_skeletal_mesh',True)
 unreal.MaterialEditingLibrary.delete_all_material_expressions(material)
 if name=='M_Mewtwo_Pale':
  vc=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionVertexColor)
  unreal.MaterialEditingLibrary.connect_material_property(vc,'RGB',unreal.MaterialProperty.MP_BASE_COLOR)
 else:
  c=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionConstant3Vector)
  c.set_editor_property('constant',unreal.LinearColor(*color,1))
  unreal.MaterialEditingLibrary.connect_material_property(c,'',unreal.MaterialProperty.MP_BASE_COLOR)
 r=unreal.MaterialEditingLibrary.create_material_expression(material,unreal.MaterialExpressionConstant)
 r.set_editor_property('r',.46)
 unreal.MaterialEditingLibrary.connect_material_property(r,'',unreal.MaterialProperty.MP_ROUGHNESS)
 unreal.MaterialEditingLibrary.recompile_material(material)
 unreal.EditorAssetLibrary.save_loaded_asset(material)
 slot.set_editor_property('material_interface',material)
mesh.set_editor_property('materials',slots)
assert unreal.PlayerAppearance.add_football_grip_socket(mesh,unreal.Vector(8,0,-7),unreal.Rotator(0,0,0))
unreal.EditorAssetLibrary.save_loaded_asset(mesh)
appearance_class=unreal.load_class(None,'/Script/Football.PlayerAppearance')
if appearance_class:
 path=DEST+'/DA_Mewtwo'
 factory=unreal.DataAssetFactory();factory.set_editor_property('data_asset_class',appearance_class)
 appearance=unreal.load_asset(path) if unreal.EditorAssetLibrary.does_asset_exist(path) else asset_tools.create_asset('DA_Mewtwo',DEST,appearance_class,factory)
 appearance.set_editor_property('mesh',mesh)
 appearance.set_editor_property('animation_class',unreal.load_class(None,'/Game/Mannequin/Animations/ThirdPerson_AnimBP.ThirdPerson_AnimBP_C'))
 unreal.EditorAssetLibrary.save_loaded_asset(appearance)
report={'imported_paths':list(task.imported_object_paths),'skeleton':skeleton.get_path_name(),'materials':[str(s.material_slot_name) for s in mesh.materials],'grip_socket':'FootballGrip','grip_bone':'hand_r','appearance_created':bool(appearance_class)}
for label,asset in [('original',original),('mewtwo',mesh)]:
 comp=unreal.new_object(unreal.SkeletalMeshComponent)
 comp.set_skeletal_mesh_asset(asset)
 report[label]={'num_bones':comp.get_num_bones(),'bones':[str(comp.get_bone_name(i)) for i in range(comp.get_num_bones())]}
 try:report[label]['bounds']=str(asset.get_bounds())
 except Exception:pass
assert report['original']['bones']==report['mewtwo']['bones'],'Bone hierarchy must match'
(OUT/'unreal_validation.json').write_text(json.dumps(report,indent=2))
unreal.log('MEWTWO_IMPORT_VERIFIED '+json.dumps(report))
