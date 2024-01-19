// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "MultiplayerSessionsSubsystem.generated.h"
/**
 * 
 */
UCLASS()
class COOPADVENTURE_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	

public:
	UMultiplayerSessionsSubsystem();

	void Initialize(FSubsystemCollectionBase& Collection) override;
	void Deinitialize() override;

	IOnlineSessionPtr SessionInterface;

	UFUNCTION(BlueprintCallable)
	void CreateServer(FString ServerName);
	
	UFUNCTION(BlueprintCallable)
	void FindServer(FString ServerName);

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccesful);

	void OnDestroySessionComplete(FName SessionName, bool bWasSuccesful);

	void OnFindSessionsComplete(bool bWasSuccesful);

	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	bool bCreateServerAfterDestroy;
	FString DestroyServerName;
	FString FindServerName;
	FName MySessionName;

	TSharedPtr<FOnlineSessionSearch> SessionSearch;
};
