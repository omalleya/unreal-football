"""Original Mewtwo fan-character blockout; Blender 4.3 background script.
Requires reference_mannequin.fbx exported by export_reference_rig.py.
Preserves the mannequin's exact armature and transfers its skinning to new geometry.
"""
import bpy, math, json, bmesh
from pathlib import Path
from mathutils import Vector, Matrix
import numpy as np
from mathutils.kdtree import KDTree
from mathutils.bvhtree import BVHTree
ROOT=Path(__file__).resolve().parents[2]
OUT=ROOT/'ArtSource/Mewtwo'
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.fbx(filepath=str(OUT/'reference_mannequin.fbx'),automatic_bone_orientation=False)
rig=next(o for o in bpy.context.scene.objects if o.type=='ARMATURE')
refs=[o for o in bpy.context.scene.objects if o.type=='MESH']
ref=max(refs,key=lambda o:len(o.data.vertices))
rig.name='root'
def H(n): return rig.matrix_world @ rig.data.bones[n].head_local
UP=Vector((0,0,1))
RIGHT=(H('upperarm_r')-H('upperarm_l')); RIGHT.z=0; RIGHT.normalize()
FRONT=(H('ball_r')-H('foot_r')+H('ball_l')-H('foot_l')); FRONT.z=0; FRONT.normalize()
S=H('head').z/1.62
BASIS=Matrix((RIGHT,FRONT,UP)).transposed()
def P(x,y,z): return S*(RIGHT*x+FRONT*y+UP*z)
def mat(name,color,rough=.5):
 m=bpy.data.materials.new(name);m.diffuse_color=(*color,1);m.use_nodes=True
 bs=m.node_tree.nodes.get('Principled BSDF');bs.inputs['Base Color'].default_value=(*color,1);bs.inputs['Roughness'].default_value=rough
 return m
WHITE=mat('M_Mewtwo_Pale',(0.69,.66,.77),.46)
PURPLE=mat('M_Mewtwo_Violet',(.36,.16,.39),.42)
DARK=mat('M_Mewtwo_Outline',(.035,.016,.06),.52)
EYEWHITE=mat('M_Mewtwo_EyeWhite',(.91,.9,.97),.26)
IRIS=mat('M_Mewtwo_Iris',(.42,.1,.68),.25)
parts=[];body=[]
def ell(name,loc,scale,material=WHITE,bone=None,unify=False):
 bpy.ops.mesh.primitive_uv_sphere_add(segments=24,ring_count=16,radius=1,location=loc)
 o=bpy.context.object;o.name=name;o.rotation_euler=BASIS.to_euler();o.scale=Vector(scale)*S
 bpy.ops.object.transform_apply(location=False,rotation=True,scale=True)
 o.data.materials.append(material)
 if bone:o['rigid_bone']=bone
 (body if unify else parts).append(o)
 return o

def segment(name,a,b,rx,ry=None,material=WHITE,bone=None,unify=True):
 a,b=Vector(a),Vector(b);mid=(a+b)/2
 o=ell(name,mid,(rx,ry or rx,(b-a).length/S/2+rx*.45),material,bone,unify)
 # Ellipsoid's long axis follows bone; rebuild its rotation around world Z.
 o.rotation_euler=(b-a).to_track_quat('Z','Y').to_euler()
 # ell was oriented already; an ellipsoid is rotationally symmetric for most limbs.
 bpy.ops.object.transform_apply(location=False,rotation=True,scale=True)
 return o

