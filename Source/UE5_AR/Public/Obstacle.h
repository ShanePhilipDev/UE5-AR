// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Obstacle.generated.h"

class UARPin;

UCLASS()
class UE5_AR_API AObstacle : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AObstacle();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Root component.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent* Root;

	// Crate mesh.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* Crate;

	// Half the height of the crate. Used for offsetting the mesh.
	float HalfHeight;

	// The size of the crate.
	float Scale;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Pin component to keep the obstacle's position in the same real world position.
	UARPin* PinComponent;

	// Getter for the scale.
	float GetScale() { return Scale; };
};
