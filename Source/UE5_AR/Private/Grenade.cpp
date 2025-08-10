// Fill out your copyright notice in the Description page of Project Settings.


#include "Grenade.h"
#include "Kismet/GameplayStatics.h"
#include "ARPlaneActor.h"
#include "ProceduralMeshComponent.h"
#include "CustomGameMode.h"
#include "FighterPawn.h"


// Sets default values
AGrenade::AGrenade()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Use constructor helpers to find the grenade mesh.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> GrenadeAssetMesh(TEXT("StaticMesh'/Game/Grenade/grenade.grenade'"));
	UStaticMesh* GrenadeAsset = GrenadeAssetMesh.Object;

	// Creating the static mesh and using the grenade's static mesh asset.
	GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	GrenadeMesh->SetStaticMesh(GrenadeAsset);
	
	// Use constructor helpers to find a cube for the ground mesh.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'"));
	Ground = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Ground"));

	// Ground mesh setup.
	Ground->SetStaticMesh(Cube.Object);
	Ground->AddLocalOffset(FVector(0.0f, 0.0f, -100.0f));
	Ground->SetRelativeScale3D(FVector(250.0f, 250.0f, 0.1f));
	Ground->SetVisibility(false);

	// Use constructor helpers to find the explosion particle system.
	static ConstructorHelpers::FObjectFinder<UParticleSystem> ExplosionParticle(TEXT("ParticleSystem'/Game/StarterContent/Particles/P_Explosion.P_Explosion'"));
	UParticleSystem* ExplosionTemplate = ExplosionParticle.Object;

	// Setup explosion particle system.
	Explosion = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Explosion"));
	Explosion->SetTemplate(ExplosionTemplate);
	Explosion->SetupAttachment(GrenadeMesh);
	Explosion->SetRelativeScale3D(FVector(40.0f));

	// Use constructor helpers to find the explosion sound cue.
	static ConstructorHelpers::FObjectFinder<USoundCue> ExplosionSFX(TEXT("SoundCue'/Game/Sounds/Explosion.Explosion'"));
	USoundCue* ExplosionCue = ExplosionSFX.Object;

	// Create the audio component and use the previously created sound cue.
	ExplosionSound = CreateDefaultSubobject<UAudioComponent>(TEXT("Explosion Sound"));
	ExplosionSound->SetSound(ExplosionCue);
	ExplosionSound->SetVolumeMultiplier(0);

	// Grenade uses physics to move.
	GrenadeMesh->SetSimulatePhysics(true);

	// Default values.
	ExplosionDelay = 1.5f;
	ExplosionRadius = 3000.f;
	Damage = 75;
}

// Called when the game starts or when spawned
void AGrenade::BeginPlay()
{
	Super::BeginPlay();
	
	// Explosion particle system is invisible until we want it to be seen.
	Explosion->SetVisibility(false);

	// Start the timer for the grenade explosion. Calls the explode function after x seconds.
	GetWorld()->GetTimerManager().SetTimer(ExplodeTimer, this, &AGrenade::Explode, ExplosionDelay, false);

	// Get the game mode.
	auto Temp = GetWorld()->GetAuthGameMode();
	auto GM = Cast<ACustomGameMode>(Temp);

	// Setup ground.
	if (GM)
	{
		FVector SpawnLocation = GM->CurrentFighter->GetMesh()->GetComponentLocation();
		Ground->SetWorldLocation(SpawnLocation);
	}
}

// Called every frame
void AGrenade::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Explode function. Handles particles, sounds and damage.
void AGrenade::Explode()
{
	// Start particle effect.
	Explosion->SetVisibility(true);
	Explosion->ResetParticles();
	Explosion->Activate();

	// Hide mesh as it has just exploded.
	GrenadeMesh->SetVisibility(false);

	// Play explosion sound.
	ExplosionSound->SetVolumeMultiplier(1);
	ExplosionSound->Play();

	// Variables for sphere overlap function. Only looking for pawns.
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECollisionChannel::ECC_Pawn));
	TArray<AActor*> ActorsToIgnore;
	TArray<AActor*> OutActors;

	// Grenade does damage within a sphere. If enemies are inside the sphere, they take damage.
	// This function gets the overlapping actors within the specified radius (explosion radius * scale).
	UKismetSystemLibrary::SphereOverlapActors(GetWorld(), GrenadeMesh->GetComponentLocation(), ExplosionRadius * GrenadeMesh->GetComponentScale().X, ObjectTypes, nullptr, ActorsToIgnore, OutActors);

	// Iterate through the found actors, then damage them if they're fighters. Does not check for enemies, allows for friendly fire.
	for (auto Actor : OutActors)
	{
		if (Actor)
		{
			auto Fighter = Cast<AFighterPawn>(Actor);

			if (Fighter)
			{
				Fighter->ReceiveDamage(Damage);
			}

		}
	}
}

