// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/GameState/BlasterGameState.h"

namespace MatchState
{
	// 定义冷却状态名称
	const FName Cooldown = FName("Cooldown");
}

/**
 * 构造函数：初始化游戏模式的基本设置
 */
ABlasterGameMode::ABlasterGameMode()
{
	// 设置延迟开始游戏，允许玩家加入
	bDelayedStart = true;
}

/**
 * 游戏开始时调用：初始化关卡开始时间
 */
void ABlasterGameMode::BeginPlay()
{
	// 调用父类的BeginPlay方法
	Super::BeginPlay();

	// 记录关卡开始的世界时间
	LevelStartingTime = GetWorld()->GetTimeSeconds();
}

/**
 * 游戏每帧更新：处理不同比赛状态下的倒计时逻辑
 * @param DeltaTime 帧间隔时间
 */
void ABlasterGameMode::Tick(float DeltaTime)
{
	// 调用父类的Tick方法
	Super::Tick(DeltaTime);

	// 等待开始状态：处理热身倒计时
	if (MatchState == MatchState::WaitingToStart)
	{
		// 计算热身阶段剩余时间
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		// 热身时间结束，开始比赛
		if (CountdownTime <= 0.f)
		{
			StartMatch();
		}
	}
	// 进行中状态：处理比赛倒计时
	else if (MatchState == MatchState::InProgress)
	{
		// 计算比赛阶段剩余时间
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		// 比赛时间结束，进入冷却状态
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	// 冷却状态：处理重置倒计时
	else if (MatchState == MatchState::Cooldown)
	{
		// 计算冷却阶段剩余时间
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		// 冷却时间结束，重新开始游戏
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

/**
 * 比赛状态改变时调用：通知所有玩家控制器比赛状态已改变
 */
void ABlasterGameMode::OnMatchStateSet()
{
	// 调用父类的OnMatchStateSet方法
	Super::OnMatchStateSet();

	// 遍历所有玩家控制器，通知比赛状态变化
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if (BlasterPlayer)
		{
			// 通知玩家控制器当前比赛状态和是否为团队比赛
			BlasterPlayer->OnMatchStateSet(MatchState, bTeamsMatch);
		}
	}
}

/**
 * 计算伤害：基础伤害计算逻辑，可被子类重写以实现不同的伤害规则
 * @param Attacker 攻击者控制器
 * @param Victim 受害者控制器
 * @param BaseDamage 基础伤害值
 * @return 最终伤害值
 */
float ABlasterGameMode::CalculateDamage(AController* Attacker, AController* Victim, float BaseDamage)
{
	// 基础实现：直接返回基础伤害值
	return BaseDamage;
}

/**
 * 玩家被淘汰处理：更新攻击者和受害者的分数，并处理相关视觉反馈
 * @param ElimmedCharacter 被淘汰的角色
 * @param VictimController 受害者的控制器
 * @param AttackerController 攻击者的控制器
 */
void ABlasterGameMode::PlayerEliminated(class ABlasterCharacter* ElimmedCharacter, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	// 验证攻击者和受害者控制器及玩家状态是否有效
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	
	// 获取攻击者和受害者的玩家状态
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	// 获取游戏状态
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();

	// 如果攻击者存在且不是自毁，且游戏状态有效
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && BlasterGameState)
	{
		// 记录当前领先玩家列表
		TArray<ABlasterPlayerState*> PlayersCurrentlyInTheLead;
		for (auto LeadPlayer : BlasterGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}

		// 攻击者得分加1
		AttackerPlayerState->AddToScore(1.f);
		// 更新最高分玩家列表
		BlasterGameState->UpdateTopScore(AttackerPlayerState);
		// 如果攻击者现在处于领先位置，触发获得领先的视觉效果
		if (BlasterGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			ABlasterCharacter* Leader = Cast<ABlasterCharacter>(AttackerPlayerState->GetPawn());
			if (Leader)
			{
				// 多播通知所有客户端显示获得领先效果
				Leader->MulticastGainedTheLead();
			}
		}

		// 检查之前的领先玩家是否不再领先
		for (int32 i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
		{
			if (!BlasterGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))
			{
				ABlasterCharacter* Loser = Cast<ABlasterCharacter>(PlayersCurrentlyInTheLead[i]->GetPawn());
				if (Loser)
				{
					// 多播通知所有客户端显示失去领先效果
					Loser->MulticastLostTheLead();
				}
			}
		}
	}
	// 受害者失败次数加1
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}

	// 处理被淘汰角色的消除效果
	if (ElimmedCharacter)
	{
		// 调用角色的消除方法，false表示不是因为退出游戏
		ElimmedCharacter->Elim(false);
	}

	// 广播淘汰消息给所有玩家
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ABlasterPlayerController* BlasterPlayer = Cast<ABlasterPlayerController>(*It);
		if (BlasterPlayer && AttackerPlayerState && VictimPlayerState)
		{
			// 通知玩家控制器显示淘汰信息
			BlasterPlayer->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}

/**
 * 请求重生：处理被淘汰玩家的重生逻辑
 * @param ElimmedCharacter 被淘汰的角色
 * @param ElimmedController 被淘汰玩家的控制器
 */
void ABlasterGameMode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	// 处理被淘汰角色
	if (ElimmedCharacter)
	{
		// 重置角色状态
		ElimmedCharacter->Reset();
		// 销毁角色
		ElimmedCharacter->Destroy();
	}
	// 处理被淘汰玩家的重生
	if (ElimmedController)
	{
		// 获取所有玩家出生点
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		// 随机选择一个出生点
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		// 在选择的出生点重生玩家
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

/**
 * 玩家离开游戏：处理玩家退出游戏时的逻辑
 * @param PlayerLeaving 离开游戏的玩家状态
 */
void ABlasterGameMode::PlayerLeftGame(ABlasterPlayerState* PlayerLeaving)
{
	// 验证玩家状态是否有效
	if (PlayerLeaving == nullptr) return;
	// 获取游戏状态
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	// 如果离开的玩家是领先玩家，从领先列表中移除
	if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		BlasterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	// 获取离开玩家的角色
	ABlasterCharacter* CharacterLeaving = Cast<ABlasterCharacter>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		// 调用角色的消除方法，true表示是因为退出游戏
		CharacterLeaving->Elim(true);
	}
}