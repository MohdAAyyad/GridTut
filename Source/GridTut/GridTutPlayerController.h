// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GridTutCharacter.h"
#include "Grid/Tile.h"
#include "SRPGPlayer.h"
#include "GridTutPlayerController.generated.h"

UCLASS()
class AGridTutPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AGridTutPlayerController();

	void SetSRPGPawn(ASRPGPlayer* pawn_);

protected:
	/** True if the controlled character should navigate to the mouse cursor. */
	uint32 bMoveToMouseCursor : 1;

	// Begin PlayerController interface
	virtual void PlayerTick(float DeltaTime) override;
	virtual void SetupInputComponent() override;
	// End PlayerController interface
	
	/** Navigate player to the given world location. */
	float SetNewMoveDestination(const FVector DestLocation);

	/** Input handlers for SetDestination action. */
	void HandleMousePress();

	AGridTutCharacter* controlledCharacter;
	ASRPGPlayer* srpgPawn;
	ATile* targetTile;
	FVector destination;
	TArray<FVector> path;
	int tileInPathIndex;

	void UpdateDestination();

	bool bMovingCamera;

	void MoveCamera();

	void LookRate(float rate_);
	void LookUpRate(float rate_);

	void ResetView();
};