# Torso, broad thighs, slender shins and feline head. Positions follow the shared bind rig.
pel=H('pelvis'); chest=H('spine_03'); neck=H('neck_01');head=H('head')
ell('Pelvis',pel+P(0,0,.00),(.22,.145,.205),unify=True)
ell('Abdomen',(pel+chest)/2+P(0,.012,0),(.110,.097,.225),unify=True)
ell('Chest',chest+P(0,.006,-.04),(.153,.102,.215),unify=True)
segment('Neck',chest+P(0,0,.055),head,.050,unify=True)
# Sculpt a continuous tapered forehead, cheek and small wedge snout.
headrings=[(-.14,.12,.075,.13),(-.09,.14,.15,.185),(.015,.14,.172,.205),(.105,.11,.135,.155),(.175,.077,.075,.083),(.228,.050,.030,.032),(.252,.045,.010,.015)]
verts=[];faces=[];n=32
for y,z,rx,rz in headrings:
 for j in range(n):
  t=2*math.pi*j/n;verts.append(head+P(rx*math.cos(t),y,z+rz*math.sin(t)))
for i in range(len(headrings)-1):
 for j in range(n):
  a=i*n+j;b=i*n+(j+1)%n;faces.append((a,b,b+n,a+n))
faces.extend([tuple(reversed(range(n))),tuple((len(headrings)-1)*n+j for j in range(n))])
m=bpy.data.meshes.new('HeadWedge');m.from_pydata(verts,[],faces);m.update()
o=bpy.data.objects.new('HeadWedge',m);bpy.context.collection.objects.link(o);o.data.materials.append(WHITE);body.append(o)
bpy.context.view_layer.objects.active=o;o.select_set(True)
sub=o.modifiers.new('Organic head','SUBSURF');sub.levels=2;bpy.ops.object.modifier_apply(modifier=sub.name)
for side,sign in [('r',1),('l',-1)]:
 shoulder,elbow,wrist=H('upperarm_'+side),H('lowerarm_'+side),H('hand_'+side)
 ell('Shoulder_'+side,shoulder,(.060,.060,.065),unify=True)
 segment('ClavicleBridge_'+side,chest+P(0,0,.07),shoulder,.05,unify=True)
 segment('UpperArm_'+side,shoulder,elbow,.053,unify=True)
 ell('Elbow_'+side,elbow,(.046,.048,.052),unify=True)
 segment('Forearm_'+side,elbow,wrist,.045,unify=True)
 hip,knee,ankle=H('thigh_'+side),H('calf_'+side),H('foot_'+side)
 segment('Thigh_'+side,hip+P(sign*.027,0,-.02),knee,.173,.155,unify=True)
 ell('Knee_'+side,knee,(.075,.082,.092),unify=True)
 segment('Shin_'+side,knee,ankle,.065,.058,unify=True)
 ell('Foot_'+side,ankle+P(sign*.013,.087,-.080),(.098,.175,.063),unify=True)
 for i in [-1,1]:
  ell('Toe_'+side+str(i),ankle+P(i*.053,.225,-.087),(.05,.115,.042),bone='foot_'+side,unify=True)
 # Shared humanoid palm and three thick digits (thumb, index, pinky).
 palm=H('middle_01_'+side)
 segment('Palm_'+side,wrist,palm,.047,bone='hand_'+side,unify=True)
 for digit in ['thumb','index','pinky']:
  pts=[H(f'{digit}_{i:02d}_{side}') for i in [1,2,3]]
  # Long non-human fingers with broad pads, controlled by existing finger bones.
  direction=(pts[2]-pts[0]).normalized()
  terminal=pts[2]+direction*(.062*S)
  pts.append(terminal)
  for j in range(3):
   segment(f'{digit}_{side}_{j}',pts[j],pts[j+1],.018,bone=f'{digit}_{j+1:02d}_{side}',unify=True)
  ell('FingerPad_'+digit+side,pts[-1],(.035,.035,.035),bone=f'{digit}_03_{side}',unify=True)
 # Long horn-like feline ears, joined smoothly to the skull.
 a=head+P(sign*.105,-.027,.225);b=head+P(sign*.167,-.073,.45)
 bpy.ops.mesh.primitive_cone_add(vertices=32,radius1=.05*S,radius2=.014*S,depth=(b-a).length,location=(a+b)/2)
 ear=bpy.context.object;ear.name='TaperedEar_'+side;ear.rotation_euler=(b-a).to_track_quat('Z','Y').to_euler();ear.data.materials.append(WHITE);body.append(ear)
 ell('EarTip_'+side,b,(.016,.016,.021),bone='head',unify=True)

