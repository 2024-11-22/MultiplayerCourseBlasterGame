// Fill out your copyright notice in the Description page of Project Settings.


#include "Menu.h"
#include "Components/Button.h"
#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"

/**
 * 设置菜单的初始状态
 * 
 * @param NumberOfPublicConnections 公共连接数量
 * @param TypeOfMatch 匹配类型
 * @param LobbyPath 大厅地图路径
 */
void UMenu::MenuSetup(int32 NumberOfPublicConnections, FString TypeOfMatch, FString LobbyPath)
{
	// 格式化大厅路径，添加listen参数使服务器可被加入
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);
	NumPublicConnections = NumberOfPublicConnections;
	MatchType = TypeOfMatch;
	// 添加到视口并设置可见性
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;

	// 设置UI输入模式，让玩家可以操作菜单
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeUIOnly InputModeData;
			InputModeData.SetWidgetToFocus(TakeWidget()); // 设置焦点到菜单
			InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // 不锁定鼠标
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(true); // 显示鼠标光标
		}
	}

	// 获取会话子系统
	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	// 绑定会话子系统的委托到菜单的回调函数
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MultiplayerOnCreateSessionComplete.AddDynamic(this, &ThisClass::OnCreateSession);
		MultiplayerSessionsSubsystem->MultiplayerOnFindSessionsComplete.AddUObject(this, &ThisClass::OnFindSessions);
		MultiplayerSessionsSubsystem->MultiplayerOnJoinSessionComplete.AddUObject(this, &ThisClass::OnJoinSession);
		MultiplayerSessionsSubsystem->MultiplayerOnDestroySessionComplete.AddDynamic(this, &ThisClass::OnDestroySession);
		MultiplayerSessionsSubsystem->MultiplayerOnStartSessionComplete.AddDynamic(this, &ThisClass::OnStartSession);
	}
}

/**
 * 初始化菜单组件和绑定事件
 * 
 * @return 初始化是否成功
 */
bool UMenu::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	// 绑定按钮点击事件
	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::HostButtonClicked);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::JoinButtonClicked);
	}

	return true;
}

/**
 * 当关卡从世界中移除时调用
 * 
 * @param InLevel 被移除的关卡
 * @param InWorld 关卡所在的世界
 */
void UMenu::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	// 清理菜单资源
	MenuTearDown();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

/**
 * 创建会话完成的回调函数
 * 
 * @param bWasSuccessful 创建是否成功
 */
void UMenu::OnCreateSession(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		// 创建成功，切换到游戏地图（大厅）
		UWorld* World = GetWorld();
		if (World)
		{
			World->ServerTravel(PathToLobby);
		}
	}
	else
	{
		// 创建失败，显示错误信息并重新启用按钮
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString(TEXT("Failed to create session!"))
			);
		}
		HostButton->SetIsEnabled(true);
	}
}

/**
 * 查找会话完成的回调函数
 * 
 * @param SessionResults 找到的会话结果列表
 * @param bWasSuccessful 查找是否成功
 */
void UMenu::OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful)
{
	if (MultiplayerSessionsSubsystem == nullptr)
	{
		return;
	}

	// 遍历找到的会话
	for (auto Result : SessionResults)
	{
		FString SettingsValue; // 存储匹配类型
		Result.Session.SessionSettings.Get(FName("MatchType"), SettingsValue);
		
		// 检查匹配类型是否符合要求
		if (SettingsValue == MatchType)
		{
			// 找到匹配的会话，尝试加入
			MultiplayerSessionsSubsystem->JoinSession(Result);
			return;
		}
	}
	
	// 没有找到匹配的会话或查找失败，重新启用按钮
	if (!bWasSuccessful || SessionResults.Num() == 0)
	{
		JoinButton->SetIsEnabled(true);
	}
}

/**
 * 加入会话完成的回调函数
 * 
 * @param Result 加入会话的结果类型
 */
void UMenu::OnJoinSession(EOnJoinSessionCompleteResult::Type Result)
{
	// 获取在线子系统和会话接口
	IOnlineSubsystem* Subsystem = IOnlineSubsystem::Get();
	if (Subsystem)
	{
		IOnlineSessionPtr SessionInterface = Subsystem->GetSessionInterface();
		if (SessionInterface.IsValid())
		{
			FString Address; // 存储会话地址
			// 获取解析后的连接字符串
			SessionInterface->GetResolvedConnectString(NAME_GameSession, Address);

			APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
			if (PlayerController)
			{
				// 客户端旅行到会话地址
				PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
			}
		}
	}
	
	// 如果加入失败，重新启用按钮
	if (Result != EOnJoinSessionCompleteResult::Success)
	{
		JoinButton->SetIsEnabled(true);
	}
}

/**
 * 销毁会话完成的回调函数
 * 
 * @param bWasSuccessful 销毁是否成功
 */
void UMenu::OnDestroySession(bool bWasSuccessful)
{
	// 销毁会话完成的处理逻辑可以在这里添加
}

/**
 * 开始会话完成的回调函数
 * 
 * @param bWasSuccessful 开始是否成功
 */
void UMenu::OnStartSession(bool bWasSuccessful)
{
	// 开始会话完成的处理逻辑可以在这里添加
}

/**
 * 主机按钮点击事件处理函数
 */
void UMenu::HostButtonClicked()
{
	// 禁用按钮防止重复点击
	HostButton->SetIsEnabled(false);
	// 通过会话子系统创建会话
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->CreateSession(NumPublicConnections, MatchType);
	}
}

/**
 * 加入按钮点击事件处理函数
 */
void UMenu::JoinButtonClicked()
{
	// 禁用按钮防止重复点击
	JoinButton->SetIsEnabled(false);
	// 通过会话子系统查找会话
	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->FindSessions(10000); // 最多查找10000个会话
	}
}

/**
 * 清理菜单资源和委托绑定
 */
void UMenu::MenuTearDown()
{
	// 从父组件中移除菜单
	RemoveFromParent();
	// 恢复游戏输入模式
	UWorld* World = GetWorld();
	if (World)
	{
		APlayerController* PlayerController = World->GetFirstPlayerController();
		if (PlayerController)
		{
			FInputModeGameOnly InputModeData; // 仅游戏输入模式
			PlayerController->SetInputMode(InputModeData);
			PlayerController->SetShowMouseCursor(false); // 隐藏鼠标光标
		}
	}
}
