// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "GridTutPlayerController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Runtime/Engine/Classes/Components/DecalComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "GridTutCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/World.h"

AGridTutPlayerController::AGridTutPlayerController()
{
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Crosshairs;
	controlledCharacter = nullptr;
	tileInPathIndex = 0;
	bMovingCamera = false;
}

void AGridTutPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	if (bMoveToMouseCursor)
	{
		if (path.Num() > 0) //Move until path has been traversed
		{
			if (SetNewMoveDestination(destination) <= 5.0f)
			{
				//Get the next tile location
				UpdateDestination();
			}
		}
		else
		{
			bMoveToMouseCursor = false;
		}
	}
}

void AGridTutPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	InputComponent->BindAction("LeftMouse", IE_Pressed, this, &AGridTutPlayerController::HandleMousePress);
	InputComponent->BindAction("RightMouse", IE_Pressed, this, &AGridTutPlayerController::MoveCamera);
	InputComponent->BindAction("RightMouse", IE_Released, this, &AGridTutPlayerController::MoveCamera);
	InputComponent->BindAction("ResetView", IE_Released, this, &AGridTutPlayerController::ResetView);
	InputComponent->BindAxis("TurnView", this, &AGridTutPlayerController::LookRate);
	InputComponent->BindAxis("LookUpView", this, &AGridTutPlayerController::LookUpRate);

}


float AGridTutPlayerController::SetNewMoveDestination(const FVector DestLocation)
{
	APawn* const MyPawn = GetPawn();
	if (MyPawn)
	{
		float const Distance = FVector::Dist(DestLocation, MyPawn->GetActorLocation());

		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, DestLocation);

		return Distance;
	}
	return 0.0f;
}

void AGridTutPlayerController::HandleMousePress()
{
	FHitResult hit;
	GetHitResultUnderCursor(ECC_Camera, false, hit);

	if (hit.bBlockingHit)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Hit something"));
		if (!controlledCharacter) //If we don't have a controller character, see if we've 
		{
			controlledCharacter = Cast<AGridTutCharacter>(hit.Actor);
			if (controlledCharacter)
			{
				controlledCharacter->Selected();
				SetViewTargetWithBlend(controlledCharacter,0.35f);		
				if(srpgPawn)
					srpgPawn->SetUnderControl(false);
			}
			//UE_LOG(LogTemp, Warning, TEXT("Got player"));

		
		}
		else
		{
			 targetTile = Cast<ATile>(hit.Actor);
			if (targetTile)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Got Tile"));
				// We hit a tile, move there
					// set flag to keep updating destination until released
				if (targetTile->GetHighlighted())
				{
					controlledCharacter->SetTargetTile(targetTile);
					path = controlledCharacter->GetPath();
					if (path.Num() > 0)
					{
						//bMoveToMouseCursor = true;
						tileInPathIndex = 0;
						destination = path[0];
					}
				}
				else
				{
					controlledCharacter->NotSelected();
					controlledCharacter = nullptr;
				}
			}
			else
			{
				controlledCharacter->NotSelected();
				controlledCharacter = nullptr;
			}
		}
	}
}

void AGridTutPlayerController::UpdateDestination()
{
	//We have arrived at destination so remove it and move on to the next one
	if (tileInPathIndex < path.Num())
	{
		path.RemoveAt(tileInPathIndex);
		tileInPathIndex++;
		if (tileInPathIndex < path.Num())
		{
			destination = path[tileInPathIndex];
		}
	}
}

void AGridTutPlayerController::MoveCamera()
{
	bMovingCamera = !bMovingCamera;
	if (!bMovingCamera)
	{
		ControlRotation = FRotator::ZeroRotator;
	}
}

void AGridTutPlayerController::LookRate(float rate_)
{
	if (bMovingCamera)
	{

		//AddPitchInput(rate_ * 1.0f * GetWorld()->GetDeltaSeconds());
		FMath::ClampAngle(GetControlRotation().Yaw, -45.0f, 45.0f);
		AddRollInput(rate_ * 1.0f * GetWorld()->GetDeltaSeconds());
		FMath::ClampAngle(GetControlRotation().Roll, -45.0f, 45.0f);
		if (controlledCharacter)
		{
			controlledCharacter->GetTopDownCameraComponent()->AddRelativeRotation(GetControlRotation());
		}
		
	}
}
void AGridTutPlayerController::LookUpRate(float rate_)
{
	if (bMovingCamera)
	{
		UE_LOG(LogTemp, Warning, TEXT("Pitching camera"));
		if (controlledCharacter)
		{
			FVector loc = controlledCharacter->GetTopDownCameraComponent()->GetComponentLocation();
			loc.X += rate_ * 1.0f * GetWorld()->GetDeltaSeconds();
			controlledCharacter->GetCameraBoom()->AddRelativeLocation(FVector(rate_ * 1.0f * GetWorld()->GetDeltaSeconds(), 0.0f, 0.0f));
		}
	}
}

void AGridTutPlayerController::ResetView()
{
	if (srpgPawn)
	{
		SetViewTargetWithBlend(srpgPawn, 0.3f);
		srpgPawn->SetUnderControl(true);
	}
	if (controlledCharacter)
	{
		controlledCharacter->NotSelected();
	}
}

void AGridTutPlayerController::SetSRPGPawn(ASRPGPlayer* pawn_)
{
	srpgPawn = pawn_;

	if (srpgPawn)
	{
		if (InputComponent)
		{
			InputComponent->BindAxis("Up", srpgPawn, &ASRPGPlayer::MoveUpDown);
			InputComponent->BindAxis("Right", srpgPawn, &ASRPGPlayer::MoveRightLeft);
			InputComponent->BindAxis("Zoom", srpgPawn, &ASRPGPlayer::Zoom);
		}
	}
}