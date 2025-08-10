// Fill out your copyright notice in the Description page of Project Settings.


#include "Obstacle.h"
#include "ARPin.h"

// Sets default values
AObstacle::AObstacle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Default root component.
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// Create crate, attach it to the root.
	Crate = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Crate"));
	Crate->SetupAttachment(Root);

	// Set the crate's static mesh to be a cube, with help from constructor helpers.
	auto MeshAsset = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	Crate->SetStaticMesh(MeshAsset.Object);

	// Set the crate's material with help from constructor helpers.
	auto Material = ConstructorHelpers::FObjectFinder<UMaterial>(TEXT("Material'/Game/Crate_Mat.Crate_Mat'"));
	Crate->SetMaterial(0, Material.Object);

	// Set half height, scale and offset.
	HalfHeight = 50;
	Scale = 0.15f;
	Crate->AddLocalOffset(FVector(0, 0, HalfHeight));
}

// Called when the game starts or when spawned
void AObstacle::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void AObstacle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Keep the crate locked to the pin when it is tracking.
	if (PinComponent)
	{
		auto TrackingState = PinComponent->GetTrackingState();

		switch (TrackingState)
		{
		case EARTrackingState::Tracking:
			// Use the pin's transform.
			SetActorTransform(PinComponent->GetLocalToWorldTransform());

			// Set scale and rotation.
			SetActorScale3D(FVector(Scale));
			SetActorRotation(FRotator(0));
			break;

		case EARTrackingState::NotTracking:
			// If not tracking, remove pin.
			PinComponent = nullptr;
			break;

		}
	}

}

