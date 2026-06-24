# Hollow Wilds — Unity → Unreal Migration Plan

> **Status:** Planning only. Nothing built yet.
> **Source (reference):** Unity 6 project `Hollow-Wilds-Unity` — `C:\Users\JarrydAnderson\Hollow Wilds`
> **Target:** Unreal Engine **5.8** C++ project `Hollow-Wilds` — `C:\Users\JarrydAnderson\Documents\Unreal Projects\HollowWilds`
> **Art:** `C:\UnrealAssets\Synty` (Unreal-native Synty packs you already own)

---

## 0. Scope (what we are actually migrating)

This is **not** a whole-game port of all ~44,000 lines / 150 scripts. The agreed scope is four things:

1. **Synty Meadow Forest demo scene** — stand the environment back up in Unreal as the base level.
2. **Combat, controls, mechanics** — the playable core (witch movement, camera, spells, abilities, enemies).
3. **FX** — the spell/impact/ambient visual effects that sell the combat.
4. **Hazel_V2** — the player character (mesh + materials + animation + control hookup).

Everything else in the Unity project (quests, inventory, dialogue, save, weather, bosses, fast travel, etc.) is **out of scope for now** but documented in the Unity repo and can be layered on later in the same C++/GAS structure.

---

## 1. Key technical decisions

