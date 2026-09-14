"""Run in Unreal's Python console to create the isolated PvE workspace once."""

import json
from pathlib import Path
import traceback

import unreal


def main():
    project = Path(unreal.Paths.project_dir()).resolve()
    output = project / "Saved" / "ZombieCrisisSetup"
    output.mkdir(parents=True, exist_ok=True)
    report = {"success": False}
    try:
        target = "/Game/ZombieCrisis/Maps/L_ZC_PVETest"
        if unreal.EditorAssetLibrary.does_asset_exist(target):
            raise RuntimeError("Target map already exists; refusing to replace it.")

        editor = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem)
        levels = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
        world = editor.get_editor_world()
        report["previous_world"] = world.get_path_name()
        archive = "/Game/_Archive/BeforeZombieCrisis/L_OpenWorld_BeforeSetup"
        if unreal.EditorAssetLibrary.does_asset_exist(archive):
            raise RuntimeError("Archive already exists; refusing to replace it.")
        unreal.EditorAssetLibrary.make_directory("/Game/_Archive/BeforeZombieCrisis")
        if not unreal.EditorLoadingAndSavingUtils.save_map(world, archive):
            raise RuntimeError("Could not preserve the existing map; stopping before switching.")
        if not levels.save_all_dirty_levels():
            raise RuntimeError("Could not save the archived world's external actors.")
        report["archive"] = archive
        del world

        if not levels.new_level(target, is_partitioned_world=False):
            raise RuntimeError("Could not create the non-partitioned PvE map.")
        report["map"] = target
        report["success"] = True
    except Exception:
        report["error"] = traceback.format_exc()
        unreal.log_error(report["error"])
    (output / "create_level_result.json").write_text(
        json.dumps(report, ensure_ascii=False, indent=2), encoding="utf-8"
    )
    unreal.log("ZombieCrisis workspace: " + json.dumps(report, ensure_ascii=False))


main()
