// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "GameFramework/Actor.h"
#include "Obstacle.h"
#include "HelloARManager.generated.h"


class UARSessionConfig;
class AARPlaneActor;
class UARPlaneGeometry;

UCLASS()
class UE5_AR_API AHelloARManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AHelloARManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Create a default Scene Component
	UPROPERTY(Category = "SceneComp", VisibleAnywhere, BlueprintReadWrite)
		USceneComponent* SceneComponent;

	// Boolean for tracking whether a plane has been selected to be used for the arena.
	bool bPlaneSelected;

	// Van Gogh picture is used for adding obstacles, this tracks where he has been found.
	bool bGoghFound;

	// The obstacle to be placed on the image.
	AObstacle* ImageTrackedObstacle;

	// Assign the plane to be used for the arena.
	void SetUsedPlane(UARPlaneGeometry* Plane);

	// Reset plane data.
	void ResetARCoreSession();
protected:
	
	// Updates the plane actors on every frame as long as the AR Session is running
	void UpdatePlaneActors();

	// Updates image tracked obstacle - spawning and moving.
	void UpdateImageTracking();

	// Spawns a plane.
	AARPlaneActor* SpawnPlaneActor();

	// Gets specified plane's colour.
	FColor GetPlaneColor(int Index);
	
	// Configuration file for AR Session
	UARSessionConfig* Config;

	//Base plane actor for geometry detection
	AARPlaneActor* PlaneActor;

	//Map of geometry planes
	TMap<UARPlaneGeometry*, AARPlaneActor*> PlaneActors;

	//Index for plane colours and array of colours
	int PlaneIndex = 0;
	TArray<FColor> PlaneColors;

};
