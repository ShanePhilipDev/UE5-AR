// Fill out your copyright notice in the Description page of Project Settings.


#include "CustomGameMode.h"
#include "CustomARPawn.h"
#include "CustomGameState.h"
#include "ARPlaneActor.h"
#include "HelloARManager.h"
#include "ARPin.h"
#include "ARBlueprintLibrary.h"
#include "Runtime/Engine/Classes/Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ACustomGameMode::ACustomGameMode()
{
	// Add this line to your code if you wish to use the Tick() function
	PrimaryActorTick.bCanEverTick = true;

	// Set the default pawn and gamestate
	DefaultPawnClass = ACustomARPawn::StaticClass();
	GameStateClass = ACustomGameState::StaticClass();
	
	// Set default values.
	CurrentPhase = EGamePhase::MENU;
	bIsRedTurn = true;
	bHasRedWon = false;
	bHasBlueWon = false;
	RedTurnCounter = 0;
	BlueTurnCounter = 0;
	PawnsPerTeam = 3;
	ObstacleLimit = 3;

	// Create menu widget.
	ConstructorHelpers::FClassFinder<UUserWidget> MenuWidgetClass(TEXT("WidgetBlueprint'/Game/MenuWidget.MenuWidget_C'"));
	MenuWidget = CreateWidget(GetWorld(), MenuWidgetClass.Class);

	// Create game widget.
	ConstructorHelpers::FClassFinder<UUserWidget> GameWidgetClass(TEXT("WidgetBlueprint'/Game/GameWidget.GameWidget_C'"));
	GameWidget = CreateWidget(GetWorld(), GameWidgetClass.Class);
}


void ACustomGameMode::StartPlay() 
{
	SpawnInitialActors();

	// This is called before BeginPlay
	StartPlayEvent();

	// This function will transcend to call BeginPlay on all the actors 
	Super::StartPlay();
	
	// Add menu to the viewport.
	if (MenuWidget)
	{
		MenuWidget->AddToViewport();
	}
}

// An implementation of the StartPlayEvent which can be triggered by calling StartPlayEvent() 
void ACustomGameMode::StartPlayEvent_Implementation() 
{
	
}

void ACustomGameMode::StartGame()
{
	// On game start, remove menu from viewport and add game widget to viewport.
	MenuWidget->RemoveFromViewport();

	if (GameWidget)
	{
		GameWidget->AddToViewport();
	}

	// Red goes first.
	bIsRedTurn = true;

	// Select plane phase.
	CurrentPhase = EGamePhase::PLANE_SETUP;
}

void ACustomGameMode::StartTurn()
{
	// Red turn.
	if (bIsRedTurn)
	{
		// Set selected pawn.
		CurrentFighter = RedTeamActors[RedTurnCounter];

		// If selected pawn is dead, select a new pawn.
		int LoopNo = 0;
		while (CurrentFighter->GetIsDead() || LoopNo < PawnsPerTeam) // just in case to avoid infinite loop, exit after some iterations
		{
			RedTurnCounter++;
			if (RedTurnCounter > (PawnsPerTeam - 1))
			{
				RedTurnCounter = 0;
			}
			CurrentFighter = RedTeamActors[RedTurnCounter];
			LoopNo++;
		}
	}
	else // Blue turn.
	{
		// Set selected pawn.
		CurrentFighter = BlueTeamActors[BlueTurnCounter];
		
		// If selected pawn is dead, select a new pawn.
		int LoopNo = 0;
		while (CurrentFighter->GetIsDead() || LoopNo < PawnsPerTeam) // just in case to avoid infinite loop, exit after some iterations
		{
			BlueTurnCounter++;
			if (BlueTurnCounter > (PawnsPerTeam - 1))
			{
				BlueTurnCounter = 0;
			}
			CurrentFighter = BlueTeamActors[BlueTurnCounter];
			LoopNo++;
		}

	}

	// Prepare fighter for the turn.
	CurrentFighter->SetSelectionState(ESelectionState::SELECTED);
	CurrentFighter->TurnReset();
}

void ACustomGameMode::EndTurn()
{
	// Deselect fighter.
	CurrentFighter->SetSelectionState(ESelectionState::NONE);

	// Increase counter for relevant team, change to next team for the next turn.
	if (bIsRedTurn)
	{
		RedTurnCounter++;
		if (RedTurnCounter > (PawnsPerTeam - 1)) // -1 because start counting at 0
		{
			RedTurnCounter = 0;
		}
		bIsRedTurn = false;
	}
	else
	{
		BlueTurnCounter++;
		if (BlueTurnCounter > (PawnsPerTeam - 1))
		{
			BlueTurnCounter = 0;
		}
		bIsRedTurn = true;
	}

	// Start new turn.
	StartTurn();
}

