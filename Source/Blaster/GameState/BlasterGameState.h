// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "BlasterGameState.generated.h"

/**
 * 游戏状态类：管理游戏中的全局状态信息，包括玩家分数排名和队伍系统
 */
UCLASS()
class BLASTER_API ABlasterGameState : public AGameState
{
	GENERATED_BODY()
public:
	/**
	 * 设置需要在网络上复制的属性
	 * @param OutLifetimeProps 输出参数，用于存储需要复制的属性列表
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	/**
	 * 更新最高分玩家列表
	 * 当玩家得分变化时调用，更新游戏中的最高分记录和玩家排名
	 * @param ScoringPlayer 得分发生变化的玩家状态
	 */
	void UpdateTopScore(class ABlasterPlayerState* ScoringPlayer);

	/** 最高分玩家列表：存储当前游戏中得分最高的玩家 */
	UPROPERTY(Replicated)
	TArray<ABlasterPlayerState*> TopScoringPlayers;

	/** 
	* 队伍相关功能
	*/

	/** 红队得分：红队获得分数时调用 */
	void RedTeamScores();
	/** 蓝队得分：蓝队获得分数时调用 */
	void BlueTeamScores();

	/** 红队玩家列表：存储所有红队玩家的状态 */
	TArray<ABlasterPlayerState*> RedTeam;
	/** 蓝队玩家列表：存储所有蓝队玩家的状态 */
	TArray<ABlasterPlayerState*> BlueTeam;

	/** 红队当前分数：当分数变化时会自动同步到客户端并调用OnRep_RedTeamScore */
	UPROPERTY(ReplicatedUsing = OnRep_RedTeamScore)
	float RedTeamScore = 0.f;

	/** 蓝队当前分数：当分数变化时会自动同步到客户端并调用OnRep_BlueTeamScore */
	UPROPERTY(ReplicatedUsing = OnRep_BlueTeamScore)
	float BlueTeamScore = 0.f;

	/** 红队分数变化时的回调函数：在客户端上更新UI显示 */
	UFUNCTION()
	void OnRep_RedTeamScore();

	/** 蓝队分数变化时的回调函数：在客户端上更新UI显示 */
	UFUNCTION()
	void OnRep_BlueTeamScore();

private:
	/** 当前游戏中的最高个人得分 */
	float TopScore = 0.f;
};
