"""Build block-character animation graphs. Run inside Unreal Editor Python.
No input binding, actor movement, AI, damage, or map changes.
"""
import json
import re
import traceback
from pathlib import Path
import unreal
import toolset_registry
from editor_toolset.toolsets import blueprint_dsl as D
from editor_toolset.toolsets.blueprint import BlueprintTools as B, _BlueprintCache
from editor_toolset.toolsets.actor import ActorTools as A

ROOT = "/Game/ZombieCrisis/Characters/"
REPORT = Path(unreal.Paths.project_saved_dir()) / "CameraAdjust" / "animation_build.json"
result = {"success": False, "assets": []}
language = unreal.InternationalizationLibrary.get_current_language()

def vector(v):
    return f"(Math|Vector|MakeVector :X {v[0]} :Y {v[1]} :Z {v[2]})"


def write(graph, code):
    def create(g, tid, pos):
        # UE exposes some category labels in the editor language even during a
        # temporary language switch. Resolve the exact same node title.
        title = tid.split("|")[-1]
        candidates = B.find_node_types(g, title, [])
        matches = [t for t in candidates if t.split("|")[-1] == title]
        if tid not in candidates and len(matches) == 1:
            tid = matches[0]
        return B.create_node(g, tid, pos)
    with toolset_registry.tool_raising_exceptions():
        D.Transpiler(graph, create, B.connect_pins, B._get_node_info,
                     B.set_pin_value, lambda g: B.find_nodes(g),
                     delete_node_fn=B.delete_node,
                     find_node_types_fn=lambda f: B.find_node_types(graph,f,[])).transpile(code)
        B.compile_blueprint(unreal.Blueprint.cast(graph.get_outer()),True)

