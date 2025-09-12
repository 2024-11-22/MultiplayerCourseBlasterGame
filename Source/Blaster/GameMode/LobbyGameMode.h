// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "LobbyGameMode.generated.h"

/**
 * 大厅游戏模式：管理游戏大厅中的玩家登录和匹配功能
 */
UCLASS()
class BLASTER_API ALobbyGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	/**
	 * 玩家登录后调用：检查玩家数量并根据需要切换到游戏地图
	 * @param NewPlayer 新登录的玩家控制器
	 */
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
};
