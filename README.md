# ZombieCrisis

使用 **Unreal Engine 5.8** 制作的单人僵尸生存练习项目，参考《僵尸危机3》（Boxhead: 2Play Rooms）的俯视战斗、武器和倍率成长玩法。

项目以学习 UE 和积累可展示的玩法实现为目标。第一阶段只做单人、单图 PvE；美术优先使用引擎基础形状和免费素材，重点投入移动、射击、伤害、敌人行为与波次逻辑。

## 当前进度

已完成：

- 独立资源目录 `Content/ZombieCrisis`。
- 普通测试关卡 `L_ZC_PVETest`，含地板、围墙、障碍、玩家出生点和出怪位置标记。
- 独立的 GameMode、玩家控制器和玩家蓝图，包含斜俯视摄像机。
- 方块玩家外观和由 14 个 Cube 部件组成的僵尸外观。
- 默认启动地图与玩法类配置，以及基础 PIE 运行检查。

待实现：

- 玩家移动输入、朝向、射击与武器切换。
- 普通僵尸追击、恶魔远程攻击与受击压制。
- 生命、伤害、死亡结算和重新开始。
- 波次刷怪、补给、倍率成长和装备解锁。
- HUD、音效、特效及性能优化。

**目前是可以进入运行画面的玩法基础工程，还不是可完整游玩的游戏。** 出怪标记不会自动生成敌人，场景内的僵尸是静态外观预览；当前玩家也尚未配置移动或射击按键。

## 打开项目

环境要求：Windows、Unreal Engine 5.8、Git 和 Git LFS。建议使用创建工程时对应的 UE 版本。

1. 安装并初始化 Git LFS：`git lfs install`。
2. 克隆本仓库后，在项目目录运行 `git lfs pull`，确保取得真实 UE 资产，而不是 LFS 指针文件。
3. 双击 `ZombieCrisis.uproject`。首次打开会生成缓存并可能编译着色器。
4. 默认打开 `Content/ZombieCrisis/Maps/L_ZC_PVETest`；若未自动打开，可在内容浏览器中手动打开它。
5. 点击运行，检查玩家与斜俯视画面；按 Esc 停止。

项目描述文件启用了当前 UE 5.8 环境中的编辑器工具插件，包括 MCP／Toolsets。换机时需确认这些插件可用。本机助手连接设置不在仓库内，普通查看工程不需要连接助手。

当前已包含 `Source` 目录和 C++ 玩家类 `AZCPlayerCharacter`，移动控制尚待实现。蓝图继续配置外观与动画。C++ 开发需要 Visual Studio 的 C++ 工具链和 Windows SDK；本机已验证 VS 2022 17.14、MSVC 14.44.35222、Windows SDK 10.0.22621.0 可以构建 `ZombieCrisisEditor / Development / Win64`。首次打开没有编译输出的工程时，需要先构建编辑器目标或按 UE 提示编译项目模块。

## 目录约定

新玩法资源统一放在 `Content/ZombieCrisis`，避免与模板内容混放。

| 路径 | 用途 |
|---|---|
| `Content/ZombieCrisis/Maps` | 当前测试地图和后续玩法地图 |
| `Content/ZombieCrisis/Core` | GameMode、玩家控制器等玩法入口 |
| `Content/ZombieCrisis/Characters` | 玩家与敌人蓝图 |
| `Content/ZombieCrisis/Art` | 材质及美术资源 |
| `Content/ZombieCrisis/Input` | 输入动作和映射 |
| `Content/ZombieCrisis/UI` | HUD、菜单和结算界面 |
| `Content/ZombieCrisis/Data` | 武器、敌人、成长和波次配置 |
| `Content/ZombieCrisis/Audio`、`FX` | 音效和特效 |
| `Content/ZombieCrisis/Imported` | 外部素材；按素材包分目录保存 |
| `Config` | 需要共享的项目配置 |
| `Docs` | 需求与目录说明 |
| `Scripts/Editor` | 一次性建图和编辑器验证脚本 |

`TopDown`、`Characters`、`Cursor`、`LevelPrototyping` 等旧目录保留供参考。切换前的开放世界地图保存在 `Content/_Archive/BeforeZombieCrisis`。`Content/__ExternalActors__` 和 `Content/__ExternalObjects__` 中可能有旧地图需要的资源，因此一并提交，不要手动移动或删除。

预留目录使用 `.gitkeep` 保持克隆后的结构。编辑器建图脚本有防止重复创建的检查，正常打开和运行工程不需要执行这些脚本。

## 版本管理

- `.uasset`、`.umap` 由 Git LFS 管理；蓝图和地图属于二进制资产。
- `Saved`、`Intermediate`、`DerivedDataCache`、编译输出与 IDE 临时文件不提交。
- `.codex` 和 `Config/User*.ini` 属于本机配置，不提交。本机 Android 文件服务凭据保存在被忽略的 `Config/UserEngine.ini`。
- 修改蓝图或地图后，先在 UE 中保存，再查看 Git 变更并提交。
- 资产改名和移动应在 UE 内容浏览器中完成，使引用随之更新。
- 提交信息使用中文，说明本次变更，例如：`实现玩家八方向移动`、`修复倍率升级重复触发`。

忽略规则参考 [GitHub UnrealEngine.gitignore](https://github.com/github/gitignore/blob/main/UnrealEngine.gitignore)，针对本项目保留美术源文件和地图外部资源。LFS 使用方式参考 [GitHub 官方说明](https://docs.github.com/en/repositories/working-with-files/managing-large-files/configuring-git-large-file-storage)。

## 项目文档

- [目录与启动说明](Docs/ZombieCrisis目录与启动说明.md)
- [玩法参考与第一版需求报告](Docs/僵尸危机3玩法复刻-第一版需求报告.md)

需求报告保留原作调研依据；后续已明确以 UE 学习和逻辑实现为主。尚未核实的伤害、血量和刷怪数值可使用标注清楚的项目参数，逐步调试，不要求先完成原版数值考据。
