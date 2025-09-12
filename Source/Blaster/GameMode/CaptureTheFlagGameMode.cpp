// Fill out your copyright notice in the Description page of Project Settings.


#include "CaptureTheFlagGameMode.h"
#include "Blaster/Weapon/Flag.h"
#include "Blaster/CaptureTheFlag/FlagZone.h"
#include "Blaster/GameState/BlasterGameState.h"

/**
 * 处理玩家被淘汰的逻辑
 * @param ElimmedCharacter 被淘汰的角色
 * @param VictimController 受害者的控制器
 * @param AttackerController 攻击者的控制器
 */
void ACaptureTheFlagGameMode::PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	// 调用基类的PlayerEliminated方法处理玩家淘汰逻辑
	ABlasterGameMode::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);
}

/**
 * 处理旗帜被捕获的逻辑
 * @param Flag 被捕获的旗帜
 * @param Zone 捕获旗帜的区域
 */
void ACaptureTheFlagGameMode::FlagCaptured(AFlag* Flag, AFlagZone* Zone)
{
	// 检查是否为有效捕获（旗帜所属队伍与区域所属队伍不同）
	bool bValidCapture = Flag->GetTeam() != Zone->Team;
	// 获取游戏状态
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(GameState);
	if (BGameState)
	{
		// 如果旗帜被捕获到蓝队区域
		if (Zone->Team == ETeam::ET_BlueTeam)
		{
			// 蓝队得分
			BGameState->BlueTeamScores();
		}
		// 如果旗帜被捕获到红队区域
		if (Zone->Team == ETeam::ET_RedTeam)
		{
			// 红队得分
			BGameState->RedTeamScores();
		}
	}
}
