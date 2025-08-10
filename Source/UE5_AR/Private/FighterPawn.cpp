// Fill out your copyright notice in the Description page of Project Settings.


#include "FighterPawn.h"
#include "ARPin.h"
#include "ARBlueprintLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/CapsuleComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"






// Sets default values
AFighterPawn::AFighterPawn()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Default values.
	HalfHeight = 88;
	Health = 100.0f;
	DamageMultiplier = 1.0f;
	bHasShot = false;
	Selection = ESelectionState::NONE;
	bHasGrenade = true;
	GetCharacterMovement()->MaxWalkSpeed = 60;
	Scale = 0.1;
	MovableDistance = 100;
	DistanceMoved = 0;
	Offset = FVector(0);
	MinRange = 30;
	MaxRange = 300;
	MinDamage = 25;
	MaxDamage = 35;

	// Setup player's skeletal mesh and animation class using constructor helpers.
	static ConstructorHelpers::FObjectFinder<USkeletalMesh> Skeleton(TEXT("SkeletalMesh'/Game/AnimStarterPack/UE4_Mannequin/Mesh/SK_Mannequin.SK_Mannequin'"));
	static ConstructorHelpers::FObjectFinder<UClass> Animation(TEXT("AnimBlueprint'/Game/AnimStarterPack/FighterAnimBlueprint.FighterAnimBlueprint_C'"));
	GetMesh()->SetSkeletalMesh(Skeleton.Object);
	GetMesh()->SetAnimClass(Animation.Object);

	// Setup the indicator using a plane from constructor helpers.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Plane(TEXT("StaticMesh'/Engine/BasicShapes/Plane.Plane'"));
	Indicator = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Indicator"));
	Indicator->SetupAttachment(GetRootComponent());
	Indicator->SetStaticMesh(Plane.Object);
	Indicator->AddLocalOffset(FVector(0.0f, 0.0f, 135.0f + HalfHeight));
	Indicator->SetRelativeScale3D(FVector(0.5f));

	// Setup indicator material.
	static ConstructorHelpers::FObjectFinder<UMaterial> Material(TEXT("Material'/Game/IndicatorMat.IndicatorMat'"));
	IndicatorMaterial = Material.Object;

	// Adjust mesh's offset.
	GetMesh()->AddLocalRotation(FRotator(0, -90, 0));

	// Setup gun and attach to right hand.
	Gun = CreateDefaultSubobject<UGunComponent>(TEXT("Gun Component"));
	Gun->SetupAttachment(GetMesh(), TEXT("RightHandSocket"));
	Gun->AddLocalOffset(FVector(-23.0f, 5.0f, 1.0f));
	Gun->AddLocalRotation(FRotator(-182.0f, -2.0f, 180.0f));
	Gun->SetRelativeScale3D(FVector(0.2f));

	// Setup grenade and attach to left hand. When throwing the grenade, this mesh is hidden and a grenade actor is spawned.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> GrenadeAssetMesh(TEXT("StaticMesh'/Game/Grenade/grenade.grenade'"));
	UStaticMesh* GrenadeAsset = GrenadeAssetMesh.Object;
	GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	GrenadeMesh->SetupAttachment(GetMesh(), TEXT("LeftHandSocket"));
	GrenadeMesh->SetStaticMesh(GrenadeAsset);
	GrenadeMesh->SetRelativeScale3D(FVector(0.05f));
	GrenadeMesh->AddLocalOffset(FVector(0.0f, -4.0f, 0.0f));
	GrenadeMesh->AddLocalRotation(FRotator(-180.0f, 0.0f, 180.0f));
	GrenadeMesh->SetVisibility(false);

	
}

// Called when the game starts or when spawned
void AFighterPawn::BeginPlay()
{
	Super::BeginPlay();

	// Setup dynamic materials.
	// *** //
	auto Material = GetMesh()->GetMaterial(0);

	MeshMaterial = UMaterialInstanceDynamic::Create(Material, this);
	GetMesh()->SetMaterial(0, MeshMaterial);
	MeshMaterial->SetVectorParameterValue("BodyColor", FColor::White);
	
	DynamicIndicatorMaterial = UMaterialInstanceDynamic::Create(IndicatorMaterial, this);
	Indicator->SetMaterial(0, DynamicIndicatorMaterial);
	Indicator->SetVisibility(false);
	// *** //
}

void AFighterPawn::UpdateIndicator()
{
	// Make indicator always face the camera.
	FVector DirToCamera = UKismetMathLibrary::GetDirectionUnitVector(Indicator->GetComponentLocation(), UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation());
	FVector UpVector = UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->GetActorUpVector();
	FVector CrossProduct = UKismetMathLibrary::Cross_VectorVector(UpVector, DirToCamera);
	FRotator IndicatorRotation = UKismetMathLibrary::MakeRotationFromAxes(DirToCamera, CrossProduct, UpVector);
	Indicator->SetWorldRotation(IndicatorRotation);
	Indicator->AddLocalRotation(FRotator(0.0f, -90.0f, 90.0f));
}

