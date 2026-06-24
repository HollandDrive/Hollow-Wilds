// Copyright Kepler Interactive. All Rights Reserved.

#include "Core/HWGameMode.h"
#include "Core/HWPlayerState.h"
#include "Core/HWPlayerController.h"
#include "Character/HWWitchCharacter.h"

AHWGameMode::AHWGameMode()
{
	DefaultPawnClass = AHWWitchCharacter::StaticClass();
	PlayerControllerClass = AHWPlayerController::StaticClass();
	PlayerStateClass = AHWPlayerState::StaticClass();
}
