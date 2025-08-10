// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundCue.h"
#include "GunComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UE5_AR_API UGunComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGunComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Static mesh component for the gun's mesh - the actual model of the gun.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* GunMesh;

	// Audio component. The sound to be played when the gun is fired.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAudioComponent* GunshotSound;

	// Particle system component. This is placed at the end of the barrel and is activated when the gun is fired.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UParticleSystemComponent* MuzzleFlash;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Function to play sound and start particle effect.
	void Fire();
		
};
