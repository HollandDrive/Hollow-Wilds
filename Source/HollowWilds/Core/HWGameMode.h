// Copyright Kepler Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "HWGameMode.generated.h"

/**
 * Default game mode for Hollow Wilds.
 * Wires up the HW pawn, player controller and player state classes.
 */
UCLASS()
class HOLLOWWILDS_API AHWGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AHWGameMode();
};
