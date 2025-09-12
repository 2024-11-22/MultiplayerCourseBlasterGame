// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"

/**
 * 设置显示文本内容
 * @param TextToDisplay - 要显示的文本字符串
 */
void UOverheadWidget::SetDisplayText(FString TextToDisplay)
{
	// 检查显示文本组件是否有效
	if (DisplayText)
	{
		// 设置文本内容，将FString转换为FText格式
		DisplayText->SetText(FText::FromString(TextToDisplay));
	}
}

/**
 * 显示玩家的网络角色类型
 * 获取并显示指定pawn的网络角色（Authority、Autonomous Proxy、Simulated Proxy或None）
 * @param InPawn - 要显示网络角色的pawn
 */
void UOverheadWidget::ShowPlayerNetRole(APawn* InPawn)
{
	// 获取pawn的远程网络角色
	ENetRole RemoteRole = InPawn->GetRemoteRole();
	FString Role;
	// 根据网络角色类型设置对应的文本描述
	switch (RemoteRole)
	{
	case ENetRole::ROLE_Authority:
		Role = FString("Authority"); // 权威角色（服务器）
		break;
	case ENetRole::ROLE_AutonomousProxy:
		Role = FString("Autonomous Proxy"); // 自主代理（本地玩家）
		break;
	case ENetRole::ROLE_SimulatedProxy:
		Role = FString("Simulated Proxy"); // 模拟代理（远程玩家）
		break;
	case ENetRole::ROLE_None:
		Role = FString("None"); // 无网络角色
		break;
	}
	// 格式化网络角色显示文本
	FString RemoteRoleString = FString::Printf(TEXT("Remote Role: %s"), *Role);
	// 调用SetDisplayText设置显示内容
	SetDisplayText(RemoteRoleString);
}

/**
 * 当关卡从世界中移除时调用的函数
 * 重写父类方法，实现从父组件移除的逻辑
 * @param InLevel - 被移除的关卡
 * @param InWorld - 关卡所属的世界
 */
void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	// 从父组件移除自身，清理UI引用
	RemoveFromParent();
	// 调用父类的OnLevelRemovedFromWorld完成剩余清理
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}
