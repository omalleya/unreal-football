import bpy,json,math
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2];OUT=ROOT/'ArtSource/Mewtwo'
bpy.ops.wm.open_mainfile(filepath=str(OUT/'Mewtwo.blend'))
rig=bpy.data.objects['root'];model=bpy.data.objects['SK_Mewtwo']
before=set(bpy.data.objects)
bpy.ops.import_scene.fbx(filepath=str(OUT/'reference_run.fbx'),automatic_bone_orientation=False)
added=set(bpy.data.objects)-before
other=next(o for o in added if o.type=='ARMATURE')
assert other.animation_data and other.animation_data.action
# Apply evaluated world-space poses to the matching rig; animation FBX carries different unit-scale curves.
for o in added:o.hide_render=True
scene=bpy.context.scene;scene.render.resolution_x=800;scene.render.resolution_y=800;scene.cycles.samples=24
scene.render.film_transparent=False
report={'animation':'Existing mannequin ThirdPersonRun','frames':[]}
for frame in [1,7,13,19]:
 scene.frame_set(frame);bpy.context.view_layer.update()
 for bone in rig.pose.bones:
  bone.matrix=rig.matrix_world.inverted() @ other.matrix_world @ other.pose.bones[bone.name].matrix
  bpy.context.view_layer.update()
 bpy.context.view_layer.update()
 deps=bpy.context.evaluated_depsgraph_get();evaluated=model.evaluated_get(deps)
 verts=[evaluated.matrix_world@v.co for v in evaluated.data.vertices]
 lo=[min(v[i] for v in verts) for i in range(3)];hi=[max(v[i] for v in verts) for i in range(3)]
 assert all(math.isfinite(x) for x in lo+hi)
 assert all(hi[i]-lo[i]<3.2 for i in range(3))
 assert 1.3<hi[2]-lo[2]<2.5
 report['frames'].append({'frame':frame,'min':lo,'max':hi,'thigh_l_rotation':list(rig.pose.bones['thigh_l'].rotation_quaternion)})
 if frame in [7,19]:
  scene.render.filepath=str(OUT/f'Mewtwo_run_frame_{frame:02d}.png');bpy.ops.render.render(write_still=True)
(OUT/'animation_validation.json').write_text(json.dumps(report,indent=2))
print('MEWTWO_ANIMATION_VALIDATION_PASS',json.dumps(report))
