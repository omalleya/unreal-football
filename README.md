# Football

A small Unreal Engine **5.8.3** practice prototype for an original American-football game: one animated placeholder player, a full-size marked field, and a physical ball.

## Play on macOS

Open `Football.uproject` in Unreal Engine 5.8.3, build the Football module if prompted, and press **Play**. Click the viewport to capture input; Shift+F1 releases the mouse and Escape stops Play in Editor.

Or, from the project folder:

```sh
bash Scripts/build_macos.sh
bash Scripts/play_macos.sh
```

The scripts default to `/Users/Shared/Epic Games/UE_5.8`; set `UE_ROOT` to use another installation path. Close the running game/editor before rebuilding the C++ module. The standalone test window can be closed with Cmd+Q.

| Control | Action |
| --- | --- |
| WASD | Move relative to camera |
| Mouse | Orbit camera |
| Left Shift | Sprint |
| Space | Jump |
| E | Pick up a ball within 2 meters, or drop the carried ball |
| R | Reset player and ball to the starting area |

The ball starts ahead of the player at the home 10-yard line. It has gravity and collision when loose. Carrying attaches it to the animated `FootballGrip` hand socket. A football carry pose and finger-contact calibration are still needed. Leaving the practice area or falling below it resets the affected actor.

Mewtwo is selected through `DefaultAppearancePath` in `Config/DefaultGame.ini`. To test another compatible model without changing the project setting, pass its appearance asset, for example:

```sh
bash Scripts/play_macos.sh -FootballAppearance=/Game/Football/Characters/Shared/DA_PracticeMannequin.DA_PracticeMannequin
```

Each appearance supplies its mesh, shared animation blueprint, and hand-grip adjustment. Movement and ball handling stay on the same player class.

## Structure

- `Source/Football/PracticeCharacter.*`: movement, camera, Enhanced Input, interaction requests.
- `Source/Football/PlayerAppearance.*`: validated mesh/animation/grip configuration for swapping models without changing gameplay. See [the player model contract](PLAYER_MODEL_CONTRACT.md).
- `Source/Football/PracticeBall.*`: pickup ownership, attachment, loose-ball physics, reset.
- `Source/Football/PracticeField.*`: reusable field actor with yard lines, hashes, labels, endzones, and goalposts. Units are centimeters; the field is 120 by 53 1/3 yards including endzones.
- `Source/Football/PracticeGameMode.*`: chooses the pawn/HUD and spawns the ball.
- `Source/Football/PracticeHUD.*`: a minimal Canvas HUD that displays controls and ball status; it does not own gameplay state.
- `Content/Football/`: project map and materials.
- `Content/Mannequin/`: Epic's bundled template mannequin, animation, and material assets, retained at their original paths so references remain valid.

This milestone has no teams, passing, tackles, scoring, or networking. The Mewtwo prototype and fallback mannequin share the template humanoid skeleton and locomotion. Lighting is intentionally modest, with Lumen and virtual shadow maps disabled for the initial Mac test.

## Verification

An opt-in runtime integration check exercises ground collision, movement, pickup/drop, compatible appearance swaps while carrying, invalid-grip rejection, preserved movement/collision settings, distant pickup rejection, reset, and jump/landing:

```sh
bash Scripts/play_macos.sh -FootballSmokeTest -unattended -nosound
```

The test closes its process when finished. Search `Saved/Logs/Football.log` for `FOOTBALL_TEST RESULT: PASS` or `FAIL`; use that marker rather than the process exit status on macOS. This does not replace a visual/input test or a packaged build test.

The saved map is intended to be opened directly. `Scripts/create_practice_map.py` documents its initial generation and refuses to overwrite an existing map.

## Windows

Use the same engine version, with its supported Visual Studio C++ tools and Windows SDK. Generate project files from `Football.uproject`, build the Development Editor / Win64 target, then open the project. Windows compilation and packaged builds require separate validation; this prototype's initial checks run on macOS.

## Source control and assets

Track `Source`, `Config`, `Content`, scripts, and the `.uproject`. Generated `Binaries`, `Intermediate`, `Saved`, and shader caches are ignored. Asset files are marked binary. Git LFS is not installed on the development Mac; configure it before adding large production art or animation libraries. The current small template asset set can be stored in ordinary Git.

The mannequin assets originate from the locally installed Epic Unreal Engine template resources and remain subject to Epic's applicable license. New prototype code, field layout, and ball assembly were created for this project. There are no copied Madden assets or branding.
