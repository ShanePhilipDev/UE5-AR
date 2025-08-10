// Fill out your copyright notice in the Description page of Project Settings.


#include "GunComponent.h"

// Sets default values for this component's properties
UGunComponent::UGunComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;


	// Use constructor helper to find the gun's static mesh asset.
	static ConstructorHelpers::FObjectFinder<UStaticMesh> GunAssetMesh(TEXT("StaticMesh'/Game/Gun/M16A1.M16A1'"));
	UStaticMesh* GunAsset = GunAssetMesh.Object;

	// Create a static mesh component for the gun, use the mesh asset that was found and attach it to this object.
	GunMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Gun Mesh"));
	GunMesh->SetStaticMesh(GunAsset);
	GunMesh->SetupAttachment(this);
	GunMesh->SetCollisionProfileName(FName("NoCollision"));

	// Use constructor helper to find the gun's muzzle flash particle system asset.
	static ConstructorHelpers::FObjectFinder<UParticleSystem> MuzzleParticle(TEXT("/Game/VFX/P_AssaultRifle_MF"));
	UParticleSystem* MuzzleTemplate = MuzzleParticle.Object;

	// Create a particle system component for the gun's muzzle flash using the muzzle flash particle system that was found.
	// This particle system is attached to the end of the gun's barrel, and is set to be invisible by default. It is set to be visible when firing.
	MuzzleFlash = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Muzzle Flash"));
	MuzzleFlash->SetTemplate(MuzzleTemplate);
	MuzzleFlash->SetupAttachment(GunMesh);
	MuzzleFlash->AddLocalOffset(FVector(322, 0, 24));
	MuzzleFlash->SetVisibility(false);

	// Setup a gunshot sound cue using a constructor helper.
	static ConstructorHelpers::FObjectFinder<USoundCue> Gunshot(TEXT("SoundCue'/Game/Sounds/Gunshot.Gunshot'"));
	USoundCue* GunshotCue = Gunshot.Object;

	// Create the audio component and use the previously created sound cue.
	GunshotSound = CreateDefaultSubobject<UAudioComponent>(TEXT("Gunshot Sound"));
	GunshotSound->SetSound(GunshotCue);
	GunshotSound->SetVolumeMultiplier(0);

	// ...
}


// Called when the game starts
void UGunComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGunComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Muzzle flash is deactivated on first tick after calling it - we only want it to display for a split second anyway.
	if (MuzzleFlash->IsActive())
	{
		MuzzleFlash->SetActive(false);
		MuzzleFlash->SetVisibility(false);
	}
}

void UGunComponent::Fire()
{
	// Play the gunshot sound.
	GunshotSound->SetVolumeMultiplier(1);
	GunshotSound->Play();

	// Activate the muzzle flash.
	MuzzleFlash->Activate();
	MuzzleFlash->SetVisibility(true);
}

