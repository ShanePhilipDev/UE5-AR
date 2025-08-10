// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameModeBase.h"
#include "FighterPawn.h"
#include "Blueprint/UserWidget.h"
#include "Obstacle.h"

#include "CustomGameMode.generated.h"

//Forward Declarations
class APlaceableActor;

/**
 * 
 */

// Enum for the stages of the game.
UENUM(BlueprintType)
enum class EGamePhase : uint8
{
	MENU			UMETA(DisplayName = "Menu"),
	PLANE_SETUP		UMETA(DisplayName = "Plane Setup"),
	PAWN_SETUP		UMETA(DisplayName = "Pawn Setup"),
	OBSTACLE_SETUP	UMETA(DisplayName = "Obstacle Setup"),
	TURN_IDLE		UMETA(DisplayName = "Turn - Idle"),
	TURN_SHOOT		UMETA(DisplayName = "Turn - Shoot"),
	TURN_GRENADE	UMETA(DisplayName = "Turn - Grenade"),
	TURN_MOVEMENT	UMETA(DisplayName = "Turn - Movement"),
	GAME_END		UMETA(DisplayName = "Game End")
};

UCLASS()
class UE5_AR_API ACustomGameMode : public AGameModeBase
{
	GENERATED_BODY()
private:
	// Fighter and obstacle arrays.
	TArray<AFighterPawn*> RedTeamActors;
	TArray<AFighterPawn*> BlueTeamActors;
	TArray<AObstacle*> Obstacles;

	// Fighter and obstacle limits.
	int ObstacleLimit;
	int PawnsPerTeam;

	// Fighter turn counters for each team- determines which fighter is used this turn.
	int RedTurnCounter;
	int BlueTurnCounter;

public:
	// Constructor and destructor.
	ACustomGameMode();
	virtual ~ACustomGameMode() = default;

	virtual void StartPlay() override;

	UFUNCTION(BlueprintNativeEvent, Category = "GameModeBase", DisplayName = "Start Play")
	void StartPlayEvent();

	// Current stage of the game.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	EGamePhase CurrentPhase;

	// Track whether it's red or blue's turn.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bIsRedTurn;

	// Track which team has won.
	// *** //
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bHasRedWon;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool bHasBlueWon;
	// *** //

	// The current turn's pawn.
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	AFighterPawn* CurrentFighter;

	// Getters for arrays
	// *** //
	UFUNCTION(BlueprintCallable)
	TArray<AObstacle*> GetObstacles() { return Obstacles; };

	UFUNCTION(BlueprintCallable)
	TArray<AFighterPawn*> GetRedTeam() { return RedTeamActors; };

	UFUNCTION(BlueprintCallable)
	TArray<AFighterPawn*> GetBlueTeam() { return BlueTeamActors; };
	// *** //
	
	// Start the game.
	UFUNCTION(BlueprintCallable)
	void StartGame();

	// Start and end turn.
	// *** //
	UFUNCTION(BlueprintCallable)
	void StartTurn();

	UFUNCTION(BlueprintCallable)
	void EndTurn();
	// *** //
	
	// Return to the main menu.
	UFUNCTION(BlueprintCallable)
	void ReturnToMenu();
	
	// Reset for re-starting the game.
	void Reset();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	// Line trace functions for performing actions on touch.
	// *** //
	// Spawn fighter
	virtual void LineTraceSpawnActor(FVector ScreenPos);

	// Spawn obstacle
	void LineTraceSpawnObstacle(FVector ScreenPos);

	// Select plane
	bool LineTraceCheckForPlane(FVector2D ScreenPos);

	// Select fighter
	bool LineTraceSelectPawn(FVector2D ScreenPos);

	// Move fighter
	void LineTraceMovePawn(FVector2D ScreenPos);
	// *** //

	// Hides and unhides planes.
	UFUNCTION(BlueprintCallable)
	void TogglePlaneVisibility();

	// Spawn the needed actors.
	virtual void SpawnInitialActors();

	// UI widgets
	// *** //
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UUserWidget* MenuWidget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UUserWidget* GameWidget;
	// *** //
};
