// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Transporter.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class COOPADVENTURE_API UTransporter : public UActorComponent
{
	GENERATED_BODY()

public:	
	UTransporter();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FVector StartPoint;
	FVector EndPoint;

	bool bArePointsSet;

	UPROPERTY(EditAnywhere)
	TArray<AActor*> TriggerActors;

	UPROPERTY(EditAnywhere)
	float MoveTime;

	UPROPERTY(EditAnywhere)
	bool OwnerIsTriggerActor;

	UPROPERTY(VisibleAnywhere)
	int ActivatedTriggerCount;


	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	bool bAllTriggerActorsTriggered;

	UFUNCTION(BlueprintCallable)
	void SetPoints(FVector Start, FVector End);

	UFUNCTION()
	void OnPressurePlateActivated();

	UFUNCTION()
	void OnPressurePlateDeactivated();
};
