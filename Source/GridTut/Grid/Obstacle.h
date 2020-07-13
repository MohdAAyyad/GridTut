// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Obstacle.generated.h"

UCLASS()
class GRIDTUT_API AObstacle : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AObstacle();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, Category = "Obstacle")
		USceneComponent* root;

	UPROPERTY(EditAnywhere, Category = "Obstacle")
		UStaticMeshComponent* mesh;

	UPROPERTY(EditAnywhere, Category = "Obstacle")
		UBoxComponent* box;

public:	

};