# Fuse the main pale skin so shoulders, torso, hips, knees and head are contiguous.
bpy.ops.object.select_all(action='DESELECT')
for o in body:o.select_set(True)
bpy.context.view_layer.objects.active=body[0]
bpy.ops.object.join(); skin=bpy.context.object;skin.name='Mewtwo_Skin'
rem=skin.modifiers.new('Continuous skin','REMESH');rem.mode='VOXEL';rem.voxel_size=.009*S;rem.use_smooth_shade=True
bpy.ops.object.modifier_apply(modifier=rem.name)
sm=skin.modifiers.new('Sculpt smooth','SMOOTH');sm.factor=1.0;sm.iterations=4
bpy.ops.object.modifier_apply(modifier=sm.name)
dec=skin.modifiers.new('Prototype topology','DECIMATE');dec.ratio=.45
bpy.ops.object.modifier_apply(modifier=dec.name)
parts.append(skin)
def fuse_parts(objects,name,voxel=.008):
 bpy.ops.object.select_all(action='DESELECT')
 for o in objects:o.select_set(True)
 bpy.context.view_layer.objects.active=objects[0];bpy.ops.object.join();o=bpy.context.object;o.name=name
 rem=o.modifiers.new('Smooth continuous surface','REMESH');rem.mode='VOXEL';rem.voxel_size=voxel*S;rem.use_smooth_shade=True
 bpy.ops.object.modifier_apply(modifier=rem.name)
 sm=o.modifiers.new('Blend joins','SMOOTH');sm.factor=1;sm.iterations=5;bpy.ops.object.modifier_apply(modifier=sm.name)
 dec=o.modifiers.new('Efficient surface','DECIMATE');dec.ratio=.32;bpy.ops.object.modifier_apply(modifier=dec.name)
 for old in objects:
  if old in parts:parts.remove(old)
 parts.append(o)
 return o
# The almond eye openings follow the actual head surface, so they cannot protrude.
worldverts=[skin.matrix_world@v.co for v in skin.data.vertices]
bvh=BVHTree.FromPolygons(worldverts,[list(p.vertices) for p in skin.data.polygons])
def surface(sign,x,z):
 origin=head+P(sign*x,.50,z)
 hit=bvh.ray_cast(origin,-FRONT,1.0*S)[0]
 if hit is None:raise RuntimeError('Eye coordinate falls outside head')
 return hit
