// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TeamsGameMode.h"
#include "CaptureTheFlagGameMode.generated.h"

/**
 * 夺旗游戏模式：管理夺旗游戏中的旗帜捕获机制和得分逻辑
 */
UCLASS()
class BLASTER_API ACaptureTheFlagGameMode : public ATeamsGameMode
{
	GENERATED_BODY()
public:
	/**
	 * 处理玩家被淘汰的逻辑
	 * @param ElimmedCharacter 被淘汰的角色
	 * @param VictimController 受害者的控制器
	 * @param AttackerController 攻击者的控制器
	 */
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController) override;
	
	/**
	 * 处理旗帜被捕获的逻辑
	 * @param Flag 被捕获的旗帜
	 * @param Zone 捕获旗帜的区域
	 */
	void FlagCaptured(class AFlag* Flag, class AFlagZone* Zone);
};
