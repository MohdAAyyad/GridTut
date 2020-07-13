// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Grid/GridManager.h"
#include "GridTutCharacter.generated.h"

UCLASS(Blueprintable)
class AGridTutCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGridTutCharacter();

	// Called every frame.
	virtual void Tick(float DeltaSeconds) override;

	/** Returns TopDownCameraComponent subobject **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns CursorToWorld subobject **/
	FORCEINLINE class UDecalComponent* GetCursorToWorld() { return CursorToWorld; }

private:
	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** A decal that projects to the cursor location. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UDecalComponent* CursorToWorld;

protected:

	UPROPERTY(EditAnywhere, Category = "Grid")
		int rowSpeed;
	UPROPERTY(EditAnywhere, Category = "Grid")
		int columnSpeed;
	UPROPERTY(EditAnywhere, Category = "Grid")
		int depth;

	ATile* currentTile;
	ATile* targetTile;
	TArray<ATile*> movementPath;

	ATile* GetTileWithMinFCost(TArray<ATile*> tiles_);
	TArray<FVector> path;

	bool bMoving;

	void MoveAccordingToPath();

public:

	void Selected();
	void NotSelected();
	void SetTargetTile(ATile* tile_);
	TArray<FVector> GetPath();

	void UpdateMovementPath(ATile* tile_);
	bool DoesOpenListHaveALowerFCost(TArray<ATile*> list_, int fCost_);
	bool DoesClosedListHaveALowerFCost(TArray<ATile*> list_, int fCost_);

	

};

