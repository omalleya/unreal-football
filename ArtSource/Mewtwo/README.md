# Mewtwo gameplay prototype

Original procedurally authored fan-character geometry for the football prototype. No downloaded/ripped character mesh or textures. The common skeletal rig and locomotion originate from the project's Epic mannequin assets; their existing license still applies. This is a work-in-progress likeness, not a finished character.

## Deliverables

- `Mewtwo.blend`: editable source scene, shared rig and studio setup.
- `SK_Mewtwo.fbx`: Unreal skeletal-mesh interchange export.
- `Mewtwo_preview.png`: current reference-pose studio render.

Validation scripts generate local reports (`validation.json`, `unreal_validation.json`, `fbx_roundtrip_validation.json`, `animation_validation.json`) and run-frame renders. These generated files are ignored by Git. Regenerate them against the current model before using them as evidence; earlier local run previews predate the final revision.

## Integration

Use `/Game/Football/Characters/Mewtwo/DA_Mewtwo` with the shared `PlayerAppearance` system. It selects `SK_Mewtwo`, the exact existing mannequin skeleton asset, and `ThirdPerson_AnimBP`. The mesh keeps the common 68-bone hierarchy (67 Blender bones plus exported root), centimeter scale in Unreal, and unit appearance scale. Do not retarget or change movement/capsule settings for this appearance.

`FootballGrip` is a mesh socket parented to `hand_r`, using the shared provisional local offset `(8, 0, -7)` cm. This is an attachment contract, not a finished finger-contact/carry pose. Gameplay smoke testing and live visual inspection belong to the main task.

The body underside uses FBX vertex colors through `M_Mewtwo_Pale`; retain vertex colors on reimport. Other materials use constant colors. No external textures are needed.

## Reproduction

Run `Scripts/Art/export_reference_rig.py` in Unreal with `-run=pythonscript -AllowCommandletRendering` to produce the local reference rig. Then run Blender in background with `Scripts/Art/build_mewtwo.py`. Import using `Scripts/Art/import_mewtwo.py` in a fresh Unreal Python commandlet after the project's C++ module is built. These scripts only create/update this model's assets; they do not set the game's default appearance.

`validate_mewtwo_fbx.py` compares exported bind matrices to the reference; `export_reference_animation.py` and `preview_mewtwo_animation.py` produce deformation previews. Temporary exported mannequin geometry/animation and logs are excluded by the local `.gitignore`.

## Known limitations

- Prototype anatomy, topology and likeness; no production retopology, UV textures, facial rig, LODs, or character physics asset.
- Tail follows pelvis rigidly. Neck tube has basic neck weighting. Original fingers and limbs share the common rig; no special abilities or character-specific gameplay.
- Larger feet and hips need animation/IK polish. Earlier run previews exposed contact/crease issues; final skin-weight smoothing and neck/shoulder bridges have not yet received a final animated visual approval.
- Ball attachment is functional but exact palm/finger contact needs in-game calibration and a carry pose.
- The current in-game body material appears too dark; material polish remains pending.
- Blender FBX emits a bind-pose warning that Unreal recreates successfully. Existing skeleton identity and all 68 imported bone names/order are verified by the import script.
- Final appearance iteration stopped at the user's request to prioritize gameplay testing. No claim of an exact reference match or finished art.