void ACustomGameMode::ReturnToMenu()
{
	// Remove menu from viewport.
	MenuWidget->AddToViewport();

	// Add game widget to viewport.
	GameWidget->RemoveFromViewport();

	// Enter menu phase.
	CurrentPhase = EGamePhase::MENU;

	// Reset everything.
	Reset();
}

void ACustomGameMode::Reset()
{
	// Return to default values.
	bIsRedTurn = true;
	bHasRedWon = false;
	bHasBlueWon = false;
	RedTurnCounter = 0;
	BlueTurnCounter = 0;

	// Destroy actors, and empty arrays.
	// *** //
	for (auto Actor : RedTeamActors)
	{
		Actor->Destroy();
	}

	for (auto Actor : BlueTeamActors)
	{
		Actor->Destroy();
	}

	for (auto Obstacle : Obstacles)
	{
		Obstacle->Destroy();
	}

	RedTeamActors.Empty();
	BlueTeamActors.Empty();
	Obstacles.Empty();
	// *** //

	// Get AR manager, and reset it.
	// *** //
	auto Actor = UGameplayStatics::GetActorOfClass(GetWorld(), AHelloARManager::StaticClass());
	auto ARM = Cast<AHelloARManager>(Actor);

	if (ARM)
	{
		ARM->ResetARCoreSession();
	}
	// *** //
}

void ACustomGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Count dead
	// *** //
	int RedDead = 0;
	int BlueDead = 0;

	for (auto Actor : RedTeamActors)
	{
		if (Actor->GetIsDead())
		{
			RedDead++;
		}
	}

	for (auto Actor : BlueTeamActors)
	{
		if (Actor->GetIsDead())
		{
			BlueDead++;
		}
	}
	// *** //

	// Decide winning team based on dead actors.
	if (RedDead == PawnsPerTeam)
	{
		bHasBlueWon = true;
		CurrentPhase = EGamePhase::GAME_END;
	}
	else if (BlueDead == PawnsPerTeam)
	{
		bHasRedWon = true;
		CurrentPhase = EGamePhase::GAME_END;
	}
}

void ACustomGameMode::SpawnInitialActors()
{
	// Spawn an instance of the HelloARManager class
	AHelloARManager* ARManager = GetWorld()->SpawnActor<AHelloARManager>();
}


void ACustomGameMode::LineTraceSpawnActor(FVector ScreenPos)
{
	//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Line Trace Reached"));

	//Basic variables for functionality
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	FVector WorldPos;
	FVector WorldDir;

	//Gets the screen touch in world space and the tracked objects from a line trace from the touch
	UGameplayStatics::DeprojectScreenToWorld(PlayerController, FVector2D(ScreenPos), WorldPos, WorldDir);

	// Notice that this LineTrace is in the ARBluePrintLibrary - this means that it's exclusive only for objects tracked by ARKit/ARCore
	auto TraceResult = UARBlueprintLibrary::LineTraceTrackedObjects(FVector2D(ScreenPos), false, false, false, true);

	//Checks if the location is valid
	if (TraceResult.IsValidIndex(0))
	{
		// Get the first found object in the line trace - ignoring the rest of the array elements
		auto TrackedTF = TraceResult[0].GetLocalToWorldTransform();

		if (FVector::DotProduct(TrackedTF.GetRotation().GetUpVector(), WorldDir) < 0)
		{
			//Spawn the actor pin and get the transform
			UARPin* ActorPin = UARBlueprintLibrary::PinComponent(nullptr, TraceResult[0].GetLocalToWorldTransform(), TraceResult[0].GetTrackedGeometry());

			// Check if ARPins are available on your current device. ARPins are currently not supported locally by ARKit, so on iOS, this will always be "FALSE" 
			if (ActorPin)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, TEXT("ARPin is valid"));
			
				// Pin transform
				auto PinTF = ActorPin->GetLocalToWorldTransform();

				const FActorSpawnParameters SpawnInfo;
				const FRotator MyRot(0, 0, 0);
				const FVector MyLoc(0, 0, 0);

				// construct trace vector (from point tapped to 1000.0 units beyond in same direction)
				FVector TraceEndVector = WorldDir * 1000.0;
				TraceEndVector = WorldPos + TraceEndVector;

				// perform line trace (Raycast)
				FHitResult Hit;
				bool TraceSuccess = GetWorld()->LineTraceSingleByChannel(Hit, WorldPos, TraceEndVector, ECollisionChannel::ECC_WorldDynamic);

				// Check if obstacle was hit
				if (Cast<AObstacle>(Hit.GetActor()))
				{
					// Don't spawn pawn if an obstacle is in the way.
				}
				else
				{
					// Spawn red pawns until correct number is reached.
					if (RedTeamActors.Num() < PawnsPerTeam)
					{
						// Spawn actor, set pin, then add to array.
						AFighterPawn* SpawnedActor = GetWorld()->SpawnActor<AFighterPawn>(MyLoc, MyRot, SpawnInfo);
						SpawnedActor->SetColor(FColor::Red);
						SpawnedActor->SetActorTransform(PinTF);
						SpawnedActor->PinComponent = ActorPin;
						RedTeamActors.Add(SpawnedActor);

						// Move onto next turn if all actors have been spawned.
						if (RedTeamActors.Num() == PawnsPerTeam)
						{
							bIsRedTurn = false;
						}
					}
					else if (BlueTeamActors.Num() < PawnsPerTeam)
					{
						// Spawn actor, set pin, then add to array.
						AFighterPawn* SpawnedActor = GetWorld()->SpawnActor<AFighterPawn>(MyLoc, MyRot, SpawnInfo);
						SpawnedActor->SetColor(FColor::Blue);
						SpawnedActor->SetActorTransform(PinTF);
						SpawnedActor->PinComponent = ActorPin;
						BlueTeamActors.Add(SpawnedActor);

						// Once all blue actors have been spawned...
						if (BlueTeamActors.Num() == PawnsPerTeam)
						{
							// Move to red turn.
							bIsRedTurn = true;

							// Calculate average position of each teams, and then make them face each other based on this position
							// *** //
							TArray<AActor*> Blues(BlueTeamActors);
							TArray<AActor*> Reds(RedTeamActors);

							FVector AvgBluePos = UGameplayStatics::GetActorArrayAverageLocation(Blues);
							FVector AvgRedPos = UGameplayStatics::GetActorArrayAverageLocation(Reds);

							for (auto Blue : BlueTeamActors)
							{
								Blue->SetActorRotation(UKismetMathLibrary::FindLookAtRotation(Blue->GetActorLocation(), AvgRedPos));
							}

							for (auto Red : RedTeamActors)
							{
								Red->SetActorRotation(UKismetMathLibrary::FindLookAtRotation(Red->GetActorLocation(), AvgBluePos));
							}
							// *** //

							// Start the next turn.
							CurrentPhase = EGamePhase::TURN_IDLE;
							StartTurn();
						}
					}
				}
			}
		}
	}
}

