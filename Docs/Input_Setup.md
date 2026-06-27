# Hollow Wilds — Input Setup (Enhanced Input)

How player input is wired, the full control scheme, and the **hard-won gotcha**
that cost a lot of debugging time. UE 5.8, Enhanced Input.

---

## ⚠️ The gotcha (read this first)

**In UE 5.8, an Input Mapping Context stores its key mappings under
`DefaultKeyMappings.Mappings`, NOT the top-level `Mappings` property.**

We originally authored `IMC_Default` programmatically (via the MCP property API),
which set a property literally named `Mappings`. That is a **deprecated/ignored**
field. Reading it back showed the mappings (false positive), but the editor and the
runtime only use `DefaultKeyMappings.Mappings` — which stayed **empty**. Result:
the mapping context had **zero mappings**, so no key did anything, even though:

- `EnhancedInputLocalPlayerSubsystem->AddMappingContext(IMC, 0)` logged success,
- the `UInputAction` refs were assigned on the pawn,
- the value types (Axis2D) were correct,
- `DefaultPlayerInputClass`/`DefaultInputComponentClass` were EnhancedInput.

**Diagnosis that nailed it:** in PIE, open console (`~`) → `showdebug enhancedinput`.
It read **"No enhanced player input action mappings have been applied to this
input."** Opening `IMC_Default` in the editor showed an **empty Mappings list**.

**Rule: author Input Mapping Context entries in the editor** (or via the engine's
own input APIs) — not by poking the raw `Mappings` UPROPERTY. The C++ side
(`AddMappingContext`, action bindings) was correct all along.

---

## Control scheme

Author these in **`Content/Input/IMC_Default`** → Details → **Default Key Mappings →
Mappings** (click `+` per row). Modifiers are added per-mapping via the row's
**Modifiers** `+`.

| Key | Input Action | Modifiers | Notes |
|---|---|---|---|
| **W** | IA_Move | **Swizzle Input Axis Values** (YXZ) | forward (+Y) |
| **S** | IA_Move | **Swizzle** (YXZ) **+ Negate** | back (−Y) |
| **A** | IA_Move | **Negate** | left (−X) |
| **D** | IA_Move | *(none)* | right (+X) |
| **Mouse XY** (key = `Mouse2D`) | IA_Look | *(optional Negate → Y for invert)* | camera look |
| **Space Bar** | IA_Jump | *(none)* | ✅ confirmed working |
| **Left Shift** | IA_Sprint | *(none)* | hold to sprint |
| **Left Alt** | IA_Walk | *(none)* | hold to walk |
| **Left Ctrl** | IA_Dodge | *(none)* | → GA_Blink dodge |
| **Right Mouse Button** | IA_Aim | *(none)* | over-shoulder aim |
| **Left Mouse Button** | IA_Fire | *(none)* | bolt (aiming) / melee (hip) |
| **1 / 2 / 3** | IA_Element1/2/3 | *(none)* | Fire / Ice / Wind |

`IA_Move` and `IA_Look` must have **Value Type = Axis2D** (already set).

---

## How input is wired (the rest, which IS correct)

- **`AHWWitchCharacter`** (C++) declares `UInputAction*` refs + `DefaultMappingContext`,
  binds them in `SetupPlayerInputComponent`, and adds the mapping context via the
  `EnhancedInputLocalPlayerSubsystem` (in `SetupPlayerInputComponent` — runs after
  possession, controller guaranteed valid).
- **`BP_Witch`** (subclass) assigns the IA assets + `IMC_Default` in its class defaults.
- **`BP_HWGameMode`** sets `DefaultPawnClass = BP_Witch`; it's the GameMode override on
  `L_BriarHollow`.
- Combat handlers route to GAS: `DoFire` → `GA_ElementalBolt` (aiming) / `GA_Melee`
  (hip); `DoDodge` → `GA_Blink`. Abilities are granted on possess (`DefaultAbilities`).
  These fire once their keys are mapped (LMB / Left Ctrl above).

## Project settings (already set)

- `Config/DefaultInput.ini`: `DefaultPlayerInputClass=EnhancedPlayerInput`,
  `DefaultInputComponentClass=EnhancedInputComponent`.
- `EnhancedInput` plugin enabled; `EnhancedInput` in `Build.cs`.

## Not done yet

- **Gamepad**: only keyboard/mouse mapped. To add: `Gamepad Left Stick 2D-Axis`→IA_Move
  (with a Radial **Dead Zone**), `Gamepad Right Stick 2D-Axis`→IA_Look,
  `Gamepad Face Button Bottom`→IA_Jump, etc.
- **Barrier/Beam/Nova**: abilities exist + are granted, but need their own Input Actions
  + keys (e.g. R/Q/V) and handlers.
