"""Build the initial test room in the new ZombieCrisis level, once."""

import json
from pathlib import Path
import traceback

import unreal


def main():
    output = Path(unreal.Paths.project_saved_dir()) / "ZombieCrisisSetup"
    output.mkdir(parents=True, exist_ok=True)
    result = {"success": False, "actors": []}
    try:
        world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
        if world.get_path_name() != "/Game/ZombieCrisis/Maps/L_ZC_PVETest.L_ZC_PVETest":
            raise RuntimeError("Open the ZombieCrisis test level before running this script.")
        actors = unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
        if any(a.get_actor_label().startswith("ZC_") for a in actors.get_all_level_actors()):
            raise RuntimeError("Blockout already exists; refusing to add duplicate actors.")

        root = "/Game/ZombieCrisis"
        cube_mesh = unreal.load_asset("/Engine/BasicShapes/Cube")
        floor_mat = unreal.load_asset(root + "/Art/Materials/Blockout/MI_ZC_Floor")
        wall_mat = unreal.load_asset(root + "/Art/Materials/Blockout/MI_ZC_Wall")

        def spawn(cls, label, position, folder, rotation=(0, 0, 0)):
            actor = actors.spawn_actor_from_class(
                cls, unreal.Vector(*position),
                unreal.Rotator(pitch=rotation[0], yaw=rotation[1], roll=rotation[2]),
            )
            if actor is None:
                raise RuntimeError("Could not spawn " + label)
            actor.set_actor_label(label)
            actor.set_folder_path(unreal.Name("ZombieCrisis/" + folder))
            result["actors"].append(label)
            return actor

        def block(label, position, size, material, folder="Geometry"):
            actor = spawn(unreal.StaticMeshActor, label, position, folder)
            component = actor.static_mesh_component
            component.set_static_mesh(cube_mesh)
            component.set_material(0, material)
            component.set_collision_profile_name("BlockAll")
            actor.set_actor_scale3d(unreal.Vector(*(v / 100.0 for v in size)))
            return actor

        block("ZC_Floor", (0, 0, -25), (4000, 3000, 50), floor_mat)
        for label, position, size in [
            ("North", (2000, 0, 80), (40, 3040, 160)),
            ("South", (-2000, 0, 80), (40, 3040, 160)),
            ("East", (0, 1500, 80), (4000, 40, 160)),
            ("West", (0, -1500, 80), (4000, 40, 160)),
        ]:
            block("ZC_Wall_" + label, position, size, wall_mat)
        block("ZC_Obstacle_A", (200, -650, 90), (260, 400, 180), wall_mat)
        block("ZC_Obstacle_B", (-500, 650, 90), (400, 260, 180), wall_mat)

        spawn(unreal.PlayerStart, "ZC_PlayerStart", (0, 0, 100), "Gameplay")
        for label, position in [
            ("North", (1800, 0, 100)),
            ("South", (-1800, 0, 100)),
            ("East", (0, 1300, 100)),
            ("West", (0, -1300, 100)),
        ]:
            spawn(unreal.TargetPoint, "ZC_SpawnPoint_" + label, position, "Gameplay/SpawnPoints")

        zombie = unreal.load_class(None, root + "/Characters/Enemies/BP_ZC_BlockZombie.BP_ZC_BlockZombie_C")
        spawn(zombie, "ZC_Zombie_VisualPreview", (600, 300, 2), "Preview", (0, 180, 0))

        sun = spawn(unreal.DirectionalLight, "ZC_Sun", (0, 0, 1400), "Lighting", (-45, -30, 0))
        light = sun.get_component_by_class(unreal.DirectionalLightComponent)
        light.set_mobility(unreal.ComponentMobility.MOVABLE)
        light.set_intensity(3.0)
        light.set_editor_property("atmosphere_sun_light", True)
        spawn(unreal.SkyAtmosphere, "ZC_SkyAtmosphere", (0, 0, 0), "Lighting")
        sky = spawn(unreal.SkyLight, "ZC_SkyLight", (0, 0, 1000), "Lighting")
        sky_component = sky.get_component_by_class(unreal.SkyLightComponent)
        sky_component.set_mobility(unreal.ComponentMobility.MOVABLE)
        sky_component.set_editor_property("real_time_capture", True)
        sky_component.set_intensity(1.0)

        gm = unreal.load_class(None, root + "/Core/BP_ZC_GameMode.BP_ZC_GameMode_C")
        world.get_world_settings().set_editor_property("default_game_mode", gm)
        result["game_mode"] = gm.get_path_name()
        if not unreal.get_editor_subsystem(unreal.LevelEditorSubsystem).save_current_level():
            raise RuntimeError("Level save failed.")
        result["success"] = True
    except Exception:
        result["error"] = traceback.format_exc()
        unreal.log_error(result["error"])
    (output / "build_blockout_result.json").write_text(
        json.dumps(result, ensure_ascii=False, indent=2), encoding="utf-8"
    )
    unreal.log("ZombieCrisis blockout: " + json.dumps(result, ensure_ascii=False))


main()