void ACustomGameMode::LineTraceSpawnObstacle(FVector ScreenPos)
{
	//Basic variables for functionality
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	FVector WorldPos;
	FVector WorldDir;

	//Gets the screen touch in world space and the tracked objects from a line trace from the touch
	UGameplayStatics::DeprojectScreenToWorld(PlayerController, FVector2D(ScreenPos), WorldPos, WorldDir);
	auto TraceResult = UARBlueprintLibrary::LineTraceTrackedObjects(FVector2D(ScreenPos), false, false, false, true);

	//Checks if the location is valid
	if (TraceResult.IsValidIndex(0))
	{
		// Get the first found object in the line trace - ignoring the rest of the array elements
		auto TrackedTF = TraceResult[0].GetLocalToWorldTransform();

		if (FVector::DotProduct(TrackedTF.GetRotation().GetUpVector(), WorldDir) < 0)
		{
			//Spawn the actor pin and get the transform
			UARPin* ActorPin = UARBlueprintLibrary::PinComponent(nullptr, TraceResult[0].GetLocalToWorldTransform(), TraceResult[0].GetTrackedGeometry());

			// Check if ARPins are available on your current device. ARPins are currently not supported locally by ARKit, so on iOS, this will always be "FALSE" 
			if (ActorPin)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::White, TEXT("ARPin is valid"));
				
				// Get pin transform
				auto PinTF = ActorPin->GetLocalToWorldTransform();

				const FActorSpawnParameters SpawnInfo;
				const FRotator MyRot(0, 0, 0);
				const FVector MyLoc(0, 0, 0);

				// Spawn obstacles with pins until the obstacle limit is reached.
				if (Obstacles.Num() < ObstacleLimit)
				{
					AObstacle* SpawnedActor = GetWorld()->SpawnActor<AObstacle>(MyLoc, MyRot, SpawnInfo);
					SpawnedActor->SetActorTransform(PinTF);
					SpawnedActor->PinComponent = ActorPin;
					Obstacles.Add(SpawnedActor);
				}
			}
		}
	}
}

void ACustomGameMode::TogglePlaneVisibility()
{
	// Get all planes.
	TArray<AActor*> Planes;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AARPlaneActor::StaticClass(), Planes);

	// Toggle visibility on each plane.
	if (Planes.Num() > 0)
	{
		for (auto Actor : Planes)
		{
			auto Plane = Cast<AARPlaneActor>(Actor);
			if (Plane)
			{
				Plane->bIsVisibleOverride = !Plane->bIsVisibleOverride;
			}
		}
	}

}

