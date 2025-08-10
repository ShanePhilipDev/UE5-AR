// A class for providing C++ functions to blueprints.
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Sound/SoundClass.h"
#include "BlueprintFunctions.generated.h"

/**
 * 
 */
UCLASS()
class UE5_AR_API UBlueprintFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	// Set the volume of a sound class. Used in menus for adjusting volume of individual classes such as music and SFX.
	UFUNCTION(BlueprintCallable)
	static void SetSoundClassVolume(USoundClass* SoundClass, float Volume);

	// Get the volume of a sound class.
	UFUNCTION(BlueprintPure)
	static float GetSoundClassVolume(const USoundClass* SoundClass);
};