for sign in [-1,1]:
 coords=[]
 for j in range(17):
  t=j/16;coords.append((.025+.115*t,.122+.059*t+.010*math.sin(math.pi*t)))
 for j in range(15,0,-1):
  t=j/16;coords.append((.025+.115*t,.122+.059*t-.032*math.sin(math.pi*t)))
 n=len(coords)
 vv=[head+P(sign*x,.5,z) for x,z in coords]+[head+P(sign*x,-.04,z) for x,z in coords]
 ff=[tuple(reversed(range(n))),tuple(n+j for j in range(n))]+[(j,(j+1)%n,(j+1)%n+n,j+n) for j in range(n)]
 me=bpy.data.meshes.new('OrbitCut');me.from_pydata(vv,[],ff);me.update();bm=bmesh.new();bm.from_mesh(me);bmesh.ops.recalc_face_normals(bm,faces=list(bm.faces));bm.to_mesh(me);bm.free();cutobj=bpy.data.objects.new('OrbitCut',me);bpy.context.collection.objects.link(cutobj)
 bpy.context.view_layer.objects.active=skin
 cut=skin.modifiers.new('Inset almond orbit','BOOLEAN');cut.operation='DIFFERENCE';cut.solver='EXACT';cut.object=cutobj;bpy.ops.object.modifier_apply(modifier=cut.name)
 bpy.data.objects.remove(cutobj,do_unlink=True)
 cx,cz=.0825,.142
 for name,scale,depth,material in [('EyeSocket',1,.010,DARK),('InsetEye',.92,.006,EYEWHITE)]:
  vs=[]
  for ring in [1,.72,.4]:
   for x,z in coords:
    x=cx+(x-cx)*ring*scale;z=cz+(z-cz)*ring*scale
    vs.append(surface(sign,x,z)-FRONT*depth*S)
  vs.append(surface(sign,cx,cz)-FRONT*depth*S)
  fs=[]
  for ring in range(2):
   for j in range(n):fs.append((ring*n+j,ring*n+(j+1)%n,(ring+1)*n+(j+1)%n,(ring+1)*n+j))
  for j in range(n):fs.append((2*n+j,2*n+(j+1)%n,3*n))
  me=bpy.data.meshes.new(name);me.from_pydata(vs,[],fs);me.update();o=bpy.data.objects.new(name,me);bpy.context.collection.objects.link(o);o.data.materials.append(material);o['rigid_bone']='head';parts.append(o)
 ec=surface(sign,cx,cz)
 ell('VioletIris',ec-FRONT*.007*S,(.012,.003,.018),IRIS,'head')
 ell('Pupil',ec-FRONT*.004*S,(.004,.002,.014),DARK,'head')

def tube(name,points,radii,material,bone,segments=16):
 verts=[]; faces=[]
 for i,c in enumerate(points):
  tangent=points[min(i+1,len(points)-1)]-points[max(i-1,0)];tangent.normalize()
  n=tangent.cross(UP)
  if n.length<.01:n=RIGHT.copy()
  n.normalize();b=tangent.cross(n).normalized()
  for j in range(segments):
   t=j*2*math.pi/segments;verts.append(c+radii[i]*S*(math.cos(t)*n+math.sin(t)*b))
 for i in range(len(points)-1):
  for j in range(segments):
   a=i*segments+j;c=i*segments+(j+1)%segments
   faces.append((a,c,c+segments,a+segments))
 faces.append(tuple(reversed(range(segments))));faces.append(tuple((len(points)-1)*segments+j for j in range(segments)))
 mesh=bpy.data.meshes.new(name);mesh.from_pydata(verts,[],faces);mesh.update()
 o=bpy.data.objects.new(name,mesh);bpy.context.collection.objects.link(o);o.data.materials.append(material);o['rigid_bone']=bone;parts.append(o)
 return o
# Sweeping tail behind the player with clear side silhouette; pelvis bound for shared rig v1.
control=[Vector((0,-.09,.99)),Vector((0,-.32,.91)),Vector((.10,-.56,.68)),Vector((.34,-.73,.43)),Vector((.62,-.72,.43)),Vector((.81,-.61,.64)),Vector((.83,-.46,.88)),Vector((.78,-.36,1.04))]
points=[];radii=[]
for i in range(len(control)-1):
 p0=control[max(0,i-1)];p1=control[i];p2=control[i+1];p3=control[min(len(control)-1,i+2)]
 for j in range(6):
  t=j/6;v=.5*((2*p1)+(-p0+p2)*t+(2*p0-5*p1+4*p2-p3)*t*t+(-p0+3*p1-3*p2+p3)*t*t*t)
  points.append(P(*v));u=(i+t)/(len(control)-1);radii.append(.12*(1-u)+.045*u+.022*math.exp(-((u-.95)/.18)**2))