bool ACustomGameMode::LineTraceCheckForPlane(FVector2D ScreenPos)
{
	//Basic variables for functionality
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	FVector WorldPos;
	FVector WorldDir;

	//Gets the screen touch in world space and the tracked objects from a line trace from the touch
	bool DeprojectionSuccess = UGameplayStatics::DeprojectScreenToWorld(PlayerController, FVector2D(ScreenPos), WorldPos, WorldDir);
	auto TraceResult = UARBlueprintLibrary::LineTraceTrackedObjects(FVector2D(ScreenPos), false, false, false, true);

	// construct trace vector (from point clicked to 1000.0 units beyond in same direction)
	FVector TraceEndVector = WorldDir * 1000.0;
	TraceEndVector = WorldPos + TraceEndVector;

	// If trace worked.
	if (TraceResult.IsValidIndex(0))
	{
		// Cast traced object to plane.
		UARPlaneGeometry* PlaneGeometry = Cast<UARPlaneGeometry>(TraceResult[0].GetTrackedGeometry());

		// If cast successful...
		if (PlaneGeometry)
		{
			// Set the used plane in the ARManager.
			AHelloARManager* ARManager = Cast<AHelloARManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AHelloARManager::StaticClass()));
			if (ARManager)
			{
				ARManager->SetUsedPlane(PlaneGeometry);
				return true;
			}
		}
	}
	return false;
}

bool ACustomGameMode::LineTraceSelectPawn(FVector2D ScreenPos)
{
	// Stop targeting before selecting a new pawn.
	CurrentFighter->EndTargeting();

	// Get player controller
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	// Perform deprojection taking 2d clicked area and generating reference in 3d world-space.
	FVector WorldPosition;
	FVector WorldDirection; // Unit Vector
	bool DeprojectionSuccess = UGameplayStatics::DeprojectScreenToWorld(PlayerController, ScreenPos, /*out*/ WorldPosition, /*out*/ WorldDirection);


	if (DeprojectionSuccess)
	{
		// construct trace vector (from point clicked to 1000.0 units beyond in same direction)
		FVector TraceEndVector = WorldDirection * 1000.0;
		TraceEndVector = WorldPosition + TraceEndVector;
		
		FCollisionQueryParams CollisionParameters;

		// Ignore friendly fighters and obstacles when tracing.
		// *** //
		if (bIsRedTurn)
		{
			for (auto Actor : RedTeamActors)
			{
				CollisionParameters.AddIgnoredActor(Actor);
				CollisionParameters.AddIgnoredActors(Actor->Children);
			}
		}
		else
		{
			for (auto Actor : BlueTeamActors)
			{
				CollisionParameters.AddIgnoredActor(Actor);
				CollisionParameters.AddIgnoredActors(Actor->Children);
			}
		}

		for (auto Obstacle : Obstacles)
		{
			CollisionParameters.AddIgnoredActor(Obstacle);
		}
		// *** //

		// Perform trace.
		FHitResult Hit;
		GetWorld()->LineTraceSingleByChannel(Hit, WorldPosition, TraceEndVector, ECollisionChannel::ECC_WorldDynamic, CollisionParameters);

		// Cast hit actor to fighter.
		AFighterPawn* Target = Cast<AFighterPawn>(Hit.GetActor());

		// If trace hit a fighter, and it's not dead, set that as the target.
		if (Target)
		{
			if (!Target->GetIsDead())
			{
				CurrentFighter->SelectTarget(Target);
				return true;
			}
		}
	}

	return false;
}

void ACustomGameMode::LineTraceMovePawn(FVector2D ScreenPos)
{
	// Get player controller
	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	// Perform deprojection taking 2d clicked area and generating reference in 3d world-space.
	FVector WorldPosition;
	FVector WorldDirection; // Unit Vector
	bool DeprojectionSuccess = UGameplayStatics::DeprojectScreenToWorld(PlayerController, ScreenPos, /*out*/ WorldPosition, /*out*/ WorldDirection);

	// Notice that this LineTrace is in the ARBluePrintLibrary - this means that it's exclusive only for objects tracked by ARKit/ARCore
	auto TraceResult = UARBlueprintLibrary::LineTraceTrackedObjects(FVector2D(ScreenPos), false, false, false, true);

	//Checks if the location is valid
	if (TraceResult.IsValidIndex(0))
	{
		// Get the first found object in the line trace - ignoring the rest of the array elements
		auto TrackedTF = TraceResult[0].GetLocalToWorldTransform();

		// Set the target movement location.
		if (FVector::DotProduct(TrackedTF.GetRotation().GetUpVector(), WorldDirection) < 0)
		{
			CurrentFighter->MoveTo(TrackedTF.GetLocation());
		}
	}
}



