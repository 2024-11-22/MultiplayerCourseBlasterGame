// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Blaster/BlasterTypes/Team.h"
#include "BlasterPlayerState.generated.h"

/**
 * 扩展标准PlayerState以存储玩家的游戏状态信息
 * 包括分数、击败数、队伍归属等数据，并处理这些数据的网络复制
 */
UCLASS()
class BLASTER_API ABlasterPlayerState : public APlayerState
{
	GENERATED_BODY()
public:
	/**
	 * 设置需要在网络上复制的属性
	 * @param OutLifetimeProps 输出参数，用于添加需要复制的属性
	 */
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const override;

	/**
	 * 分数值在客户端上更新时的回调函数
	 * 负责更新UI显示的分数
	 */
	virtual void OnRep_Score() override;

	/**
	 * 击败数在客户端上更新时的回调函数
	 * 负责更新UI显示的击败数
	 */
	UFUNCTION()
	virtual void OnRep_Defeats();

	/**
	 * 增加玩家分数
	 * @param ScoreAmount 要增加的分数值
	 */
	void AddToScore(float ScoreAmount);
	
	/**
	 * 增加玩家击败数
	 * @param DefeatsAmount 要增加的击败数
	 */
	void AddToDefeats(int32 DefeatsAmount);
private:
	/** 指向玩家角色的指针，用于访问角色功能 */
	UPROPERTY()
	class ABlasterCharacter* Character;
	
	/** 指向玩家控制器的指针，用于更新UI显示 */
	UPROPERTY()
	class ABlasterPlayerController* Controller;

	/** 玩家的击败数，会在网络上复制并触发OnRep_Defeats回调 */
	UPROPERTY(ReplicatedUsing = OnRep_Defeats)
	int32 Defeats;

	/** 玩家所属的队伍，会在网络上复制并触发OnRep_Team回调 */
	UPROPERTY(ReplicatedUsing = OnRep_Team)
	ETeam Team = ETeam::ET_NoTeam;

	/**
	 * 队伍归属在客户端上更新时的回调函数
	 * 负责更新玩家角色的队伍颜色
	 */
	UFUNCTION()
	void OnRep_Team();

public:
	/** 获取玩家所属的队伍 */
	FORCEINLINE ETeam GetTeam() const { return Team; }
	
	/**
	 * 设置玩家所属的队伍
	 * @param TeamToSet 要设置的队伍
	 */
	void SetTeam(ETeam TeamToSet);
};
