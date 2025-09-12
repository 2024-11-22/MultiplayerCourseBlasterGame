// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "MultiplayerSessionsSubsystem.h"

/**
 * 玩家登录后调用：检查玩家数量并根据需要切换到游戏地图
 * @param NewPlayer 新登录的玩家控制器
 */
void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	// 调用父类的PostLogin方法
	Super::PostLogin(NewPlayer);

	// 获取当前游戏中的玩家数量
	int32 NumberOfPlayers = GameState.Get()->PlayerArray.Num();

	// 获取游戏实例
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		// 获取多人会话子系统
		UMultiplayerSessionsSubsystem* Subsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		// 确保子系统存在
		check(Subsystem);

		// 如果玩家数量达到期望的公共连接数，则开始游戏
		if (NumberOfPlayers == Subsystem->DesiredNumPublicConnections)
		{
			// 获取当前世界
			UWorld* World = GetWorld();
			if (World)
			{
				// 启用无缝旅行
				bUseSeamlessTravel = true;

				// 获取期望的匹配类型
				FString MatchType = Subsystem->DesiredMatchType;
				// 根据匹配类型选择相应的地图
				if (MatchType == "FreeForAll")
				{
					// 前往自由对战模式地图
					World->ServerTravel(FString("/Game/Maps/BlasterMap?listen"));
				}
				else if (MatchType == "Teams")
				{
					// 前往团队模式地图
					World->ServerTravel(FString("/Game/Maps/Teams?listen"));
				}
				else if (MatchType == "CaptureTheFlag")
				{
					// 前往夺旗模式地图
					World->ServerTravel(FString("/Game/Maps/CaptureTheFlag?listen"));
				}
			}
		}
	}
}