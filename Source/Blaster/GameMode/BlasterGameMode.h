// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGameMode.generated.h"

namespace MatchState
{
	// 冷却状态：比赛时间结束后，显示胜利者并开始冷却计时器
	extern BLASTER_API const FName Cooldown; 
}

/**
 * 游戏模式基类：管理游戏流程、玩家消除、重生以及比赛状态
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	/**
	 * 构造函数：初始化游戏模式的基本设置
	 */
	ABlasterGameMode();
	
	/**
	 * 游戏每帧更新：处理不同比赛状态下的倒计时逻辑
	 * @param DeltaTime 帧间隔时间
	 */
	virtual void Tick(float DeltaTime) override;
	
	/**
	 * 玩家被淘汰处理：更新攻击者和受害者的分数，并处理相关视觉反馈
	 * @param ElimmedCharacter 被淘汰的角色
	 * @param VictimController 受害者的控制器
	 * @param AttackerController 攻击者的控制器
	 */
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
	
	/**
	 * 请求重生：处理被淘汰玩家的重生逻辑
	 * @param ElimmedCharacter 被淘汰的角色
	 * @param ElimmedController 被淘汰玩家的控制器
	 */
	virtual void RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController);
	
	/**
	 * 玩家离开游戏：处理玩家退出游戏时的逻辑
	 * @param PlayerLeaving 离开游戏的玩家状态
	 */
	void PlayerLeftGame(class ABlasterPlayerState* PlayerLeaving);
	
	/**
	 * 计算伤害：基础伤害计算逻辑，可被子类重写以实现不同的伤害规则
	 * @param Attacker 攻击者控制器
	 * @param Victim 受害者控制器
	 * @param BaseDamage 基础伤害值
	 * @return 最终伤害值
	 */
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage);
	
	/**
	 * 热身时间：比赛开始前的准备时间（秒）
	 */
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;

	/**
	 * 比赛时间：正式比赛阶段的持续时间（秒）
	 */
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	/**
	 * 冷却时间：比赛结束后到重置游戏的等待时间（秒）
	 */
	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;

	/**
	 * 关卡开始时间：记录关卡开始的世界时间
	 */
	float LevelStartingTime = 0.f;

	/**
	 * 是否为团队比赛模式：标记当前比赛是否为团队对抗模式
	 */
	bool bTeamsMatch = false;
protected:
	/**
	 * 游戏开始时调用：初始化关卡开始时间
	 */
	virtual void BeginPlay() override;
	
	/**
	 * 比赛状态改变时调用：通知所有玩家控制器比赛状态已改变
	 */
	virtual void OnMatchStateSet() override;

private:
	/**
	 * 倒计时时间：当前比赛阶段的剩余时间
	 */
	float CountdownTime = 0.f;
public:
	/**
	 * 获取当前倒计时时间
	 * @return 当前剩余倒计时时间
	 */
	FORCEINLINE float GetCountdownTime() const { return CountdownTime; }
};
