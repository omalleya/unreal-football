import bpy,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'ArtSource/Mewtwo'
def inspect(file):
 bpy.ops.wm.read_factory_settings(use_empty=True)
 bpy.ops.import_scene.fbx(filepath=str(file),automatic_bone_orientation=False)
 arm=next(o for o in bpy.context.scene.objects if o.type=='ARMATURE')
 meshes=[o for o in bpy.context.scene.objects if o.type=='MESH']
 return {'arm':arm.matrix_world.copy(),'bones':{b.name:(b.parent.name if b.parent else None, arm.matrix_world@b.matrix_local) for b in arm.data.bones},'meshes':[{'name':o.name,'dimensions':list(o.dimensions),'vertices':len(o.data.vertices)} for o in meshes]}
a=inspect(OUT/'reference_mannequin.fbx');b=inspect(OUT/'SK_Mewtwo.fbx')
assert a['bones'].keys()==b['bones'].keys()
errors={}
for name,(parent,matrix) in a['bones'].items():
 assert parent==b['bones'][name][0]
 errors[name]=max(abs(matrix[i][j]-b['bones'][name][1][i][j]) for i in range(4) for j in range(4))
report={'bone_count_blender':len(errors),'same_names_and_hierarchy':True,'max_world_bind_matrix_error':max(errors.values()),'mesh':b['meshes'],'bone_errors':errors}
(OUT/'fbx_roundtrip_validation.json').write_text(json.dumps(report,indent=2))
assert max(errors.values())<.0001, report
print('MEWTWO_FBX_ROUNDTRIP_PASS',json.dumps(report))