// Reset to default values.
void AFighterPawn::ResetAnimations()
{
	bIsHit = false;
	bIsFiring = false;
	bIsThrowing = false;
}

// Called every frame
void AFighterPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// If pin exists...
	if (PinComponent)
	{
		auto TrackingState = PinComponent->GetTrackingState();

		switch (TrackingState)
		{
			// Update location based on pin location and offset.
		case EARTrackingState::Tracking:
			SetActorLocation(PinComponent->GetLocalToWorldTransform().GetLocation() + Offset);
			SetActorScale3D(FVector(Scale));
			break;

		case EARTrackingState::NotTracking:
			PinComponent = nullptr;

			break;

		}
	}

	// Only update indicator when selected or targeted.
	switch (Selection)
	{
	case ESelectionState::NONE:
		
		break;
	case ESelectionState::SELECTED:
		UpdateIndicator();
		break;
	case ESelectionState::TARGETED:
		UpdateIndicator();
		break;
	default:
		break;
	}

	// Move if character should be moving.
	if (bIsMoving)
	{
		Move(DeltaTime);
	}
}

// Called to bind functionality to input
void AFighterPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

// Set dynamic material's colour parameter.
void AFighterPawn::SetColor(FColor Color)
{
	MeshMaterial->SetVectorParameterValue("BodyColor", Color);
}

// Change selection state.
void AFighterPawn::SetSelectionState(ESelectionState S)
{
	// Set selection state.
	Selection = S;

	// Set indicator visibility based on selection state. Also set indicator material's parameter. 0 and 1 change the texture of the material.
	switch (Selection)
	{
	case ESelectionState::NONE:
		Indicator->SetVisibility(false);
		break;
	case ESelectionState::SELECTED:
		Indicator->SetVisibility(true);
		DynamicIndicatorMaterial->SetScalarParameterValue("IndicatorValue", 1);
		break;
	case ESelectionState::TARGETED:
		Indicator->SetVisibility(true);
		DynamicIndicatorMaterial->SetScalarParameterValue("IndicatorValue", 0);
		break;
	default:
		break;
	}
}

// Reset values when start targeting.
void AFighterPawn::StartTargeting()
{
	DamageMultiplier = 1.0f;
	bIsObstructed = false;
	HitChance = 1;
}

// Target the specified fighter.
void AFighterPawn::SelectTarget(AFighterPawn* Target)
{
	// If target is valid...
	if (Target)
	{
		StartTargeting();

		// Face target.
		SetActorRotation(UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target->GetActorLocation()));

		// Set target, and update their indicator.
		TargetFighter = Target;
		TargetFighter->SetSelectionState(ESelectionState::TARGETED);

		// Set hit chance based on distance to target.
		float Distance = GetDistanceTo(Target);
		HitChance = UKismetMathLibrary::MapRangeClamped(Distance, MinRange, MaxRange, 1, 0);

		// Ignore fighters when checking for obstacles. Could be changed in future to allow for collaterals.
		TArray<AActor*> IgnoredActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFighterPawn::StaticClass(), IgnoredActors); 
		FCollisionQueryParams CollisionParameters;
		CollisionParameters.AddIgnoredActors(IgnoredActors);

		// Draw line trace.
		const FName TraceTag("TraceTag");
		GetWorld()->DebugDrawTraceTag = TraceTag;
		CollisionParameters.TraceTag = TraceTag;

		// Trace between centre of fighter and its target.
		FHitResult Hit;
		FVector TraceStart = GetMesh()->GetSocketByName(FName("Centre"))->GetSocketLocation(GetMesh());
		FVector TraceEnd = Target->GetMesh()->GetSocketByName(FName("Centre"))->GetSocketLocation(Target->GetMesh());
		bIsObstructed = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECollisionChannel::ECC_WorldDynamic, CollisionParameters);

		// If obstacle was found, reduce damage multiplier and hit chance by 0.5. Hit chance cannot drop below 0.
		if (bIsObstructed)
		{
			DamageMultiplier -= 0.5;
			HitChance -= 0.5;
			if (HitChance < 0)
			{
				HitChance = 0;
			}
		}
	}
}

// Reset target properties.
void AFighterPawn::EndTargeting()
{
	if (TargetFighter)
	{
		TargetFighter->SetSelectionState(ESelectionState::NONE);
		TargetFighter = NULL;
	}
}

// Shoots the target.
void AFighterPawn::Shoot()
{
	// Can only shoot if there is a target.
	if (TargetFighter)
	{
		// Generate random number.
		float Rand = FMath::RandRange(0.0f, 1.0f);

		// If random number is less than hit chance, the shot hits.
		if (Rand < HitChance)
		{
			// Randomise damage, apply multiplier, then apply damage to target.
			float Damage = FMath::RandRange(MinDamage, MaxDamage);
			Damage *= DamageMultiplier;
			TargetFighter->ReceiveDamage(Damage);
		}

		// Set IsFiring to true so animation blueprint starts animating.
		bIsFiring = true;
		GetWorld()->GetTimerManager().SetTimer(AnimationResetTimer, this, &AFighterPawn::ResetAnimations, 0.1f, false);

		// Has shot, so can't shoot anymore this turn.
		bHasShot = true;

		// Gun sound and particles.
		Gun->Fire();
	}
}