points.append(P(*control[-1]));radii.append(.066)
tail_start=len(parts)
tube('Tail',points,radii,PURPLE,'pelvis')
ell('TailTip',points[-1],(.066,.066,.066),PURPLE,'pelvis')
fuse_parts(parts[tail_start:].copy(),'ContinuousTail')
# Characteristic back-of-head tube (nonphysical, skin color).
pts=[head+P(0,-.08,.10),head+P(0,-.20,.09),head+P(0,-.22,-.03),neck+P(0,-.12,-.035)]
neckpts=[]
for i in range(len(pts)-1):
 p0=pts[max(0,i-1)];p1=pts[i];p2=pts[i+1];p3=pts[min(len(pts)-1,i+2)]
 for j in range(6):
  t=j/6;neckpts.append(.5*((2*p1)+(-p0+p2)*t+(2*p0-5*p1+4*p2-p3)*t*t+(-p0+3*p1-3*p2+p3)*t*t*t))
neckpts.append(pts[-1])
tube('NeckTube',neckpts,[.035]*len(neckpts),WHITE,'neck_01')
# Transfer source weights with nearest-neighbor interpolation in world space.
kd=KDTree(len(ref.data.vertices));weights={}
for v in ref.data.vertices:
 kd.insert(ref.matrix_world@v.co,v.index)
 weights[v.index]={ref.vertex_groups[g.group].name:g.weight for g in v.groups if ref.vertex_groups[g.group].name in rig.data.bones}
kd.balance()
for o in parts:
 for b in rig.data.bones:o.vertex_groups.new(name=b.name)
 rigid=o.get('rigid_bone')
 for v in o.data.vertices:
  if rigid:ww={rigid:1.}
  else:
   near=kd.find_n(o.matrix_world@v.co,4);ww={}
   for co,idx,dist in near:
    for name,w in weights[idx].items():ww[name]=ww.get(name,0)+w/(dist*dist+1e-8)
   ww=dict(sorted(ww.items(),key=lambda x:-x[1])[:4]);total=sum(ww.values());ww={k:v/total for k,v in ww.items()}
  for name,w in ww.items():o.vertex_groups[name].add([v.index],w,'REPLACE')
 for poly in o.data.polygons:poly.use_smooth=True
# Join all new geometry into one skeletal mesh with five material slots.
bpy.ops.object.select_all(action='DESELECT')
for o in parts:o.select_set(True)
bpy.context.view_layer.objects.active=skin;bpy.ops.object.join();model=bpy.context.object;model.name='SK_Mewtwo'
tint=model.data.color_attributes.new(name='BodyTint',type='FLOAT_COLOR',domain='POINT')
white=np.array((.72,.69,.78));mauve=np.array((.36,.16,.39))
for v in model.data.vertices:
 c=model.matrix_world@v.co;x=c.dot(RIGHT)/S;y=c.dot(FRONT)/S;z=c.z/S
 e=(x/.17)**2+((z-1.00)/.255)**2
 mask=max(0,min(1,(1.10-e)/.20))*(1 if y>0 else 0)
 tint.data[v.index].color=(*tuple(white*(1-mask)+mauve*mask),1)
node=WHITE.node_tree.nodes.new('ShaderNodeVertexColor');node.layer_name='BodyTint'
WHITE.node_tree.links.new(node.outputs['Color'],WHITE.node_tree.nodes.get('Principled BSDF').inputs['Base Color'])
# Smooth neighboring skin weights, then keep four normalized influences per vertex.
W=np.zeros((len(model.data.vertices),len(model.vertex_groups)),dtype=np.float32)
for v in model.data.vertices:
 for g in v.groups:W[v.index,g.group]=g.weight
edges=np.array([list(e.vertices) for e in model.data.edges]);degree=np.bincount(edges.flatten(),minlength=len(W)).astype(float)
for iteration in range(5):
 acc=np.zeros_like(W);np.add.at(acc,edges[:,0],W[edges[:,1]]);np.add.at(acc,edges[:,1],W[edges[:,0]])
 W=.65*W+.35*acc/np.maximum(degree[:,None],1)
for g in model.vertex_groups:g.remove(range(len(W)))
for i,row in enumerate(W):
 ids=np.argpartition(row,-4)[-4:];total=sum(row[ids])
 for idx in ids:
  if row[idx]>0:model.vertex_groups[int(idx)].add([i],float(row[idx]/total),'REPLACE')
