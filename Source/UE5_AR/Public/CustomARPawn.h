// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "CustomARPawn.generated.h"

class UCameraComponent;

UCLASS()
class UE5_AR_API ACustomARPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACustomARPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Touch input.
	virtual void OnScreenTouch(const ETouchIndex::Type FingerIndex, const FVector ScreenPos);
	virtual void OnScreenTouchHeld(const ETouchIndex::Type FingerIndex, const FVector ScreenPos);
	virtual void OnScreenTouchReleased(const ETouchIndex::Type FingerIndex, const FVector ScreenPos);

	// Track touches.
	bool bIsScreenTouched;
	FVector TouchEnd;
	FVector TouchStart;

	// Tracks grenade button presses, prevents grenades from being thrown when pressing the grenade button.
	UPROPERTY(BlueprintReadWrite)
	bool bGrenadeButtonPressed;
	

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Scene component.
	UPROPERTY(Category = "myCategory", VisibleAnywhere, BlueprintReadWrite)
	USceneComponent* SceneComponent;

	// Camera.
	UPROPERTY(Category = "myCategory", VisibleAnywhere, BlueprintReadWrite)
	UCameraComponent* CameraComponent;

	// Getters for screen touch status and position.
	bool GetScreenTouched() { return bIsScreenTouched; };
	FVector GetTouchPosition() { return TouchEnd; };
};
