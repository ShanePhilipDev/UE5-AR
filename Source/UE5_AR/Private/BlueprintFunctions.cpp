// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueprintFunctions.h"

// Set sound class volume.
void UBlueprintFunctions::SetSoundClassVolume(USoundClass* SoundClass, float Volume)
{
	// If the sound class actually exists...
	if (SoundClass)
	{
		// Set the sound class' volume property to the input value.
		SoundClass->Properties.Volume = Volume;
	}
}

// Returns the sound class' volume.
float UBlueprintFunctions::GetSoundClassVolume(const USoundClass* SoundClass)
{
	return SoundClass->Properties.Volume;
}
