// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "GridTutCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "Components/DecalComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Materials/Material.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"

AGridTutCharacter::AGridTutCharacter()
{
	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Rotate character to moving direction
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	// Create a camera boom...
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->bAbsoluteRotation = true; // Don't want arm to rotate when character does
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->RelativeRotation = FRotator(-60.f, 0.f, 0.f);
	CameraBoom->bDoCollisionTest = false; // Don't want to pull camera in when it collides with level

	// Create a camera...
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	TopDownCameraComponent->SetActive(false);

	// Create a decal in the world to show the cursor's location
	CursorToWorld = CreateDefaultSubobject<UDecalComponent>("CursorToWorld");
	CursorToWorld->SetupAttachment(RootComponent);
	static ConstructorHelpers::FObjectFinder<UMaterial> DecalMaterialAsset(TEXT("Material'/Game/TopDownCPP/Blueprints/M_Cursor_Decal.M_Cursor_Decal'"));
	if (DecalMaterialAsset.Succeeded())
	{
		CursorToWorld->SetDecalMaterial(DecalMaterialAsset.Object);
	}
	CursorToWorld->DecalSize = FVector(16.0f, 32.0f, 32.0f);
	CursorToWorld->SetRelativeRotation(FRotator(90.0f, 0.0f, 0.0f).Quaternion());

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	currentTile = nullptr;
	targetTile = nullptr;

	rowSpeed = 5;
	columnSpeed = 3;
	depth = 2;

	bMoving = false;
}

void AGridTutCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

	if (CursorToWorld != nullptr)
	{
		 if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			FHitResult TraceHitResult;
			PC->GetHitResultUnderCursor(ECC_Visibility, true, TraceHitResult);
			FVector CursorFV = TraceHitResult.ImpactNormal;
			FRotator CursorR = CursorFV.Rotation();
			CursorToWorld->SetWorldLocation(TraceHitResult.Location);
			CursorToWorld->SetWorldRotation(CursorR);
		}
	}

	if (bMoving && path.Num()>0)
	{
		MoveAccordingToPath();
	}
}

void AGridTutCharacter::Selected()
{
	FHitResult hit;
	FVector end = GetActorLocation();
	movementPath.Empty();
	end.Z -= 400.0f;
	if(GetWorld()->LineTraceSingleByChannel(hit, GetActorLocation(), end, ECollisionChannel::ECC_Visibility))
	{
		currentTile = Cast<ATile>(hit.Actor);
		if (currentTile)
		{			
			currentTile->GetGridManager()->UpdateCurrentTile(currentTile, rowSpeed, columnSpeed, depth);
		}
	}
}

void AGridTutCharacter::NotSelected()
{
	if (currentTile)
	{
		currentTile->GetGridManager()->ClearHighlighted();
	}
}

void AGridTutCharacter::SetTargetTile(ATile* tile_)
{
	targetTile = tile_;
}

