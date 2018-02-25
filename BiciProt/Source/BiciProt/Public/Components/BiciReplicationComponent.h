// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BiciMovementComponent.h"
#include "BiciReplicationComponent.generated.h"

USTRUCT()
struct FBiciState
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FVector Velocity;

	UPROPERTY()
	FTransform Transform;

	FBiciMoves LastMove;

};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BICIPROT_API UBiciReplicationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UBiciReplicationComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:

	UPROPERTY()
	class UBiciMovementComponent* MovementComp;

	UFUNCTION()
	void UpdateServerState(const FBiciMoves& Move);

	void ClientTick(float DeltaTime);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SendMove(FBiciMoves Move);

	UFUNCTION()
	void OnRep_ServerState();

	void AuthonomousProxy_OnRep_ServerState();

	void SimulatedProxy_OnRep_ServerState();

	UPROPERTY(ReplicatedUsing = OnRep_ServerState)
	FBiciState ServerState;

	void ClearAcknowledgeMoves(FBiciMoves LastMove);

	TArray<FBiciMoves> UnacknowledgedMoves;

	float ClientTimeSinceUpdate;

	float ClientTimeBetweenLastUpdate;

	FVector ClientStartLocation;
};
