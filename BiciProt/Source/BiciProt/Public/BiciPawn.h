// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BiciMovementComponent.h"
#include "BiciReplicationComponent.h"
#include "BiciPawn.generated.h"

UCLASS()
class BICIPROT_API ABiciPawn : public APawn
{
	GENERATED_BODY()

protected:

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly)
	UBiciMovementComponent* Movement;

	UPROPERTY(VisibleAnyWhere, BlueprintReadOnly)
	UBiciReplicationComponent* Replicator;

	void MoveForward(float Value);

	void MoveRight(float Value);

public:

	// Sets default values for this pawn's properties
	ABiciPawn();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
