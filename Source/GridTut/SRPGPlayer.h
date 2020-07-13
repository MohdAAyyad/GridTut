// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SRPGPlayer.generated.h"

UCLASS()
class GRIDTUT_API ASRPGPlayer : public APawn
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASRPGPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Root)
		USceneComponent* root;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
		class UCameraComponent* mainCamera;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Camera)
		class USpringArmComponent* cameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
		class UDecalComponent* CursorToWorld;

	class AGridTutPlayerController* pController;
	bool bUnderControl;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void MoveUpDown(float rate_);
	void MoveRightLeft(float rate_);
	void SetUnderControl(bool value_);
	void Zoom(float rate_);

};
