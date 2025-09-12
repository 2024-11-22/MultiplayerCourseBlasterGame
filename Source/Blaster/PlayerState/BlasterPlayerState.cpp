// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"

/**
 * 设置需要在网络上复制的属性
 * 
 * @param OutLifetimeProps 输出参数，用于添加需要复制的属性
 */
void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	// 调用父类的实现以确保基础属性也被正确复制
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 将Defeats属性添加到复制列表中
	DOREPLIFETIME(ABlasterPlayerState, Defeats);
	// 将Team属性添加到复制列表中
	DOREPLIFETIME(ABlasterPlayerState, Team);
}

/**
 * 增加玩家分数并更新UI显示
 * 
 * @param ScoreAmount 要增加的分数值
 */
void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
	// 更新玩家分数
	SetScore(GetScore() + ScoreAmount);
	
	// 延迟初始化Character指针
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		// 延迟初始化Controller指针
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			// 更新HUD上显示的分数
			Controller->SetHUDScore(GetScore());
		}
	}
}

/**
 * 当分数在客户端上复制时被调用
 * 更新HUD上显示的分数
 */
void ABlasterPlayerState::OnRep_Score()
{
	// 调用父类的实现
	Super::OnRep_Score();

	// 延迟初始化Character指针
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		// 延迟初始化Controller指针
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			// 更新HUD上显示的分数
			Controller->SetHUDScore(GetScore());
		}
	}
}

/**
 * 增加玩家击败数并更新UI显示
 * 
 * @param DefeatsAmount 要增加的击败数
 */
void ABlasterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	// 更新玩家击败数
	Defeats += DefeatsAmount;
	
	// 延迟初始化Character指针
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		// 延迟初始化Controller指针
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			// 更新HUD上显示的击败数
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

/**
 * 当击败数在客户端上复制时被调用
 * 更新HUD上显示的击败数
 */
void ABlasterPlayerState::OnRep_Defeats()
{
	// 延迟初始化Character指针
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		// 延迟初始化Controller指针
		Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
		{
			// 更新HUD上显示的击败数
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

/**
 * 设置玩家所属的队伍并更新角色颜色
 * 
 * @param TeamToSet 要设置的队伍
 */
void ABlasterPlayerState::SetTeam(ETeam TeamToSet)
{
	// 设置玩家队伍
	Team = TeamToSet;

	// 获取玩家角色并更新其队伍颜色
	ABlasterCharacter* BCharacter = Cast <ABlasterCharacter>(GetPawn());
	if (BCharacter)
	{
		// 设置角色的队伍颜色
		BCharacter->SetTeamColor(Team);
	}
}

/**
 * 当队伍归属在客户端上复制时被调用
 * 更新角色的队伍颜色
 */
void ABlasterPlayerState::OnRep_Team()
{
	// 获取玩家角色并更新其队伍颜色
	ABlasterCharacter* BCharacter = Cast <ABlasterCharacter>(GetPawn());
	if (BCharacter)
	{
		// 设置角色的队伍颜色
		BCharacter->SetTeamColor(Team);
	}
}
