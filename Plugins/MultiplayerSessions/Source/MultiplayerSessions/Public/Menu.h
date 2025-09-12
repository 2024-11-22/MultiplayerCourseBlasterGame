// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Menu.generated.h"

/**
 * 多人游戏菜单用户界面类，负责处理主机创建、加入游戏等功能
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	/**
	 * 设置菜单的初始状态
	 * 
	 * @param NumberOfPublicConnections 公共连接数量，默认为4
	 * @param TypeOfMatch 匹配类型，默认为"FreeForAll"（自由混战）
	 * @param LobbyPath 大厅地图路径，默认为"/Game/ThirdPersonCPP/Maps/Lobby"
	 */
	UFUNCTION(BlueprintCallable)
	void MenuSetup(int32 NumberOfPublicConnections = 4, FString TypeOfMatch = FString(TEXT("FreeForAll")), FString LobbyPath = FString(TEXT("/Game/ThirdPersonCPP/Maps/Lobby")));

protected:

	/**
	 * 初始化菜单组件和绑定事件
	 * 
	 * @return 初始化是否成功
	 */
	virtual bool Initialize() override;
	
	/**
	 * 当关卡从世界中移除时调用
	 * 
	 * @param InLevel 被移除的关卡
	 * @param InWorld 关卡所在的世界
	 */
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

	//
	// 自定义委托的回调函数（在MultiplayerSessionsSubsystem中定义）
	//
	/**
	 * 创建会话完成的回调函数
	 * 
	 * @param bWasSuccessful 创建是否成功
	 */
	UFUNCTION()
	void OnCreateSession(bool bWasSuccessful);
	
	/**
	 * 查找会话完成的回调函数
	 * 
	 * @param SessionResults 找到的会话结果列表
	 * @param bWasSuccessful 查找是否成功
	 */
	void OnFindSessions(const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
	
	/**
	 * 加入会话完成的回调函数
	 * 
	 * @param Result 加入会话的结果类型
	 */
	void OnJoinSession(EOnJoinSessionCompleteResult::Type Result);
	
	/**
	 * 销毁会话完成的回调函数
	 * 
	 * @param bWasSuccessful 销毁是否成功
	 */
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);
	
	/**
	 * 开始会话完成的回调函数
	 * 
	 * @param bWasSuccessful 开始是否成功
	 */
	UFUNCTION()
	void OnStartSession(bool bWasSuccessful);

private:

	/** 主机按钮引用 */
	UPROPERTY(meta = (BindWidget))
	class UButton* HostButton;

	/** 加入按钮引用 */
	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	/**
	 * 主机按钮点击事件处理函数
	 */
	UFUNCTION()
	void HostButtonClicked();

	/**
	 * 加入按钮点击事件处理函数
	 */
	UFUNCTION()
	void JoinButtonClicked();

	/**
	 * 清理菜单资源和委托绑定
	 */
	void MenuTearDown();

	/** 负责处理所有在线会话功能的子系统 */
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;
	
	/** 公共连接数量 */
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int32 NumPublicConnections{4};

	/** 匹配类型 */
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FString MatchType{TEXT("FreeForAll")};

	/** 大厅地图路径 */
	FString PathToLobby{TEXT("")};
};
