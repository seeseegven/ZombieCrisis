**ZombieCrisis：目录与启动说明**

更新日期：2026年9月14日。

本项目用于学习UE及展示玩法实现。当前阶段为单人、单图PvE基础搭建；美术以可辨识的基础形状为主。伤害、血量和刷怪数量可以使用明确标注的自定参数，后续根据手感调整。

**日常入口**

在内容浏览器进入 `Content/ZombieCrisis`。新建资源均放在该目录内，避免与模板资源混放。编辑器启动地图和游戏默认地图已设为 `Maps/L_ZC_PVETest`。

| 目录 | 用途／已有内容 |
|---|---|
| `Maps` | 测试地图 `L_ZC_PVETest`；普通关卡，不使用世界分区 |
| `Core` | `BP_ZC_GameMode`、`BP_ZC_PlayerController`，独立于TopDown模板 |
| `Characters/Player` | `BP_ZC_Player`：基础Character、方块外观和斜俯视摄像机 |
| `Characters/Enemies` | `BP_ZC_BlockZombie`：由14个Cube部件组成的僵尸外观 |
| `Art/Materials/Blockout` | 地板、墙体、玩家和僵尸的简单颜色材质 |
| `Input` | 预留输入动作和映射资源 |
| `UI` | 预留HUD、菜单和结算界面 |
| `Data` | 预留武器、成长、敌人、波次数据 |
| `Audio` | 预留音效 |
| `FX` | 预留枪口、命中、爆炸等特效 |
| `Imported` | 预留外部素材；每个素材包使用独立子目录，保留来源与许可 |

未使用的预留目录暂为空；内容浏览器若隐藏空文件夹，可能不会显示它们。引擎的Cube和基础材质继续引用 `/Engine`，不需要复制。

**当前场景与玩法状态**

地图包含40×30米地板、四面围墙、两个障碍、玩家出生点、四个出怪标记、一只僵尸外观预览及基础灯光。大纲内统一放在 `ZombieCrisis` 文件夹，再按Geometry、Gameplay、Preview、Lighting分类。

新GameMode指定新玩家和控制器，地图同时显式指定同一个GameMode。蓝图目前作为玩法基础容器；移动输入、射击、敌人AI、伤害、波次和升级尚未实现。出怪标记目前只是位置，不会生成敌人。僵尸目前是Actor外观，不是已经能寻路的敌人Character。

验证已在实际PIE会话中确认：运行地图、GameMode、PlayerController、玩家类均来自新目录；玩家有一个摄像机和7个Cube外观部件；僵尸保留14个Cube部件并直立显示。检查新目录下13项资产的依赖，没有指向其他 `/Game` 目录的引用。此检查不等于完整PvE验收。

**旧内容与保留副本**

- `TopDown`、`Characters`、`Cursor`、`LevelPrototyping` 等原有目录仍保留，供参考；当前新地图不依赖它们。
- 原先 `Prototype` 中的僵尸蓝图和4个材质已通过UE资产移动功能移入新目录，引用随移动更新。以后使用 `Characters/Enemies/BP_ZC_BlockZombie`。
- 切换前的开放世界场景已另存为 `Content/_Archive/BeforeZombieCrisis/L_OpenWorld_BeforeSetup`，作为旧场景保留副本。
- `__ExternalActors__`、`__ExternalObjects__` 是UE管理世界分区外部资源的目录，应由UE处理，不要手动移动其中的文件。
- 改动前的地图与项目设置保存在项目的 `Saved/ZombieCrisisSetup/DefaultEngine.before.ini` 和 `DefaultGame.before.ini`；这是本机备份，不属于游戏内容。

**维护材料**

项目根目录的 `Scripts/Editor` 保存本次建图和验证脚本，运行结果位于 `Saved/ZombieCrisisSetup`。建图脚本只用于本次初始化，有防止重复建图／重复添加的检查；日常打开项目无需运行这些脚本。后续核心规则可以使用C++，蓝图继续配置外观、摄像机和其他资源。
