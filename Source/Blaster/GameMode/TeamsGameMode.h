// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BlasterGameMode.h"
#include "TeamsGameMode.generated.h"

/**
 * 团队游戏模式：管理团队对抗游戏中的玩家分组、分数计算和比赛逻辑
 */
UCLASS()
class BLASTER_API ATeamsGameMode : public ABlasterGameMode
{
	GENERATED_BODY()
public:
	/**
	 * 构造函数：初始化团队游戏模式的基本设置
	 */
	ATeamsGameMode();
	
	/**
	 * 玩家登录后调用：将玩家分配到人数较少的队伍
	 * @param NewPlayer 新登录的玩家控制器
	 */
	virtual void PostLogin(APlayerController* NewPlayer) override;
	
	/**
	 * 玩家退出游戏时调用：从队伍中移除退出的玩家
	 * @param Exiting 退出的玩家控制器
	 */
	virtual void Logout(AController* Exiting) override;
	
	/**
	 * 计算伤害：实现团队模式下的伤害规则（队友之间不造成伤害）
	 * @param Attacker 攻击者控制器
	 * @param Victim 受害者控制器
	 * @param BaseDamage 基础伤害值
	 * @return 最终伤害值
	 */
	virtual float CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage) override;
	
	/**
	 * 玩家被淘汰处理：在基础游戏模式上添加团队得分逻辑
	 * @param ElimmedCharacter 被淘汰的角色
	 * @param VictimController 受害者的控制器
	 * @param AttackerController 攻击者的控制器
	 */
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController) override;
protected:
	/**
	 * 比赛开始时处理：确保所有玩家都被分配到队伍
	 */
	virtual void HandleMatchHasStarted() override;
};