### 1.1 No automatic conversion exists
Unity and Unreal share **no** code language (C# vs C++), scene format (`.unity` YAML vs `.umap`), or component model. There is no reliable tool that converts a project of this size. This is an **incremental re-implementation** using the Unity C# and the `Docs/` design bible as the **spec**, not as source to paste.

### 1.2 C++ for all logic — Blueprints only where Unreal forces it (my recommendation)
You asked for "C++ only, no Blueprints — unless I really think we should." My honest recommendation:

- **All gameplay logic in C++.** This matches how your Unity C# is structured, keeps everything diff-able and authorable as text, and scales for a project this size. Agreed.
- **A pure "zero Blueprints" project is not practical in Unreal**, and fighting that costs more than it saves. Several systems are *asset-based* and have no meaningful C++-only path:
  - **Enhanced Input** — Input Actions and Input Mapping Contexts are data assets.
  - **Animation Blueprints** — the state-machine/blendspace graph for Hazel is graph-based. (We write the `UAnimInstance` logic in C++, but the state machine lives in an AnimBP.)
  - **Niagara** — all FX are Niagara assets (the Unity FX were partly code-built; in Unreal they should be Niagara).
  - **UMG** — HUD widgets are assets (C++ `UUserWidget` base, WBP for layout).
  - **GAS** — abilities/effects can be authored as pure C++ classes; we'll do that, with thin Blueprint subclasses only to expose designer-tunable numbers + asset references.

**Net stance:** *C++ base classes for everything; Blueprints used only as thin data/config layers and for the asset-based systems above.* This is Epic's own recommended hybrid and is the fastest route to a maintainable result. We will not put gameplay logic in Blueprint graphs.

### 1.3 GAS (Gameplay Ability System) for combat — confirmed
Your combat maps onto GAS almost 1:1 (see §4). We'll enable `GameplayAbilities`, `GameplayTags`, `GameplayTasks` plugins.

---

## 2. Asset migration — the easy, mechanical part

### 2.1 Synty art (Meadow + others)
The Synty packs in `C:\UnrealAssets\Synty` are **already Unreal projects** (e.g. `POLYGON - Meadow Forest - Nature Biome\Polygon_Biomes_Meadow\Polygon_Biomes_Meadow.uproject`). Migration path:

1. Open each Synty `.uproject` in UE 5.8 (let it upgrade if prompted).
2. In the Content Browser, right-click the pack's content root → **Migrate…** → target `HollowWilds/Content/Synty/<PackName>`.
3. This carries meshes, materials, textures, and the **demo map** with all dependencies intact.

Packs in scope (others available when needed): **Meadow Forest – Nature Biome** (environment), **Particle FX Pack** (FX base), **Fantasy Rivals / Knights / Sidekick** (characters/enemies), **Dungeon**, **Bow and Crossbow**, **Fantasy Warrior HUD**, **Simple Sky**.

### 2.2 The Meadow demo scene (scope item 1)
Synty's Unreal Meadow pack ships its own demo/showcase map. Plan:
- Migrate the pack (above) → open the migrated **demo map** → **Save As** `Content/Maps/L_BriarHollow_Meadow.umap`. That becomes our base playable level.
- Set it as the editor + game default map in Project Settings.
- (Later) re-dress it toward the `BriarHollow` / "Mosswood Hollow" layout described in `Docs/BriarHollow.md` & `Docs/BriarHollow_Reimagined.md` from the Unity repo.

> Note: we do **not** parse the Unity `.unity` scene into Unreal. We use Synty's native Unreal demo map as the starting point, which is higher quality and lossless.

### 2.3 Hazel_V2 character (scope item 4)
Source: `Assets/Tripo/Hazel_v2/Hazel_v2.fbx` (Tripo-generated mesh + PBR textures `basecolor/metallic/normal/roughness`), plus `Assets/Prefabs/Player_Hazel.prefab` for the wiring intent, and the toon material `Hazel_v2_FlatKit.mat`.

Plan:
1. Import `Hazel_v2.fbx` as a **Skeletal Mesh** → `Content/Characters/Hazel/`.
2. Decide skeleton: retarget onto the **Synty/UE Mannequin skeleton** so we can reuse Synty + UE animations directly (strongly recommended), rather than Hazel's raw Tripo rig. Use **IK Retargeter** if Hazel's proportions differ.
3. Import the four locomotion FBX anims in `Assets/Animations/` (`Running`, `StandToRoll`, `SprintingForwardRoll`, `QuickRollToRun`) and retarget; fill gaps from Synty/UE anim libraries.
4. Materials: rebuild the **toon/cel look** (the Unity project used FlatKit) as an Unreal **post-process + cel-shaded master material**. This is a small art task tracked in Phase 5.

---

## 3. Unreal project foundation (Phase 0)

Before any gameplay:
- Enable plugins: `GameplayAbilities`, `EnhancedInput`, `Niagara`, `ModelingToolsEditorMode` (already on).
- `HollowWilds.Build.cs` dependencies: `GameplayAbilities`, `GameplayTags`, `GameplayTasks`, `EnhancedInput`, `Niagara`, `UMG`, `AIModule`, `NavigationSystem`.
- Module skeleton (C++ classes):
  - `AHWGameMode`, `AHWPlayerController`, `AHWPlayerState` (holds the AbilitySystemComponent for the player — standard for player-owned ASC).
  - `UHWAbilitySystemComponent`, base `UHWGameplayAbility`, base `UHWAttributeSet`.
  - `UHWGameplayTags` — native gameplay tag registry.
- Source layout under `Source/HollowWilds/`:
  ```
  Core/        Character/   Camera/    Abilities/   Attributes/
  Enemies/     AI/          FX/        Input/       UI/
  ```

---

## 4. Combat / controls / mechanics → Unreal mapping (Phases 1–4)

The Unity combat is cleanly designed; here is the concrete mapping. (Line counts are the Unity sources for reference effort.)

| Unity (C#) | Lines | Unreal target | Approach |
|---|---|---|---|
| `WitchController` (movement tiers, dodge-roll i-frames, slide, jump, aim steering) | 344 | `AHWWitchCharacter : ACharacter` + `UCharacterMovementComponent` tuning | C++; dodge i-frames as a `GameplayEffect` granting an Invulnerable tag |
| `ThirdPersonCamera` (hip + over-shoulder aim) | 219 | `USpringArmComponent` + `UCameraComponent`, two camera states | C++ on the character; aim toggles arm length/socket offset |
| `Element` + `ElementProfiles` (Fire/Ice/Wind/Earth tuning) | 46 | `EHWElement` enum + `UHWElementProfile` DataAsset + `GameplayTag` per element | C++ enum/tags; per-element numbers in a Data Asset |
| `Health` (HP + events) | 56 | `UHWAttributeSet` (`Health`, `MaxHealth`) + GAS damage `GameplayEffect` | GAS attributes; UI binds to attribute-change delegates |
| `Mana` (essence) | — | `UHWAttributeSet` (`Essence`, `MaxEssence`) | GAS attribute; abilities use native **Cost** GE |
| `WandCaster` (elemental bolt / staff melee / element swap) | 356 | `UGA_FireBolt` / `GA_Melee` abilities + `EHWElement` state | One bolt ability parameterized by element; melee separate ability |
| `Projectile` (flies, damages, applies status) | — | `AHWProjectile` (`UProjectileMovementComponent`) spawned by the ability | On hit: apply damage GE + element status GE |
| `AbilityCaster` (Blink E / Barrier R / Beam Q / Nova V) | 684 | 4 `UGameplayAbility` subclasses | Blink=dash+invuln GE; Barrier=shield GE w/ absorb attr; Beam=`UAbilityTask` channel; Nova=radial GE+impulse |
| Status effects (burn, slow/freeze, lift, lava) | — | `GameplayEffect`s (DoT, MoveSpeed mod, stun tag, etc.) | Native GAS — burn=periodic GE, slow=MoveSpeed multiplier, freeze=stun tag |
| `WeakPoint` (damage multiplier collider) | — | Component on enemy; ability damage execution reads it | C++ component; damage calc multiplies |
| `EnemyController` / `EnemyAnimator` / behaviors (Charger/Exploder/Healer/Summoner) | — | `AHWEnemyCharacter` + **Behavior Tree / StateTree** + AbilitySystemComponent | C++ pawns; AI via BT (assets) driving C++ tasks |

### Phased build order (within scope)
- **Phase 1 — Locomotion & camera:** Hazel character moves (jog/sprint/walk), dodge-roll w/ i-frames, slide, jump; third-person + aim camera. Enhanced Input mapping matching the Unity control table (WASD/Shift/Alt/Ctrl/Space/RMB/LMB/1-2-3/E-R-Q-V).
- **Phase 2 — GAS core:** ASC on PlayerState, AttributeSets (Health/Essence/Poise/Absorb), damage & cost/cooldown GEs, element tags + Data Assets.
- **Phase 3 — Spells:** elemental bolt (Fire/Ice/Wind/Earth) + projectile + per-element status GEs; staff melee; the 4 utility abilities (Blink/Barrier/Beam/Nova).
- **Phase 4 — Enemy slice:** one enemy (Bramblkin equivalent from Synty) with Health, WeakPoint, BT chase/attack, death; a simple wave spawner (mirrors the Unity Arena loop) dropped into the Meadow level.

---

## 5. FX (scope item 3) — Phase 5

The Unity FX were a mix of code-built (LineRenderer/primitives in `AbilityCaster`, `SpellFXLibrary`, `Assets/Scripts/VFX/*`) and Synty Particle FX. In Unreal these become **Niagara**:

- Migrate **POLYGON – Particle FX Pack** for base emitters.
- Per element: bolt trail, impact burst, lingering ground-fire (Fire), freeze shatter (Ice), wind vortex (Wind), lava fissure (Earth) — Niagara systems spawned by abilities/projectile-hit.
- Ability FX: Blink afterimage, Barrier dome, Beam ribbon, Nova shockwave.
- Hit feedback: damage numbers (UMG), hit-flash material param, camera shake, hit-stop (global time dilation, as Unity did).

---

## 6. Repo / source control

- Unity project remote is currently `github.com/HollandDrive/Hollow-Wilds.git`. Suggested: rename that repo to **`Hollow-Wilds-Unity`** (reference), and create a fresh **`Hollow-Wilds`** for this Unreal project.
- `gh` CLI is **not installed** on this machine — install it (or use the GitHub web UI) before we wire remotes. I can script the `git init` / `.gitignore` (Unreal-appropriate: ignore `Binaries/ Intermediate/ Saved/ DerivedDataCache/ .vs/`) and the initial commit when you're ready.
- **Do not commit** `C:\UnrealAssets\Synty` source packs or migrated Synty content as your own — keep them as marketplace dependencies per Synty's license; migrated content lives in `Content/Synty/` and is fine in your private repo.

---

## 7. Effort & risk summary

| Area | Effort | Risk |
|---|---|---|
| Synty asset + Meadow demo migration | Low | Low — native Unreal packs |
| Hazel import + retarget + toon material | Low–Med | Med — Tripo rig may need IK retarget; toon look is art iteration |
| Locomotion + camera (Phase 1) | Med | Low |
| GAS core + spells + abilities (Phases 2–3) | High | Med — GAS learning curve, but architecture fits |
| Enemy + AI slice (Phase 4) | Med | Med |
| FX in Niagara (Phase 5) | Med | Low–Med — art iteration |

**Recommended first build target once approved:** Phase 0 (foundation) + §2 asset migration → a Meadow level you can walk Hazel around in, before any combat.

---

## 8. Open questions for you

1. **Hazel skeleton:** retarget onto the Synty/UE Mannequin skeleton (reuse all Synty + UE anims — recommended), or keep Hazel's native Tripo rig?
2. **Toon look:** match the Unity FlatKit cel-shaded style now, or ship Synty's default flat-lit look first and stylize later?
3. **Enemy source:** which Synty pack is the Bramblkin stand-in (Fantasy Rivals? a creature from another pack)?
4. **`gh` install:** want me to set up git + `.gitignore` + initial commit for the new `Hollow-Wilds` repo once `gh` is available?
