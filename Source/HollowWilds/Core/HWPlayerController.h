// Copyright Kepler Interactive. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "HWPlayerController.generated.h"

class UInputMappingContext;

/**
 * Player controller for Hollow Wilds.
 * Minimal for now; room to add Enhanced Input mapping context setup later.
 */
UCLASS()
class HOLLOWWILDS_API AHWPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	/** Default Enhanced Input mapping context to push for this controller. */
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
};
