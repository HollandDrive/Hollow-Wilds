#include "Core/HWGameplayTags.h"

namespace HWGameplayTags
{
	// Abilities
	UE_DEFINE_GAMEPLAY_TAG(Ability_Spell_Bolt, "Ability.Spell.Bolt");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Melee, "Ability.Melee");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Blink, "Ability.Blink");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Barrier, "Ability.Barrier");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Beam, "Ability.Beam");
	UE_DEFINE_GAMEPLAY_TAG(Ability_Nova, "Ability.Nova");

	// Elements
	UE_DEFINE_GAMEPLAY_TAG(Element_Fire, "Element.Fire");
	UE_DEFINE_GAMEPLAY_TAG(Element_Ice, "Element.Ice");
	UE_DEFINE_GAMEPLAY_TAG(Element_Wind, "Element.Wind");
	UE_DEFINE_GAMEPLAY_TAG(Element_Earth, "Element.Earth");

	// Status
	UE_DEFINE_GAMEPLAY_TAG(Status_Burn, "Status.Burn");
	UE_DEFINE_GAMEPLAY_TAG(Status_Slow, "Status.Slow");
	UE_DEFINE_GAMEPLAY_TAG(Status_Freeze, "Status.Freeze");
	UE_DEFINE_GAMEPLAY_TAG(Status_Lift, "Status.Lift");
	UE_DEFINE_GAMEPLAY_TAG(Status_Lava, "Status.Lava");

	// States
	UE_DEFINE_GAMEPLAY_TAG(State_Invulnerable, "State.Invulnerable");
	UE_DEFINE_GAMEPLAY_TAG(State_Stunned, "State.Stunned");
	UE_DEFINE_GAMEPLAY_TAG(State_Dead, "State.Dead");
	UE_DEFINE_GAMEPLAY_TAG(State_Aiming, "State.Aiming");

	// Cooldown
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Blink, "Cooldown.Blink");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Barrier, "Cooldown.Barrier");
	UE_DEFINE_GAMEPLAY_TAG(Cooldown_Nova, "Cooldown.Nova");

	// Data
	UE_DEFINE_GAMEPLAY_TAG(Data_Damage, "Data.Damage");
}
