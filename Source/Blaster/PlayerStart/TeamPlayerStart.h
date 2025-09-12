// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerStart.h"
#include "Blaster/BlasterTypes/Team.h"
#include "TeamPlayerStart.generated.h"

/**
 * 扩展标准PlayerStart以支持团队游戏模式
 * 允许为特定队伍的玩家指定专属出生点
 */
UCLASS()
class BLASTER_API ATeamPlayerStart : public APlayerStart
{
	GENERATED_BODY()
public:
	/** 此出生点所属的队伍，用于游戏模式中为玩家分配对应的出生点 */
	UPROPERTY(EditAnywhere, Category = "Team Player Start", BlueprintReadWrite, meta = (ExposeOnSpawn = "true"))
	ETeam Team;
};
