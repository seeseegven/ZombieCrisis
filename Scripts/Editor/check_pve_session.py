"""Read-only check of the active PIE session; writes a local verification result."""

import json
from pathlib import Path
import traceback

import unreal

result = {"success": False}
try:
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
    if world is None:
        raise RuntimeError("No PIE world is running.")
    controller = unreal.GameplayStatics.get_player_controller(world, 0)
    pawn = unreal.GameplayStatics.get_player_pawn(world, 0)
    game_mode = unreal.GameplayStatics.get_game_mode(world)
    result.update({
        "world": world.get_path_name(),
        "game_mode": game_mode.get_class().get_path_name(),
        "controller": controller.get_class().get_path_name(),
        "pawn": pawn.get_class().get_path_name(),
        "pawn_location": str(pawn.get_actor_location()),
        "camera_count": len(pawn.get_components_by_class(unreal.CameraComponent)),
        "player_mesh_count": len(pawn.get_components_by_class(unreal.StaticMeshComponent)),
    })
    assert result["game_mode"] == "/Game/ZombieCrisis/Core/BP_ZC_GameMode.BP_ZC_GameMode_C"
    assert result["controller"] == "/Game/ZombieCrisis/Core/BP_ZC_PlayerController.BP_ZC_PlayerController_C"
    assert result["pawn"] == "/Game/ZombieCrisis/Characters/Player/BP_ZC_Player.BP_ZC_Player_C"
    assert result["camera_count"] == 1
    zombie_cls = unreal.load_class(None, "/Game/ZombieCrisis/Characters/Enemies/BP_ZC_BlockZombie.BP_ZC_BlockZombie_C")
    zombies = unreal.GameplayStatics.get_all_actors_of_class(world, zombie_cls)
    result["zombie_preview_count"] = len(zombies)
    result["zombie_mesh_counts"] = [len(z.get_components_by_class(unreal.StaticMeshComponent)) for z in zombies]
    def mesh_details(actor):
        return [{"name": c.get_name(), "mesh": c.static_mesh.get_path_name() if c.static_mesh else None,
                 "location": str(c.get_editor_property("relative_location")),
                 "scale": str(c.get_editor_property("relative_scale3d")),
                 "parent": c.get_attach_parent().get_name() if c.get_attach_parent() else None,
                 "visible": c.is_visible(), "hidden_in_game": c.get_editor_property("hidden_in_game"),
                 "material": c.get_material(0).get_path_name() if c.get_material(0) else None}
                for c in actor.get_components_by_class(unreal.StaticMeshComponent)]
    result["player_meshes"] = mesh_details(pawn)
    result["zombie_meshes"] = [mesh_details(z) for z in zombies]
    player_parts = [m for m in result["player_meshes"] if m["mesh"] == "/Engine/BasicShapes/Cube.Cube"]
    assert len(player_parts) == 7
    assert result["zombie_mesh_counts"] == [14]
    assert abs(zombies[0].get_actor_rotation().pitch) < 0.1
    assert abs(zombies[0].get_actor_rotation().roll) < 0.1
    result["success"] = True
except Exception:
    result["error"] = traceback.format_exc()
    unreal.log_error(result["error"])
output = Path(unreal.Paths.project_saved_dir()) / "ZombieCrisisSetup" / "pie_check.json"
output.write_text(json.dumps(result, ensure_ascii=False, indent=2), encoding="utf-8")
unreal.log("ZombieCrisis PIE check: " + json.dumps(result, ensure_ascii=False))
