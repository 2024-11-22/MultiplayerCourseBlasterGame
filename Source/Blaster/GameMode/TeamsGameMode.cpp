// Fill out your copyright notice in the Description page of Project Settings.


#include "TeamsGameMode.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"

/**
 * 构造函数：初始化团队游戏模式的基本设置
 */
ATeamsGameMode::ATeamsGameMode()
{
	// 设置为团队比赛模式
	bTeamsMatch = true;
}

/**
 * 玩家登录后调用：将玩家分配到人数较少的队伍
 * @param NewPlayer 新登录的玩家控制器
 */
void ATeamsGameMode::PostLogin(APlayerController* NewPlayer)
{
	// 调用父类的PostLogin方法
	Super::PostLogin(NewPlayer);

	// 获取游戏状态
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		// 获取新玩家的玩家状态
		ABlasterPlayerState* BPState = NewPlayer->GetPlayerState<ABlasterPlayerState>();
		// 如果玩家还没有分配队伍
		if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
		{
			// 根据队伍人数将玩家分配到人数较少的队伍
			if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
			{
				// 蓝队人数多于或等于红队，将玩家加入红队
				BGameState->RedTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_RedTeam);
			}
			else
			{
				// 红队人数多于蓝队，将玩家加入蓝队
				BGameState->BlueTeam.AddUnique(BPState);
				BPState->SetTeam(ETeam::ET_BlueTeam);
			}
		}
	}
}

/**
 * 玩家退出游戏时调用：从队伍中移除退出的玩家
 * @param Exiting 退出的玩家控制器
 */
void ATeamsGameMode::Logout(AController* Exiting)
{
	// 获取游戏状态
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	// 获取退出玩家的玩家状态
	ABlasterPlayerState* BPState = Exiting->GetPlayerState<ABlasterPlayerState>();
	if (BGameState && BPState)
	{
		// 从红队中移除玩家（如果在红队中）
		if (BGameState->RedTeam.Contains(BPState))
		{
			BGameState->RedTeam.Remove(BPState);
		}
		// 从蓝队中移除玩家（如果在蓝队中）
		if (BGameState->BlueTeam.Contains(BPState))
		{
			BGameState->BlueTeam.Remove(BPState);
		}
	}

}

/**
 * 比赛开始时处理：确保所有玩家都被分配到队伍
 */
void ATeamsGameMode::HandleMatchHasStarted()
{
	// 调用父类的HandleMatchHasStarted方法
	Super::HandleMatchHasStarted();

	// 获取游戏状态
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	if (BGameState)
	{
		// 遍历所有玩家
		for (auto PState : BGameState->PlayerArray)
		{
			// 将玩家状态转换为BlasterPlayerState
			ABlasterPlayerState* BPState = Cast<ABlasterPlayerState>(PState.Get());
			// 如果玩家还没有分配队伍
			if (BPState && BPState->GetTeam() == ETeam::ET_NoTeam)
			{
				// 根据队伍人数将玩家分配到人数较少的队伍
				if (BGameState->BlueTeam.Num() >= BGameState->RedTeam.Num())
				{
					// 蓝队人数多于或等于红队，将玩家加入红队
					BGameState->RedTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_RedTeam);
				}
				else
				{
					// 红队人数多于蓝队，将玩家加入蓝队
					BGameState->BlueTeam.AddUnique(BPState);
					BPState->SetTeam(ETeam::ET_BlueTeam);
				}
			}
		}
	}
}

/**
 * 计算伤害：实现团队模式下的伤害规则（队友之间不造成伤害）
 * @param Attacker 攻击者控制器
 * @param Victim 受害者控制器
 * @param BaseDamage 基础伤害值
 * @return 最终伤害值
 */
float ATeamsGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	// 获取攻击者和受害者的玩家状态
	ABlasterPlayerState* AttackerPState = Attacker->GetPlayerState<ABlasterPlayerState>();
	ABlasterPlayerState* VictimPState = Victim->GetPlayerState<ABlasterPlayerState>();
	// 如果任何一方的玩家状态无效，返回基础伤害
	if (AttackerPState == nullptr || VictimPState == nullptr) return BaseDamage;
	// 如果是自我伤害，返回基础伤害
	if (VictimPState == AttackerPState)
	{
		return BaseDamage;
	}
	// 如果攻击者和受害者是同一队伍，返回0伤害（队友之间不造成伤害）
	if (AttackerPState->GetTeam() == VictimPState->GetTeam())
	{
		return 0.f;
	}
	// 不同队伍之间造成正常伤害
	return BaseDamage;
}

/**
 * 玩家被淘汰处理：在基础游戏模式上添加团队得分逻辑
 * @param ElimmedCharacter 被淘汰的角色
 * @param VictimController 受害者的控制器
 * @param AttackerController 攻击者的控制器
 */
void ATeamsGameMode::PlayerEliminated(ABlasterCharacter* ElimmedCharacter, ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	// 调用父类的PlayerEliminated方法，处理基本逻辑
	Super::PlayerEliminated(ElimmedCharacter, VictimController, AttackerController);

	// 获取游戏状态
	ABlasterGameState* BGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
	// 获取攻击者的玩家状态
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	if (BGameState && AttackerPlayerState)
	{
		// 根据攻击者的队伍为相应队伍加分
		if (AttackerPlayerState->GetTeam() == ETeam::ET_BlueTeam)
		{
			// 蓝队得分
			BGameState->BlueTeamScores();
		}
		if (AttackerPlayerState->GetTeam() == ETeam::ET_RedTeam)
		{
			// 红队得分
			BGameState->RedTeamScores();
		}
	}
}