TArray<FVector> AGridTutCharacter::GetPath()
{
	TArray<ATile*> open;
	TArray<ATile*> closed;


	ATile* currentNode = nullptr;

	bool bFoundTarget = false;
	open.Push(currentTile);

	while (open.Num() > 0 )
	{
		currentNode = GetTileWithMinFCost(open);

		open.Remove(currentNode);

		if (currentNode == targetTile) //We've found the goal, get out
			break;

		closed.Push(currentNode); //The current node has been visited

		//Update parent nodes for immediate neighbors
		for (int i = 0; i < currentNode->GetImmediateNeighbors().Num(); i++)
		{
			if (currentNode->GetImmediateNeighbors()[i] != nullptr)
			{
				if (closed.Contains(currentNode->GetImmediateNeighbors()[i])) //If the neighbor has already been visited, move on to the next one
					continue;

				currentNode->GetImmediateNeighbors()[i]->SetParentTile(currentNode); // Update the parent of the tile
				if(currentNode->gCost + 10 < currentNode->GetImmediateNeighbors()[i]->gCost) //Update the gcost of the neighbor only if the new gcost is smaller than the previous one
					currentNode->GetImmediateNeighbors()[i]->gCost = currentNode->gCost + 10; //Distance between node and imm neighbor is assumed 10
				currentNode->GetImmediateNeighbors()[i]->CalculateHCost(targetTile);
				currentNode->GetImmediateNeighbors()[i]->fCost = currentNode->GetImmediateNeighbors()[i]->gCost + currentNode->GetImmediateNeighbors()[i]->hCost;

				if (!open.Contains(currentNode->GetImmediateNeighbors()[i]) 
					&& !DoesClosedListHaveALowerFCost(open, currentNode->GetImmediateNeighbors()[i]->fCost)
					&& !DoesOpenListHaveALowerFCost(open, currentNode->GetImmediateNeighbors()[i]->fCost)
					&& currentNode->GetImmediateNeighbors()[i]->GetTraversable() 
					&& currentNode->GetImmediateNeighbors()[i]->GetHighlighted())
						open.Push(currentNode->GetImmediateNeighbors()[i]);
			}
			

		}

		//Same as before but for diagonal neighbors
		//Update parent for diagonal neighbors 
		for (int d = 0; d < currentNode->GetDiagonalNeighbors().Num(); d++)
		{
			if (currentNode->GetDiagonalNeighbors()[d] != nullptr)
			{
				if (closed.Contains(currentNode->GetDiagonalNeighbors()[d]))
					continue;

				currentNode->GetDiagonalNeighbors()[d]->SetParentTile(currentNode);
				if (currentNode->gCost + 14 < currentNode->GetDiagonalNeighbors()[d]->gCost)
					currentNode->GetDiagonalNeighbors()[d]->gCost = currentNode->gCost + 14; //Distance between node and imm neighbor is assumed 10
				currentNode->GetDiagonalNeighbors()[d]->CalculateHCost(targetTile);
				currentNode->GetDiagonalNeighbors()[d]->fCost = currentNode->GetDiagonalNeighbors()[d]->gCost + currentNode->GetDiagonalNeighbors()[d]->hCost;

				if (!open.Contains(currentNode->GetDiagonalNeighbors()[d])
					&& !DoesClosedListHaveALowerFCost(open, currentNode->GetDiagonalNeighbors()[d]->fCost)
					&& !DoesOpenListHaveALowerFCost(open, currentNode->GetDiagonalNeighbors()[d]->fCost)
					&& currentNode->GetDiagonalNeighbors()[d]->GetTraversable() 
					&& currentNode->GetDiagonalNeighbors()[d]->GetHighlighted())
						open.Push(currentNode->GetDiagonalNeighbors()[d]);
			}

		}
	}

	UpdateMovementPath(targetTile);
	currentTile->HighlightPath();

	for (int i = 0; i < movementPath.Num(); i++)
	{
		movementPath[i]->HighlightPath();
		path.Push(movementPath[i]->GetActorLocation());
	}
	bMoving = true;
	return path;
}

void AGridTutCharacter::MoveAccordingToPath()
{
	FVector direction = path[path.Num()-1] - GetActorLocation();
	if (FMath::Abs(path[path.Num() - 1].X - GetActorLocation().X) > 10.0f || FMath::Abs(path[path.Num() - 1].Y - GetActorLocation().Y) > 10.0f)
	{
		AddMovementInput(direction, 1.0f);
	}
	else
	{
		path.RemoveAt(path.Num() - 1);
		if (path.Num() == 0)
			bMoving = false;
	}

}

ATile* AGridTutCharacter::GetTileWithMinFCost(TArray<ATile*> tiles_)
{
	int min = 0;
	ATile* minTile = nullptr;
	if (tiles_.Num() > 0)
	{
		min = tiles_[0]->fCost;
		minTile = tiles_[0];
		for (int i = 0; i < tiles_.Num(); i++)
		{
			if (tiles_[i]->fCost < min)
			{
				min = tiles_[i]->fCost;
				minTile = tiles_[i];
			}
			else if (tiles_[i]->fCost == min && tiles_[i]->hCost < minTile->hCost)
			{
				min = tiles_[i]->fCost;
				minTile = tiles_[i];
			}
		}
	}

	return minTile;
}

void AGridTutCharacter::UpdateMovementPath(ATile* tile_)
{
	if (tile_->GetParentTile()!=nullptr && !movementPath.Contains(tile_))
	{
		movementPath.Push(tile_);
		if( tile_->GetParentTile() != currentTile)
			UpdateMovementPath(tile_->GetParentTile());
	}
}

bool AGridTutCharacter::DoesOpenListHaveALowerFCost(TArray<ATile*> list_, int fCost_)
{
	if (list_.Num() > 0)
	{
		int min = fCost_;

		for (int i = 0; i < list_.Num(); i++)
		{
			if (list_[i]->fCost < min)
				return true;
		}
	}
	return false;
}
bool AGridTutCharacter::DoesClosedListHaveALowerFCost(TArray<ATile*> list_, int fCost_)
{
	if (list_.Num() > 0)
	{
		int min = fCost_;

		for (int i = 0; i < list_.Num(); i++)
		{
			if (list_[i]->fCost < min)
				return true;
		}
	}
	return false;
}