// Start throwing grenade process.
void AFighterPawn::ThrowGrenade(FVector Dir)
{
	if (bHasGrenade)
	{
		// Grenade's one use is done.
		bHasGrenade = false;

		// Show grenade mesh.
		GrenadeMesh->SetVisibility(true);

		// Start animation.
		bIsThrowing = true;
		GetWorld()->GetTimerManager().SetTimer(AnimationResetTimer, this, &AFighterPawn::ResetAnimations, 0.1f, false);

		// Timer to release the grenade partway through the animation.
		FTimerDelegate ReleaseDelegate = FTimerDelegate::CreateUObject(this, &AFighterPawn::ReleaseGrenade, Dir);
		GetWorld()->GetTimerManager().SetTimer(GrenadeReleaseTimer, ReleaseDelegate, 0.82f, false);
	}
}

// Release grenade - spawns the grenade actor.
void AFighterPawn::ReleaseGrenade(FVector Dir)
{
	// Hide the grenade mesh.
	GrenadeMesh->SetVisibility(false);

	// Spawn the grenade with the grenade mesh's position, rotation and scale.
	const FActorSpawnParameters SpawnInfo;
	auto Grenade = GetWorld()->SpawnActor<AGrenade>(GrenadeMesh->GetComponentLocation(), GrenadeMesh->GetComponentRotation(), SpawnInfo);
	Grenade->SetActorScale3D(GrenadeMesh->GetComponentScale());

	// Add force to the grenade forward and up.
	float ThrowPower = 0.2f;
	FVector ThrowDir = Dir.Length() * GrenadeMesh->GetComponentScale() * ThrowPower * GetActorForwardVector();
	Grenade->GetMesh()->AddImpulse(ThrowDir);
	Grenade->GetMesh()->AddImpulse(GetActorUpVector() * Dir.Length() * GrenadeMesh->GetComponentScale() * ThrowPower);


}

// Take damage.
void AFighterPawn::ReceiveDamage(int Dmg)
{
	// Start animation in anim bp.
	bIsHit = true;
	GetWorld()->GetTimerManager().SetTimer(AnimationResetTimer, this, &AFighterPawn::ResetAnimations, 0.1f, false);

	// Reduce health.
	Health -= Dmg;

	// If health drops below zero, they are dead.
	if (Health <= 0)
	{
		Health = 0;
		bIsDead = true;
		SetSelectionState(ESelectionState::NONE);
	}
}

// Give fighter a shot and reset distance moved..
void AFighterPawn::TurnReset()
{
	bHasShot = false;
	DistanceMoved = 0.f;
}

// Start moving and set target location.
void AFighterPawn::MoveTo(FVector Location)
{
	bIsMoving = true;
	TargetPos = Location;
}

void AFighterPawn::Move(float DeltaTime)
{
	// Start and end positions.
	// *** //
	FVector StartPos = FVector(GetActorLocation());
	StartPos.Z = 0;

	FVector EndPos = TargetPos;
	EndPos.Z = 0;
	// *** //

	// If the pawn has exceeded movement quota, stop moving.
	if (DistanceMoved > MovableDistance)
	{
		bIsMoving = false;
	}
	else if (StartPos.Equals(EndPos, 1.0f)) // If pawn has reached end position, stop moving.
	{
		bIsMoving = false;
	}
	else
	{
		// Rotate to face target position.
		FRotator Rot = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetPos);
		Rot.Pitch = 0;
		SetActorRotation(Rot);

		// The amount to move forward in this frame.
		FVector Movement = GetActorForwardVector() * GetCharacterMovement()->MaxWalkSpeed * DeltaTime;

		// Trace forward from the pawn.
		FHitResult Hit;
		FVector TraceStart = GetMesh()->GetSocketByName(FName("Centre"))->GetSocketLocation(GetMesh());
		FVector TraceEnd = TraceStart + Movement * 5;
		FCollisionQueryParams Params;
		Params.AddIgnoredComponent(GetMesh());
		Params.AddIgnoredComponent(GetCapsuleComponent());
		Params.AddIgnoredComponent(Cast<UPrimitiveComponent>(Gun));
		bool TraceSuccess = GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECollisionChannel::ECC_WorldDynamic, Params);

		// If trace hits something, there is something in the way. Stop moving.
		if (TraceSuccess)
		{
			bIsMoving = false;
		}
		else // Otherwise increase offset and tracked distance moved.
		{
			Offset += Movement;
			DistanceMoved += Movement.Length();
		}

	}
}

