// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GunComponent.h"
#include "Grenade.h"

#include "FighterPawn.generated.h"

class UARPin;

// Enum for tracking whether it is a fighter's turn or they are being targeted.
UENUM(BlueprintType)
enum class ESelectionState : uint8
{
	NONE		UMETA(DisplayName = "None"),
	TARGETED	UMETA(DisplayName = "Targeted"),
	SELECTED	UMETA(DisplayName = "Selected")
};

UCLASS()
class UE5_AR_API AFighterPawn : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFighterPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Update the fighter's indicator - displaying whether it is their turn or if they are being targeted.
	void UpdateIndicator();

	// Timer handles for resetting animations and throwing grenades.
	FTimerHandle AnimationResetTimer;
	FTimerHandle GrenadeReleaseTimer;

	// Function to reset variables used by animation blueprint.
	UFUNCTION()
	void ResetAnimations();

	// Size of the fighter.
	float Scale;

	// Variables for animation blueprint.
	// *** //
	UPROPERTY(BlueprintReadWrite)
		bool bIsHit;

	UPROPERTY(BlueprintReadWrite)
		bool bIsDead;

	UPROPERTY(BlueprintReadWrite)
		bool bIsFiring;

	UPROPERTY(BlueprintReadWrite)
		bool bIsThrowing;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		bool bIsMoving;
	// *** //

	// Whether the fighter has a grenade or not.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bHasGrenade;

	// Whether the fighter has shot their gun this turn.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bHasShot;

	// The target position for movement.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector TargetPos;

	// The fighter's offset from the pin's location.
	FVector Offset;

	// Player movement is limited to this distance per turn.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MovableDistance;

	// The distance moved in this turn.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceMoved;

	// The fighter's health.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Health;

	// The fighter's damage multiplier. This is reduced when there is an obstacle between the fighter and his target.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DamageMultiplier;

	// The fighter's hit chance. This is reduced based on distance and whether the target is obstructed.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float HitChance;

	// Tracks whether the fighter's target is obstructed.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bIsObstructed;

	// The target fighter for shooting.
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		AFighterPawn* TargetFighter;

	// Half the character's height, used for offsets.
	float HalfHeight;
	
	// The fighter's gun.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UGunComponent* Gun;

	// The fighter's grenade mesh. A child actor for the grenade would be awkward to use, so this mesh is used instead until the actual grenade is spawned.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* GrenadeMesh;

	// Dynamic material used for changing the fighter's colour.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstanceDynamic* MeshMaterial;

	// Dynamic material used for changing the indicator texture.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UMaterialInstanceDynamic* DynamicIndicatorMaterial;

	// A plane mesh for the indicator.
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* Indicator;

	// The indicator's material.
	UMaterial* IndicatorMaterial;

	// Fighter's selection state.
	ESelectionState Selection;

	// Guns min and max range for hit chance.
	float MinRange;
	float MaxRange;

	// Min and max damage for shooting.
	float MinDamage;
	float MaxDamage;
public:	
	// Pin component to keep the fighter in the same real world position.
	UARPin* PinComponent;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Getter for half height.
	float GetHalfHeight() { return HalfHeight; };
		 
	// Set the dynamic material colour.
	void SetColor(FColor Color);

	// *** Some functions are blueprint callable so they can be called from the UI widget, i.e. Shoot() is called when the shoot widget button is pressed. *** //

	// Setter for the selection state.
	UFUNCTION(BlueprintCallable)
	void SetSelectionState(ESelectionState S);

	// Returns the fighter's target.
	UFUNCTION(BlueprintCallable)
	AFighterPawn* GetTarget() { return TargetFighter; };

	// Start the targeting process.
	UFUNCTION(BlueprintCallable)
	void StartTargeting();

	// Select a target.
	void SelectTarget(AFighterPawn* Target);

	// End the targeting process.
	UFUNCTION(BlueprintCallable)
	void EndTargeting();

	// Shoot the current target.
	UFUNCTION(BlueprintCallable)
	void Shoot();

	// Throws a grenade in the specified direction.
	UFUNCTION(BlueprintCallable)
	void ThrowGrenade(FVector Dir);

	// Releases the grenade - called after timer to match animation.
	void ReleaseGrenade(FVector Dir);

	// Take damage.
	void ReceiveDamage(int Dmg);

	// Reset certain variables on turn start.
	void TurnReset();

	// Getter for the death status.
	bool GetIsDead() { return bIsDead; };

	// Function for setting move to location.
	void MoveTo(FVector Location);

	// Function to move the pawn.
	void Move(float DeltaTime);
};
