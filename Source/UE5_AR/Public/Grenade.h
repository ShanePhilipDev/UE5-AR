// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Particles/ParticleSystemComponent.h"

#include "Grenade.generated.h"

UCLASS()
class UE5_AR_API AGrenade : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AGrenade();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// The mesh of the grenade.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* GrenadeMesh;

	// The sound that the grenade plays when it explodes.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UAudioComponent* ExplosionSound;

	// The particle system for the explosion.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UParticleSystemComponent* Explosion;

	// The ground for the grenade to roll on.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		UStaticMeshComponent* Ground;

	// A timer for handling the delay between the grenade spawning and blowing up.
	FTimerHandle ExplodeTimer;

	// How long the grenade takes to explode.
	float ExplosionDelay;

	// How big the explosion is.
	float ExplosionRadius;

	// How much damage the explosion does.
	float Damage;

	// Function to blow up the grenade.
	void Explode();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Function to get the grenade's mesh.
	UStaticMeshComponent* GetMesh() { return GrenadeMesh; };
};
