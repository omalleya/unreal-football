# Swappable player models — version 1

Every player appearance runs on `APracticeCharacter`. The initial differences are visual. Models do not supply their own movement code, collision capsule, input handling, ball ownership, or speed settings.

## Rig and proportions

- Use the existing `/Game/Mannequin/Character/Mesh/SK_Mannequin_Skeleton` skeleton asset.
- Preserve the baseline bone names, hierarchy, orientations, and reference pose; matching names alone is insufficient. Export the mannequin rig as the reference for new geometry.
- Preserve the standard humanoid chains, including `root`, `pelvis`, `spine_03`, `hand_l`, `hand_r`, `foot_l`, and `foot_r`.
- Import at baseline humanoid scale in centimeters. Apply transforms in the authoring tool. The gameplay capsule remains radius 36 cm / half-height 96 cm. A big tail or ears do not enlarge that collision capsule.
- Author at approximately the same shoulder, hip, and hand positions initially. Large proportion differences require animation retargeting and contact correction before acceptance.
- Extra visual bones may be added only after verifying they do not change the baseline hierarchy or reference transforms. Do not casually regenerate the shared skeleton during import.

## Appearance asset

Create a `PlayerAppearance` Data Asset beside the character's mesh. It contains:

| Field | Contract |
| --- | --- |
| Mesh | Skeletal mesh using the shared skeleton |
| AnimationClass | Animation Blueprint targeting that same skeleton; initially `ThirdPerson_AnimBP` |
| MeshTransform | Baseline offset: location `(0, 0, -96)`, yaw `-90`, scale `(1, 1, 1)` |
| GripSocket | `FootballGrip` |
| GripOffset | Small position/rotation correction relative to the socket; scale must remain `(1, 1, 1)` |

`ApplyAppearance` validates compatibility before making changes. It leaves movement and collision settings intact and reattaches an already-carried football after a successful swap. Invalid assets leave the current mesh in place.

For development, launch with `-FootballAppearance=/Game/Football/Characters/YourCharacter/DA_YourCharacter`. `DefaultAppearancePath` under `[/Script/Football.PracticeCharacter]` in `Config/DefaultGame.ini` selects the normal default, currently `DA_Mewtwo`. `DA_PracticeMannequin` in `Characters/Shared` remains the fallback for invalid assets. Blueprints may also call `ApplyAppearance` on the existing player; a selection menu can use that same API later.

## Football grip

- Add a **mesh socket** named `FootballGrip`, parented to `hand_r`. Keep the shared skeleton unchanged when a particular hand needs a different socket offset.
- Treat the socket as the ball center, with its local X axis along the long axis of the football. Calibrate position and rotation on each mesh, at actual gameplay scale.
- The ball attaches to this socket when picked up. Loose-ball physics is disabled while held; dropping restores it. The ball stays 30 x 18 x 18 cm across appearances.
- The socket keeps the football connected to the animated hand. It does **not** automatically curl fingers, avoid body clipping, or create a tucked running pose. Those require the shared carry animation layer and hand-contact/IK calibration.
- `IsCarryingBall()` is exposed to the animation system as the common signal for that future carry layer. Throw and catch timings will also be shared gameplay signals.

## Acceptance check for each model

1. Verify the animation blueprint and skeleton match, unit scale, required bone chains, and right-hand grip socket.
2. Test idle, walk, sprint, turning, jumping, and landing for deformation or incorrect feet placement.
3. Pick up the ball, move and turn, and check that the grip stays aligned; inspect palm/finger contact and body clipping from several camera angles.
4. Swap appearances while holding the ball. Confirm ownership, ball size, controls, speed, and capsule are unchanged.
5. Drop and reset. Confirm physics resumes and the character still moves normally.

This first implementation supplies the swap/attachment contract and a baseline mannequin appearance. The existing locomotion animation is still a placeholder for football carrying. Final carry poses and per-model visual sign-off remain separate requirements; a socket alone is not a completed football grip.

Future character ratings or abilities should live in a separate gameplay profile, so choosing a visual model does not accidentally change the rules.
