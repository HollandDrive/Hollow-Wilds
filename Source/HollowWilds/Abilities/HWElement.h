#pragma once

#include "CoreMinimal.h"
#include "HWElement.generated.h"

/** The four spell elements the witch channels through her wand. */
UENUM(BlueprintType)
enum class EHWElement : uint8
{
	Fire,
	Ice,
	Wind,
	Earth
};