def build(path, zombie):
    bp = unreal.load_asset(path)
    event = B.get_graph(bp, "EventGraph")
    old = B.read_graph_dsl(event)
    if any(token not in {"event", "OtherActor", "DeltaSeconds"} and not token.endswith("|UpdateZCAnimation") for token in re.findall(r"\(([^\s()]+)", old)):
        raise RuntimeError("Existing event logic needs preservation: " + path)
    variables = {
        "AnimationSpeed": 0.0, "ZCAnimationPhase": 0.0,
        "ZCAnimationBlend": 0.0, "ZCAnimationSwing": 0.0,
    }
    if zombie:
        variables.update({"ZCAnimationAttackTime": 0.0, "ZCAnimationAttackPitch": 0.0,
                          "IsAttackAnimationPlaying": False})
    existing = B.list_variables(bp)
    for name, value in variables.items():
        if name not in existing:
            B.add_variable(bp, name, "bool" if isinstance(value, bool) else "float")
    B.compile_blueprint(bp, True)
    cdo = B.get_default_object(bp)
    for name, val in variables.items():
        cdo.set_editor_property(name, val)
    parts = {}
    for part in A.get_components(cdo, unreal.StaticMeshComponent.static_class()):
        name = part.get_name().replace("_GEN_VARIABLE", "")
        loc = part.get_editor_property("relative_location")
        scale = part.get_editor_property("relative_scale3d")
        parts[name] = {"position": (loc.x, loc.y, loc.z), "scale": (scale.x, scale.y, scale.z)}
    # Node IDs contain localized/display names; discover them in this graph.
    def node(g, title):
        candidates = B.find_node_types(g, title, [])
        matches = [x for x in candidates if x.split("|")[-1] == title]
        if not matches:
            raise RuntimeError("Node not found: " + title)
        matches.sort(key=lambda x: (0 if x.startswith("Variables|") else 1, len(x)))
        chosen = matches[0]
        B.get_node_type_pins(g, chosen)
        return chosen
    def getter(g, name):
        return "(" + node(g, "Get" + name.replace("_","")) + ")"
    def setter(g, name, expr):
        return "(" + node(g, "Set" + name.replace("_","")) + " " + expr + ")"
    def function(name, params):
        g = B.add_function_graph(bp, name)
        # Functions are added once; rebuilding existing signature is safe.
        if True:
            present = {p.name for n in B.find_nodes(g) if isinstance(n, unreal.K2Node_FunctionEntry) for p in B._get_node_info(n).output_pins}
            for pn, pt in params:
                if pn in present:
                    continue
                if pt == "SceneComponent":
                    B.add_object_function_param(g, pn, unreal.SceneComponent.static_class(), True)
                else:
                    B.add_function_param(g, pn, pt, True)
        B.compile_blueprint(bp, True)
        return g
    # Rotate around shoulder/hip instead of the cube's center; no hierarchy changes.
    pose = function("ZCAnimationPosePart", [
        ("Part","SceneComponent"), ("Base","Vector"),
        ("Pivot","Vector"), ("Angle","float")])
    rot = node(pose,"MakeRotator")
    rotate = node(pose,"RotateVector")
    transform = node(pose,"SetRelativeLocationAndRotation")
    code = f"""(fn ZCAnimationPosePart (Part Base Pivot Angle)
      (bind r ({rot} :Pitch Angle :Yaw 0.0 :Roll 0.0))
      ({transform} :self Part
        :NewLocation (+ Pivot ({rotate} :A (- Base Pivot) :B r))
        :NewRotation r :bSweep false :bTeleport true))"""
    write(pose, code)

    speed = function("SetAnimationSpeed", [("Speed","float")])
    clamp = node(speed,"Clamp(Float)")
    write(speed, f"""(fn SetAnimationSpeed (Speed)
      {setter(speed,"AnimationSpeed",f"({clamp} :Value Speed :Min 0.0 :Max 900.0)")})""")

    if zombie:
        attack = function("PlayAttackAnimation", [])
        write(attack, f"""(fn PlayAttackAnimation ()
          (if (not {getter(attack,"IsAttackAnimationPlaying")})
            {setter(attack,"ZCAnimationAttackTime","0.0")}
            {setter(attack,"ZCAnimationAttackPitch","0.0")}
            {setter(attack,"IsAttackAnimationPlaying","true")}))""")

    update = function("UpdateZCAnimation", [("DeltaSeconds","float")])
    get = lambda n: getter(update,n)
    setv = lambda n,e: setter(update,n,e)
    interp = node(update,"FInterpTo")
    clamp = node(update,"Clamp(Float)")
    sin = node(update,"Sin(Degrees)")
    posecall = node(update,"ZCAnimationPosePart")
    phase = get("ZCAnimationPhase")
    blend = get("ZCAnimationBlend")
    swing = get("ZCAnimationSwing")
    speedget = get("AnimationSpeed")
    freq = 1.15 if zombie else 1.8
    reference = 150.0 if zombie else 300.0
    amp = 22.0 if zombie else 30.0
    lines = [
        "(fn UpdateZCAnimation (DeltaSeconds)",
        f'(bind dt ({clamp} :Value DeltaSeconds :Min 0.0 :Max 0.1))',
        setv("ZCAnimationBlend",f"({interp} :Current {blend} :Target ({clamp} :Value (/ {speedget} {reference}) :Min 0.0 :Max 1.0) :DeltaTime dt :InterpSpeed 10.0)"),
        setv("ZCAnimationPhase",f"(+ {phase} (* dt (* {freq*360.0} ({clamp} :Value (/ {speedget} {reference}) :Min 0.25 :Max 2.0))))"),
        f"(if (>= {phase} 360.0) " + setv("ZCAnimationPhase",f"(- {phase} 360.0)") + ")",
        setv("ZCAnimationSwing",f"(* ({sin} :A {phase}) (* {amp} {blend}))"),
    ]
    if zombie:
        elapsed = get("ZCAnimationAttackTime")
        lines += [
            f'(if {get("IsAttackAnimationPlaying")}',
            setv("ZCAnimationAttackTime",f"(+ {elapsed} dt)"),
            setv("ZCAnimationAttackPitch",
                 f"(select (< {elapsed} 0.2) (* (/ {elapsed} 0.2) 55.0) "
                 f"(select (< {elapsed} 0.35) (- 55.0 (* (/ (- {elapsed} 0.2) 0.15) 85.0)) "
                 f"(* -30.0 (- 1.0 ({clamp} :Value (/ (- {elapsed} 0.35) 0.4) :Min 0.0 :Max 1.0)))))"),
            f'(if (>= {elapsed} 0.75)',
            setv("IsAttackAnimationPlaying","false"),
            setv("ZCAnimationAttackPitch","0.0") + "))",
        ]
    for name in ["Leg_L","Leg_R","Arm_L","Arm_R"] + (["Foot_L","Foot_R","Hand_L","Hand_R"] if zombie else []):
        base = parts[name]["position"]
        side = name[-1]
        kind = "Leg_" if name.startswith(("Leg","Foot")) else "Arm_"
        main = parts[kind+side]
        p = main["position"]
        if zombie and kind == "Arm_":
            pivot = (p[0] - main["scale"][0]*50, p[1], p[2])
            walk = f"(* {swing} {0.28 if side=='L' else -0.28})"
            angle = f"(select {get('IsAttackAnimationPlaying')} {get('ZCAnimationAttackPitch')} {walk})"
        else:
            pivot = (p[0], p[1], p[2]+main["scale"][2]*50)
            sign = 1.0 if side == "L" else -1.0
            if kind == "Arm_":
                sign *= -0.8
            angle = f"(* {swing} {sign})"
        lines.append(f"({posecall} :Part {get(name)} :Base {vector(base)} :Pivot {vector(pivot)} :Angle {angle})")
    lines.append(")")
    write(update, "\n".join(lines))
    B.add_event(bp, "ReceiveTick")
    event = B.get_graph(bp, "EventGraph")
    updatecall = node(event,"UpdateZCAnimation")
    write(event, f"""(event EventTick (DeltaSeconds)
      ({updatecall} :DeltaSeconds DeltaSeconds))""")
    # A reset endpoint avoids leaving the last pose when disabling animation.
    reset = function("ResetZCAnimation", [])
    resetlines = ["(fn ResetZCAnimation ()"]
    for n in variables:
        resetlines.append(setter(reset,n,"false" if isinstance(variables[n],bool) else "0.0"))
    resetlines.append(f"({node(reset,'UpdateZCAnimation')} :DeltaSeconds 0.0))")
    write(reset, "\n".join(resetlines))
    B.compile_blueprint(bp, True)
    # Newly compiled CDO keeps defaults explicitly at rest.
    cdo = B.get_default_object(bp)
    for name, val in variables.items():
        cdo.set_editor_property(name,val)
    tick = cdo.get_editor_property("primary_actor_tick")
    tick.set_editor_property("start_with_tick_enabled",True)
    cdo.set_editor_property("primary_actor_tick",tick)
    if not unreal.EditorAssetLibrary.save_loaded_asset(bp, False):
        raise RuntimeError("Save failed: "+path)
    return {"path":path,"parts":parts,"functions":["SetAnimationSpeed","UpdateZCAnimation","ResetZCAnimation"]+(["PlayAttackAnimation"] if zombie else [])}

try:
    unreal.InternationalizationLibrary.set_current_language("en", False)
    _BlueprintCache._graph_node_types.clear()
    _BlueprintCache._macros = None
    result["assets"].append(build(ROOT+"Player/BP_ZC_Player",False))
    result["assets"].append(build(ROOT+"Enemies/BP_ZC_BlockZombie",True))
    result["success"] = True
except:
    result["error"] = traceback.format_exc()
    unreal.log_error(result["error"])
finally:
    unreal.InternationalizationLibrary.set_current_language(language, False)
    _BlueprintCache._graph_node_types.clear()
    _BlueprintCache._macros = None
    REPORT.write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding="utf-8")


