"""Run inside PIE; validates animation only and restores each actor to idle."""
import unreal, json, traceback
from pathlib import Path
report={"success":False,"checks":[]}
changed=[]
try:
 world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_game_world()
 assert world, "Start PIE first"
 player=unreal.GameplayStatics.get_player_pawn(world,0)
 zombies=unreal.GameplayStatics.get_all_actors_of_class(world,unreal.load_class(None,"/Game/ZombieCrisis/Characters/Enemies/BP_ZC_BlockZombie.BP_ZC_BlockZombie_C"))
 assert player and zombies
 for actor,zombie in [(player,False),(zombies[0],True)]:
  changed.append((actor,actor.is_actor_tick_enabled()))
  actor.set_actor_tick_enabled(False)
  actor.call_method("ResetZCAnimation")
  initial=actor.get_actor_transform()
  meshes={m.get_name():m for m in actor.get_components_by_class(unreal.StaticMeshComponent)}
  leg=meshes["Leg_L"]
  arm=meshes["Arm_L"]
  rest={n:(m.get_editor_property("relative_location"),m.get_editor_property("relative_rotation")) for n,m in meshes.items()}
  actor.call_method("SetAnimationSpeed",(-50.0,))
  assert actor.get_editor_property("AnimationSpeed")==0
  actor.call_method("SetAnimationSpeed",(150.0 if zombie else 300.0,))
  angles=[]
  for _ in range(80):
   actor.call_method("UpdateZCAnimation",(1/60,))
   angles.append(leg.get_editor_property("relative_rotation").pitch)
  assert max(angles)>15 and min(angles)<-15, str(angles)
  if zombie:
   actor.call_method("PlayAttackAnimation")
   actor.call_method("UpdateZCAnimation",(0.1,))
   t=actor.get_editor_property("ZCAnimationAttackTime")
   actor.call_method("PlayAttackAnimation")
   assert actor.get_editor_property("ZCAnimationAttackTime")==t,"Attack restarted"
   armangles=[]
   for _ in range(50):
    actor.call_method("UpdateZCAnimation",(1/60,))
    armangles.append(arm.get_editor_property("relative_rotation").pitch)
   assert max(armangles)>45 and min(armangles)<-20, str(armangles)
   assert not actor.get_editor_property("IsAttackAnimationPlaying")
   report["checks"].append("Zombie attack winds up, swipes, recovers; retrigger ignored")
  actor.call_method("SetAnimationSpeed",(0.0,))
  for _ in range(120):
   actor.call_method("UpdateZCAnimation",(1/60,))
  assert abs(leg.get_editor_property("relative_rotation").pitch)<0.05
  actor.call_method("ResetZCAnimation")
  for n,(loc,rot) in rest.items():
   m=meshes[n]
   assert (m.get_editor_property("relative_location")-loc).length()<0.01,n
   assert abs(m.get_editor_property("relative_rotation").pitch-rot.pitch)<0.01,n
  assert (actor.get_actor_location()-initial.translation).length()<0.01
  report["checks"].append(("Zombie" if zombie else "Player")+" walk, stop blend, exact reset and unchanged actor location")
 report["success"]=True
except:
 report["error"]=traceback.format_exc()
finally:
 for actor,ticking in changed:
  try:
   actor.call_method("ResetZCAnimation")
   actor.set_actor_tick_enabled(ticking)
  except:
   pass
 Path(unreal.Paths.project_saved_dir(),"CameraAdjust","animation_test.json").write_text(json.dumps(report,ensure_ascii=False,indent=2),encoding="utf-8")