arm=model.modifiers.new('Shared mannequin rig','ARMATURE');arm.object=rig
model.data.transform(rig.matrix_world.inverted() @ model.matrix_world)
model.parent=rig;model.matrix_parent_inverse=Matrix.Identity(4);model.matrix_basis=Matrix.Identity(4)
bpy.context.view_layer.update()
# Original Epic mesh was only a local reference; remove it from authored deliverables.
for o in refs:bpy.data.objects.remove(o,do_unlink=True)
# Validate geometry and skinning before export.
model.data.calc_loop_triangles()
invalid=[]
for v in model.data.vertices:
 total=sum(g.weight for g in v.groups)
 if abs(total-1)>1e-4:invalid.append(v.index)
assert not invalid, f'Invalid weights: {len(invalid)}'
assert all(b.name in model.vertex_groups for b in rig.data.bones)
report={'mesh':model.name,'vertices':len(model.data.vertices),'triangles':len(model.data.loop_triangles),'bones':len(rig.data.bones),'bone_names':[b.name for b in rig.data.bones],'dimensions_blender_units':list(model.dimensions),'world_scale_factor':S,'bad_weight_vertices':len(invalid),'max_influences':max(len(v.groups) for v in model.data.vertices),'rig_contract':'Exact mannequin bind skeleton; no added bones. Tail follows pelvis.','basis':{'right':list(RIGHT),'front':list(FRONT)}}
(OUT/'validation.json').write_text(json.dumps(report,indent=2))
bpy.ops.object.select_all(action='DESELECT');model.select_set(True);rig.select_set(True);bpy.context.view_layer.objects.active=rig
bpy.ops.export_scene.fbx(filepath=str(OUT/'SK_Mewtwo.fbx'),use_selection=True,object_types={'MESH','ARMATURE'},use_mesh_modifiers=True,add_leaf_bones=False,bake_anim=False,axis_forward='-Z',axis_up='Y',use_armature_deform_only=False,mesh_smooth_type='FACE')
# Studio presentation, excluded from FBX.
floor=mat('Studio',(.055,.067,.09),.85)
bpy.ops.mesh.primitive_plane_add(size=200*S,location=(0,0,-.02*S));bpy.context.object.data.materials.append(floor)
scene=bpy.context.scene;scene.render.engine='CYCLES';scene.cycles.samples=32
scene.world=bpy.data.worlds.new('StudioWorld');scene.world.color=(.2,.2,.2)
def point(o,p):o.rotation_euler=(p-o.location).to_track_quat('-Z','Y').to_euler()
for name,xyz,power,size in [('Key',(3,4,5),650,4),('Fill',(-4,2,3),450,3),('Rim',(0,-4,4),900,3)]:
 data=bpy.data.lights.new(name,'AREA');data.energy=power;data.shape='DISK';data.size=size*S
 o=bpy.data.objects.new(name,data);bpy.context.collection.objects.link(o);o.location=P(*xyz);point(o,P(0,0,1))
camdata=bpy.data.cameras.new('PreviewCamera');cam=bpy.data.objects.new('PreviewCamera',camdata);bpy.context.collection.objects.link(cam);scene.camera=cam
cam.location=P(3.6,6,2.8);point(cam,P(.1,-.07,1.07));camdata.type='ORTHO';camdata.ortho_scale=2.65*S
scene.render.resolution_x=1000;scene.render.resolution_y=1000;scene.render.resolution_percentage=100
scene.view_settings.view_transform='AgX';scene.render.image_settings.file_format='PNG';scene.render.filepath=str(OUT/'Mewtwo_preview.png')
rig.hide_render=True
bpy.data.orphans_purge(do_recursive=True)
bpy.ops.wm.save_as_mainfile(filepath=str(OUT/'Mewtwo.blend'))
bpy.ops.render.render(write_still=True)
print('MEWTWO_BUILD_COMPLETE',json.dumps(report))
