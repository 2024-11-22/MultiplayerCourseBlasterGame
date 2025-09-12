// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameState.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"

/**
 * 设置需要在网络上复制的属性
 * 重写父类方法，添加本类特有的需要网络同步的属性
 * @param OutLifetimeProps 输出参数，用于存储需要复制的属性列表
 */
void ABlasterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	// 调用父类方法，复制父类中需要同步的属性
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 添加最高分玩家列表到网络复制属性
	DOREPLIFETIME(ABlasterGameState, TopScoringPlayers);
	// 添加红队分数到网络复制属性
	DOREPLIFETIME(ABlasterGameState, RedTeamScore);
	// 添加蓝队分数到网络复制属性
	DOREPLIFETIME(ABlasterGameState, BlueTeamScore);
}

/**
 * 更新最高分玩家列表
 * 当玩家得分变化时调用，更新游戏中的最高分记录和玩家排名
 * @param ScoringPlayer 得分发生变化的玩家状态
 */
void ABlasterGameState::UpdateTopScore(class ABlasterPlayerState* ScoringPlayer)
{
	// 如果最高分玩家列表为空，则直接添加当前玩家
	if (TopScoringPlayers.Num() == 0)
	{
		TopScoringPlayers.Add(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
	// 如果当前玩家得分与最高分相同，则添加到最高分玩家列表（避免重复）
	else if (ScoringPlayer->GetScore() == TopScore)
	{
		TopScoringPlayers.AddUnique(ScoringPlayer);
	}
	// 如果当前玩家得分超过最高分，则清空列表并只添加当前玩家
	else if (ScoringPlayer->GetScore() > TopScore)
	{
		TopScoringPlayers.Empty();
		TopScoringPlayers.AddUnique(ScoringPlayer);
		TopScore = ScoringPlayer->GetScore();
	}
}

/**
 * 红队得分
 * 增加红队分数，并更新UI显示
 */
void ABlasterGameState::RedTeamScores()
{
	// 红队分数加1
	++RedTeamScore;
	// 获取第一个玩家控制器，用于更新UI
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		// 更新玩家HUD上显示的红队分数
		BPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

/**
 * 蓝队得分
 * 增加蓝队分数，并更新UI显示
 */
void ABlasterGameState::BlueTeamScores()
{
	// 蓝队分数加1
	++BlueTeamScore;
	// 获取第一个玩家控制器，用于更新UI
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		// 更新玩家HUD上显示的蓝队分数
		BPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}

/**
 * 红队分数变化时的网络复制回调
 * 在客户端上接收到红队分数更新时调用，更新UI显示
 */
void ABlasterGameState::OnRep_RedTeamScore()
{
	// 获取第一个玩家控制器，用于更新UI
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		// 更新玩家HUD上显示的红队分数
		BPlayer->SetHUDRedTeamScore(RedTeamScore);
	}
}

/**
 * 蓝队分数变化时的网络复制回调
 * 在客户端上接收到蓝队分数更新时调用，更新UI显示
 */
void ABlasterGameState::OnRep_BlueTeamScore()
{
	// 获取第一个玩家控制器，用于更新UI
	ABlasterPlayerController* BPlayer = Cast<ABlasterPlayerController>(GetWorld()->GetFirstPlayerController());
	if (BPlayer)
	{
		// 更新玩家HUD上显示的蓝队分数
		BPlayer->SetHUDBlueTeamScore(BlueTeamScore);
	}
}
