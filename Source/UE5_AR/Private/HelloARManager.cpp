// Fill out your copyright notice in the Description page of Project Settings.


#include "HelloARManager.h"
#include "ARPlaneActor.h"
#include "ARPin.h"
#include "ARSessionConfig.h"
#include "ARBlueprintLibrary.h"
#include "CustomGameMode.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

// Sets default values
AHelloARManager::AHelloARManager()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// This constructor helper is useful for a quick reference within UnrealEngine and if you're working alone. But if you're working in a team, this could be messy.
	// If the artist chooses to change the location of an art asset, this will throw errors and break the game.
	// Instead let unreal deal with this. Usually avoid this method of referencing.
	// For long term games, create a Data-Only blueprint (A blueprint without any script in it) and set references to the object using the blueprint editor.
	// This way, unreal will notify your artist if the asset is being used and what can be used to replace it.
	static ConstructorHelpers::FObjectFinder<UARSessionConfig> ConfigAsset(TEXT("ARSessionConfig'/Game/Blueprints/HelloARSessionConfig.HelloARSessionConfig'"));
	Config = ConfigAsset.Object;

	//Populate the plane colours array
	PlaneColors.Add(FColor::Blue);
	PlaneColors.Add(FColor::Red);
	PlaneColors.Add(FColor::Green);
	PlaneColors.Add(FColor::Cyan);
	PlaneColors.Add(FColor::Magenta);
	PlaneColors.Add(FColor::Emerald);
	PlaneColors.Add(FColor::Orange);
	PlaneColors.Add(FColor::Purple);
	PlaneColors.Add(FColor::Turquoise);
	PlaneColors.Add(FColor::White);
	PlaneColors.Add(FColor::Yellow);

	// Image has not been found by default.
	bGoghFound = false;
}

// Called when the game starts or when spawned
void AHelloARManager::BeginPlay()
{
	Super::BeginPlay();

	//Start the AR Session
	UARBlueprintLibrary::StartARSession(Config);

	
}

// Called every frame
void AHelloARManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Switch based on AR session status...
	switch (UARBlueprintLibrary::GetARSessionStatus().Status)
	{
		// If AR is running, update the planes and track images.
	case EARSessionStatus::Running:
		UpdatePlaneActors();
		UpdateImageTracking();
		break;

		// If something goes wrong, reset and restart.
	case EARSessionStatus::FatalError:
		ResetARCoreSession();
		UARBlueprintLibrary::StartARSession(Config);
		break;
	}

	
}



//Updates the geometry actors in the world
void AHelloARManager::UpdatePlaneActors()
{
	// Get all world geometries and store in an array.
	auto Geometries = UARBlueprintLibrary::GetAllGeometriesByClass<UARPlaneGeometry>();
	bool bFound = false;

	// Loop through all geometries
	for (auto& It : Geometries)
	{
		// Check if current plane exists 
		if (PlaneActors.Contains(It))
		{
			AARPlaneActor* CurrentPActor = *PlaneActors.Find(It);

			// Check if plane is subsumed
			if (It->GetSubsumedBy()->IsValidLowLevel())
			{
				CurrentPActor->Destroy();
				PlaneActors.Remove(It);
				break;
			}
			else
			{
				// Get tracking state switch
				switch (It->GetTrackingState())
				{
					// If tracking update
				case EARTrackingState::Tracking:
					CurrentPActor->UpdatePlanePolygonMesh();
					break;
					// If not tracking destroy the actor and remove from map of actors
				case EARTrackingState::StoppedTracking:
					CurrentPActor->Destroy();
					PlaneActors.Remove(It);
					break;
				}
			}
		}
		else
		{
			// Get tracking state switch
			switch (It->GetTrackingState())
			{

			case EARTrackingState::Tracking:
				if (!It->GetSubsumedBy()->IsValidLowLevel())
				{
					// Spawn new planes, only when a plane has not been selected yet.
					if (bPlaneSelected == false)
					{
						PlaneActor = SpawnPlaneActor();
						PlaneActor->SetColor(GetPlaneColor(PlaneIndex));
						PlaneActor->ARCorePlaneObject = It;

						PlaneActors.Add(It, PlaneActor);
						PlaneActor->UpdatePlanePolygonMesh();
						PlaneIndex++;
					}
				}
				break;
			}
		}

	}
}

void AHelloARManager::UpdateImageTracking()
{
	// Get tracked images.
	TArray<UARTrackedImage*> TrackedImages = UARBlueprintLibrary::GetAllGeometriesByClass<UARTrackedImage>();

	for (auto TrackedImage : TrackedImages)
	{
		// Get detected image.
		if (TrackedImage->GetDetectedImage())
		{
			// If the image is Van Gogh...
			if (TrackedImage->GetDetectedImage()->GetFriendlyName().Equals("VanGogh"))
			{
				// If he hasn't already been found, spawn the obstacle at his position.
				if (!bGoghFound)
				{
					bGoghFound = true;
					auto Transform = TrackedImage->GetLocalToWorldTransform();
					const FActorSpawnParameters SpawnInfo;
					const FVector Loc = Transform.GetLocation();
					const FRotator Rot = Transform.GetRotation().Rotator();
					ImageTrackedObstacle = GetWorld()->SpawnActor<AObstacle>(Loc, Rot, SpawnInfo);
					ImageTrackedObstacle->SetActorScale3D(FVector(ImageTrackedObstacle->GetScale()));
				}
				else // Otherwise, move the already spawned obstacle to the image's position.
				{
					auto Transform = TrackedImage->GetLocalToWorldTransform();
					ImageTrackedObstacle->SetActorTransform(Transform);
					ImageTrackedObstacle->SetActorScale3D(FVector(ImageTrackedObstacle->GetScale()));
				}
			}
		}
	}
}

// Simple spawn function for the tracked AR planes
AARPlaneActor* AHelloARManager::SpawnPlaneActor()
{
	const FActorSpawnParameters SpawnInfo;
	const FRotator MyRot(0, 0, 0);
	const FVector MyLoc(0, 0, 0);

	AARPlaneActor* CustomPlane = GetWorld()->SpawnActor<AARPlaneActor>(MyLoc, MyRot, SpawnInfo);

	return CustomPlane;
}

// Gets the colour to set the plane to when its spawned
FColor AHelloARManager::GetPlaneColor(int Index)
{
	return PlaneColors[Index % PlaneColors.Num()];
}

void AHelloARManager::ResetARCoreSession()
{

	// Get all actors in the level and destroy them as well as emptying the respective arrays and resetting plane selection.
	TArray<AActor*> Planes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AARPlaneActor::StaticClass(), Planes);

	for ( auto& It : Planes)
		It->Destroy();
	
	Planes.Empty();
	PlaneActors.Empty();
	bPlaneSelected = false;
}

// Sets the used plane for the arena and deletes the others.
void AHelloARManager::SetUsedPlane(UARPlaneGeometry* Plane)
{
	// Set plane selected to true.
	bPlaneSelected = true;

	// Iterate through the planes, and delete any that aren't the selected one.
	for (auto P : PlaneActors)
	{
		if (P.Key != Plane)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Plane deleted"));
			P.Value->Destroy();
			PlaneActors.Remove(P.Key);
		}
	}
}
