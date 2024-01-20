

#include "Transporter.h"
#include "PressurePlate.h"

UTransporter::UTransporter()
{

	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicatedByDefault(true);

	MoveTime = 3.0f;
	ActivatedTriggerCount = 0;

	bArePointsSet = false;
	StartPoint = FVector::Zero();
	EndPoint = FVector::Zero();
}



void UTransporter::BeginPlay()
{
	Super::BeginPlay();

	if (OwnerIsTriggerActor)
	{
		TriggerActors.Add(GetOwner());
	}

	for (AActor* TA : TriggerActors)
	{
		APressurePlate *PressurePlateActor = Cast<APressurePlate>(TA);
		if (PressurePlateActor)
		{
			PressurePlateActor->OnActivated.AddDynamic(this, &UTransporter::OnPressurePlateActivated);
			PressurePlateActor->OnDeactivated.AddDynamic(this, &UTransporter::OnPressurePlateDeactivated);
		}
	}
	
}

void UTransporter::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (TriggerActors.Num() > 0)
	{
		bAllTriggerActorsTriggered = ActivatedTriggerCount >= TriggerActors.Num();
		if (bAllTriggerActorsTriggered)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, FString("AllTriggerActorsTriggered!"));
		}
	}

	AActor* MyOwner = GetOwner();

	if (MyOwner && MyOwner->HasAuthority() && bArePointsSet)
	{
		FVector CurrentLocation = MyOwner->GetActorLocation();
		float Speed = FVector::Distance(StartPoint, EndPoint) / MoveTime;

		FVector TargetLocation = bAllTriggerActorsTriggered ? EndPoint : StartPoint;
		if (!CurrentLocation.Equals(TargetLocation))
		{
			FVector NewLocation = FMath::VInterpConstantTo(CurrentLocation, TargetLocation, DeltaTime, Speed);
			MyOwner->SetActorLocation(NewLocation);
		}
	}

}

void UTransporter::SetPoints(FVector Start, FVector End)
{
	if (Start.Equals(End)) return;

	StartPoint = Start;
	EndPoint = End;
	bArePointsSet = true;
}

void UTransporter::OnPressurePlateActivated()
{
	ActivatedTriggerCount++;

	FString Msg = FString::Printf(TEXT("Transporter Activated: %d"), ActivatedTriggerCount);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, Msg);
}

void UTransporter::OnPressurePlateDeactivated()
{
	ActivatedTriggerCount--;
	FString Msg = FString::Printf(TEXT("Transporter Deactivated: %d"), ActivatedTriggerCount);
	GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::White, Msg);
}

