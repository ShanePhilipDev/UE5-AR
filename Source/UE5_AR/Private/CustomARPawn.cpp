// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomARPawn.h"
#include "Runtime/Engine/Classes/Kismet/KismetSystemLibrary.h"
#include "ARBlueprintLibrary.h"
#include "Camera/CameraComponent.h"
#include "CustomGameMode.h"
#include "Kismet/KismetMathLibrary.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"

// Sets default values
ACustomARPawn::ACustomARPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Setup root component.
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	SetRootComponent(SceneComponent);

	// Setup camera.
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SceneComponent);

}

// Called when the game starts or when spawned
void ACustomARPawn::BeginPlay()
{
	Super::BeginPlay();

}

// Called every frame
void ACustomARPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Get game mode.
	auto Temp = GetWorld()->GetAuthGameMode();
	auto GM = Cast<ACustomGameMode>(Temp);

	// If player is in grenade throw phase...
	if (GM->CurrentPhase == EGamePhase::TURN_GRENADE)
	{
		// Create a rotator based on direction between touches, and rotate the fighter using this.
		FVector Dir = UKismetMathLibrary::GetDirectionUnitVector(TouchStart, TouchEnd);
		Dir.Z = 0;
		FRotator Rot = Dir.Rotation();
		Rot.Add(0, 90, 0);
		GM->CurrentFighter->SetActorRotation(Rot);
	}
}

// Called to bind functionality to input
void ACustomARPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Bind inputs.
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ACustomARPawn::OnScreenTouch);
	PlayerInputComponent->BindTouch(IE_Repeat, this, &ACustomARPawn::OnScreenTouchHeld);
	PlayerInputComponent->BindTouch(IE_Released, this, &ACustomARPawn::OnScreenTouchReleased);
}

void ACustomARPawn::OnScreenTouch(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
{
	// Get game mode.
	auto Temp = GetWorld()->GetAuthGameMode();
	auto GM = Cast<ACustomGameMode>(Temp);

	// Start tracking touch.
	bIsScreenTouched = true;
	TouchStart = ScreenPos;


	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Screen pressed"));

	// Based on current game phase, perform different actions on touch.
	switch (GM->CurrentPhase)
	{
	case EGamePhase::PLANE_SETUP:
		// In plane setup phase, check for planes and switch to obstacle setup if plane is found.
		if (GM->LineTraceCheckForPlane(FVector2D(ScreenPos)))
		{
			GM->CurrentPhase = EGamePhase::OBSTACLE_SETUP;
		}
		break;
	case EGamePhase::PAWN_SETUP:
		// Spawn actors in pawn setup phase.
		GM->LineTraceSpawnActor(ScreenPos);
		break;
	case EGamePhase::OBSTACLE_SETUP:
		// Spawn obstacles in obstacle setup phase.
		GM->LineTraceSpawnObstacle(ScreenPos);
		break;
	case EGamePhase::TURN_SHOOT:
		// Select targets in shooting phase.
		GM->LineTraceSelectPawn(FVector2D(ScreenPos));
		break;
	case EGamePhase::TURN_MOVEMENT:
		// Select move position in movement phase.
		GM->LineTraceMovePawn(FVector2D(ScreenPos));
		break;
	default:
		break;
	}

	
}

void ACustomARPawn::OnScreenTouchHeld(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
{
	// Track last touch position.
	TouchEnd = ScreenPos;
}

void ACustomARPawn::OnScreenTouchReleased(const ETouchIndex::Type FingerIndex, const FVector ScreenPos)
{
	// Get game mode.
	auto Temp = GetWorld()->GetAuthGameMode();
	auto GM = Cast<ACustomGameMode>(Temp);

	// Finish tracking touch.
	bIsScreenTouched = false;
	TouchEnd = ScreenPos;

	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Screen press released"));

	// On first release after pressing grenade button, do nothing.
	if (bGrenadeButtonPressed)
	{
		bGrenadeButtonPressed = false;
	}
	else if (GM->CurrentPhase == EGamePhase::TURN_GRENADE) // If in the grenade phase...
	{
		// Get distance between
		float Dist = FVector::Distance(TouchStart, TouchEnd);

		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, FString::Printf(TEXT("Distance: %f"), Dist));

		// If drag is long enough, throw grenade.
		if (abs(Dist) > 50)
		{
			GM->CurrentFighter->ThrowGrenade(FVector(Dist));
			GM->CurrentPhase = EGamePhase::TURN_IDLE;
		}
	}
}
	
