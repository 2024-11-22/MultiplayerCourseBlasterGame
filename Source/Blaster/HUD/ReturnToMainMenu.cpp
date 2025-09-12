// Fill out your copyright notice in the Description page of Project Settings.


#include "ReturnToMainMenu.h"
#include "GameFramework/PlayerController.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "GameFramework/GameModeBase.h"
#include "Blaster/Character/BlasterCharacter.h"

/**
 * 设置菜单：初始化菜单组件和功能
 * 将菜单添加到视口，设置可见性，配置输入模式，并绑定按钮事件和会话事件
 */
void UReturnToMainMenu::MenuSetup()
{
	// 将菜单添加到视口
	AddToViewport();
	// 设置菜单为可见状态
	SetVisibility(ESlateVisibility::Visible);
	// 允许菜单获取焦点
	bIsFocusable = true;

	// 获取当前世界上下文
	UWorld* World = GetWorld();
	if (World)
	{
		// 获取第一个玩家控制器（如果还未设置）
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			// 设置输入模式为游戏和UI混合模式
			FInputModeGameAndUI InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget());
			PlayerController->SetInputMode(InputModeData);
			// 显示鼠标光标
			PlayerController->SetShowMouseCursor(true);
		}
	}
	// 绑定返回按钮的点击事件（如果尚未绑定）
	if (ReturnButton && !ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.AddDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
	}
	// 获取游戏实例
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		// 获取多人会话子系统
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
		if (MultiplayerSessionsSubsystem)
		{
			// 绑定会话销毁完成事件
			MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &UReturnToMainMenu::OnDestroySession);
		}
	}
}

/**
 * 初始化UI组件
 * 重写父类方法，进行UI组件的初始化
 * @return 初始化是否成功
 */
bool UReturnToMainMenu::Initialize()
{
	// 调用父类的Initialize方法
	if (!Super::Initialize())
	{
		return false;
	}

	return true;
}

/**
 * 销毁会话回调函数
 * 当会话销毁时调用，处理会话销毁后的逻辑
 * @param bWasSuccessful 会话销毁是否成功
 */
void UReturnToMainMenu::OnDestroySession(bool bWasSuccessful)
{
	// 如果会话销毁失败，重新启用返回按钮
	if (!bWasSuccessful)
	{
		ReturnButton->SetIsEnabled(true);
		return;
	}

	// 获取当前世界上下文
	UWorld* World = GetWorld();
	if (World)
	{
		// 尝试获取权威游戏模式（服务器端）
		AGameModeBase* GameMode = World->GetAuthGameMode<AGameModeBase>();
		if (GameMode)
		{
			// 如果是服务器端，调用游戏模式的返回主菜单方法
			GameMode->ReturnToMainMenuHost();
		}
		else
		{
			// 如果是客户端，获取玩家控制器
			PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
			if (PlayerController)
			{
				// 客户端返回主菜单
				PlayerController->ClientReturnToMainMenuWithTextReason(FText());
			}
		}
	}
}

/**
 * 关闭菜单：清理菜单资源和取消注册的事件
 * 从父组件移除，恢复游戏输入模式，并解除事件绑定
 */
void UReturnToMainMenu::MenuTearDown()
{
	// 从父组件移除菜单
	RemoveFromParent();
	// 获取当前世界上下文
	UWorld* World = GetWorld();
	if (World)
	{
		// 获取第一个玩家控制器（如果还未设置）
		PlayerController = PlayerController == nullptr ? World->GetFirstPlayerController() : PlayerController;
		if (PlayerController)
		{
			// 设置输入模式为仅游戏
			FInputModeGameOnly InputModeData;
			PlayerController->SetInputMode(InputModeData);
			// 隐藏鼠标光标
			PlayerController->SetShowMouseCursor(false);
		}
	}
	// 解除返回按钮的点击事件绑定（如果已绑定）
	if (ReturnButton && ReturnButton->OnClicked.IsBound())
	{
		ReturnButton->OnClicked.RemoveDynamic(this, &UReturnToMainMenu::ReturnButtonClicked);
	}
	// 解除会话销毁完成事件绑定（如果已绑定）
	if (MultiplayerSessionsSubsystem && MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.IsBound())
	{
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.RemoveDynamic(this, &UReturnToMainMenu::OnDestroySession);
	}
}

/**
 * 返回按钮点击事件处理函数
 * 处理返回按钮点击事件，禁用按钮并调用角色的离开游戏逻辑
 */
void UReturnToMainMenu::ReturnButtonClicked()
{
	// 禁用返回按钮，防止重复点击
	ReturnButton->SetIsEnabled(false);

	// 获取当前世界上下文
	UWorld* World = GetWorld();
	if (World)
	{
		// 获取第一个玩家控制器
		APlayerController* FirstPlayerController = World->GetFirstPlayerController();
		if (FirstPlayerController)
		{
			// 尝试获取玩家角色
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FirstPlayerController->GetPawn());
			if (BlasterCharacter)
			{
				// 如果有角色，调用角色的ServerLeaveGame方法（RPC到服务器）
				BlasterCharacter->ServerLeaveGame();
				// 绑定角色离开游戏事件
				BlasterCharacter->OnLeftGame.AddDynamic(this, &UReturnToMainMenu::OnPlayerLeftGame);
			}
			else
			{
				// 如果没有角色，重新启用返回按钮
				ReturnButton->SetIsEnabled(true);
			}
		}
	}
}

/**
 * 玩家离开游戏后的回调函数
 * 当玩家离开游戏时调用，销毁当前会话
 */
void UReturnToMainMenu::OnPlayerLeftGame()
{
	// 输出日志，标记进入函数
	UE_LOG(LogTemp, Warning, TEXT("OnPlayerLeftGame()"))
		// 检查多人会话子系统是否有效
		if (MultiplayerSessionsSubsystem)
		{
			// 输出日志，标记子系统有效
			UE_LOG(LogTemp, Warning, TEXT("MultiplayerSessionsSubsystem valid"))
				// 销毁当前会话
				MultiplayerSessionsSubsystem->DestroySession();
		}
}
