// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include <Online/OnlineSessionNames.h>

void PrintString(const FString& Str)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 15.f, FColor::Cyan, Str);
	}
	
}

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem()
{
	//PrintString("MSS Constructor");

	bCreateServerAfterDestroy = false;
	DestroyServerName = "";
	FindServerName = "";
	MySessionName = FName("Co-op Adventure Session Name");
}

void UMultiplayerSessionsSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	//PrintString("MSS Initialize");
	IOnlineSubsystem* OnlineSubSystem = IOnlineSubsystem::Get();
	if (OnlineSubSystem)
	{
		FString SubSystemName = OnlineSubSystem->GetSubsystemName().ToString();
		PrintString(SubSystemName);

		SessionInterface = OnlineSubSystem->GetSessionInterface();

		if (SessionInterface.IsValid())
		{
			SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, 
				&UMultiplayerSessionsSubsystem::OnCreateSessionComplete);

			SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(this, 
				&UMultiplayerSessionsSubsystem::OnDestroySessionComplete);

			SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this,
				&UMultiplayerSessionsSubsystem::OnFindSessionsComplete);

			SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this,
				&UMultiplayerSessionsSubsystem::OnJoinSessionComplete);
		}
	}
}

void UMultiplayerSessionsSubsystem::Deinitialize()
{
	UE_LOG(LogTemp, Warning, TEXT("MSS Deinitialize"));
}

void UMultiplayerSessionsSubsystem::CreateServer(FString ServerName)
{
	PrintString("CreateServer");
	
	if (ServerName.IsEmpty())
	{
		PrintString("Server name cannot be empty");
		ServerCreateDel.Broadcast(false);
		return;
	}

	FNamedOnlineSession *ExistingSession = SessionInterface->GetNamedSession(MySessionName);
	if (ExistingSession)
	{
		PrintString(FString::Printf(TEXT("Destroying existing session: %s"), 
			*MySessionName.ToString()));
		bCreateServerAfterDestroy = true;
		DestroyServerName         = ServerName;
		SessionInterface->DestroySession(MySessionName);
		return;
	}

	FOnlineSessionSettings SessionSettings;

	SessionSettings.bAllowJoinInProgress = true;
	SessionSettings.bIsDedicated = false;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.NumPublicConnections = 2;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bAllowJoinViaPresence = true;
	bool bIsLan = false;
	if(IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
	{
		bIsLan = true;
	}
	SessionSettings.bIsLANMatch = bIsLan;

	SessionSettings.Set(FName("SERVER_NAME"), ServerName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	SessionInterface->CreateSession(0, MySessionName, SessionSettings);
}

void UMultiplayerSessionsSubsystem::FindServer(FString ServerName)
{
	PrintString("FindServer");

	if (ServerName.IsEmpty())
	{
		PrintString("Server name cannot be empty");
		ServerJoinDel.Broadcast(false);
		return;
	}

	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	bool bIsLan = false;
	if (IOnlineSubsystem::Get()->GetSubsystemName() == "NULL")
	{
		bIsLan = true;
	}
	SessionSearch->bIsLanQuery = bIsLan;
	SessionSearch->MaxSearchResults = 9999;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	FindServerName = ServerName;

	SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
}

void UMultiplayerSessionsSubsystem::OnCreateSessionComplete(FName SessionName, bool bWasSuccesful)
{
	PrintString(FString::Printf(TEXT("OnCreateSessionComplete: %d"), bWasSuccesful));

	ServerCreateDel.Broadcast(bWasSuccesful);

	if (bWasSuccesful)
	{
		GetWorld()->ServerTravel("/Game/ThirdPerson/Maps/ThirdPersonMap?listen");
	}
}

void UMultiplayerSessionsSubsystem::OnDestroySessionComplete(FName SessionName, bool bWasSuccesful)
{
	PrintString(FString::Printf(TEXT("OnDestroySessionComplete, SessionName: %s, Success: %d"), 
		*SessionName.ToString(), bWasSuccesful));
	if (bCreateServerAfterDestroy)
	{
		bCreateServerAfterDestroy = false;
		CreateServer(DestroyServerName);
	}
}

void UMultiplayerSessionsSubsystem::OnFindSessionsComplete(bool bWasSuccesful)
{
	if (!bWasSuccesful) return;
	if (FindServerName.IsEmpty()) return;

	TArray<FOnlineSessionSearchResult> Results = SessionSearch->SearchResults;
	FOnlineSessionSearchResult *CorrectResult = 0;
	if (Results.Num() > 0)
	{
		PrintString(FString::Printf(TEXT("OnFindSessionsComplete, Success: %d, Sessions Found: %d"), bWasSuccesful, Results.Num()));

		for (FOnlineSessionSearchResult Result : Results)
		{
			if (Result.IsValid())
			{
				FString ServerName = "No-name";
				Result.Session.SessionSettings.Get(FName("SERVER_NAME"), ServerName);

				if (ServerName.Equals(FindServerName))
				{
					CorrectResult = &Result;
					PrintString(FString::Printf(TEXT("Server Found, Server: %s"), *ServerName));
					break;
				}
			}
		}

		if (CorrectResult)
		{
			SessionInterface->JoinSession(0, MySessionName, *CorrectResult);
		}
		else
		{
			PrintString(FString::Printf(TEXT("Server not found: %s"), *FindServerName));
			FindServerName = "";
			ServerJoinDel.Broadcast(false);
		}
	}
	else 
	{
		PrintString(FString::Printf(TEXT("OnFindSessionsComplete, Success: %d, Sessions Found: %d"), bWasSuccesful, Results.Num()));
		ServerJoinDel.Broadcast(false);
	}	
}

void UMultiplayerSessionsSubsystem::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	ServerJoinDel.Broadcast(Result == EOnJoinSessionCompleteResult::Success);
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		PrintString(FString::Printf(TEXT("Successfully joined session %s"), *SessionName.ToString()));

		FString Address = "";
		bool Success = SessionInterface->GetResolvedConnectString(SessionName, Address);
		if (Success)
		{
			PrintString(FString::Printf(TEXT("Address: %s"), *Address));
			APlayerController *PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
		else
		{
			PrintString("GetResolvedConnectString Returned False");
		}
	}
	else
	{
		PrintString("OnJoinSessionComplete Failed");
	}
}