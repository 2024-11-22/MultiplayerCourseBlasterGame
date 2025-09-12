// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"

#include "MultiplayerSessionsSubsystem.generated.h"

//
// 声明自定义委托，供Menu类绑定回调函数
//
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnCreateSessionComplete, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMultiplayerOnFindSessionsComplete, const TArray<FOnlineSessionSearchResult>& SessionResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_OneParam(FMultiplayerOnJoinSessionComplete, EOnJoinSessionCompleteResult::Type Result);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnDestroySessionComplete, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMultiplayerOnStartSessionComplete, bool, bWasSuccessful);

/**
 * 多人会话子系统类，负责处理多人游戏会话的创建、查找、加入、销毁和启动等功能
 */
UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	/** 构造函数 */
	UMultiplayerSessionsSubsystem();

	//
	// 处理会话功能的方法，Menu类将调用这些方法
	//
	/** 创建一个多人游戏会话 */
	void CreateSession(int32 NumPublicConnections, FString MatchType);
	/** 查找可用的多人游戏会话 */
	void FindSessions(int32 MaxSearchResults);
	/** 加入指定的游戏会话 */
	void JoinSession(const FOnlineSessionSearchResult& SessionResult);
	/** 销毁当前游戏会话 */
	void DestroySession();
	/** 开始游戏会话 */
	void StartSession();

	//
	// 自定义委托实例，供Menu类绑定回调函数
	//
	/** 会话创建完成时触发的委托 */
	FMultiplayerOnCreateSessionComplete MultiplayerOnCreateSessionComplete;
	/** 会话查找完成时触发的委托 */
	FMultiplayerOnFindSessionsComplete MultiplayerOnFindSessionsComplete;
	/** 加入会话完成时触发的委托 */
	FMultiplayerOnJoinSessionComplete MultiplayerOnJoinSessionComplete;
	/** 会话销毁完成时触发的委托 */
	FMultiplayerOnDestroySessionComplete MultiplayerOnDestroySessionComplete;
	/** 会话开始完成时触发的委托 */
	FMultiplayerOnStartSessionComplete MultiplayerOnStartSessionComplete;

	/** 期望的公共连接数量 */
	int32 DesiredNumPublicConnections{};
	/** 期望的匹配类型 */
	FString DesiredMatchType{};
protected:

	//
	// 内部回调函数，用于在线会话接口委托列表
	// 这些方法不需要在类外部调用
	//
	/** 会话创建完成的内部回调 */
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	/** 会话查找完成的内部回调 */
	void OnFindSessionsComplete(bool bWasSuccessful);
	/** 加入会话完成的内部回调 */
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	/** 会话销毁完成的内部回调 */
	void OnDestroySessionComplete(FName SessionName, bool bWasSuccessful);
	/** 会话开始完成的内部回调 */
	void OnStartSessionComplete(FName SessionName, bool bWasSuccessful);

private:
	/** 会话接口指针，用于与UE的在线子系统交互 */
	IOnlineSessionPtr SessionInterface;
	/** 存储上一次的会话设置 */
	TSharedPtr<FOnlineSessionSettings> LastSessionSettings;
	/** 存储上一次的会话搜索结果 */
	TSharedPtr<FOnlineSessionSearch> LastSessionSearch;

	//
	// 在线会话接口委托列表的委托实例
	// 我们将把MultiplayerSessionsSubsystem的内部回调函数绑定到这些委托
	//
	/** 创建会话完成的委托 */
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	/** 创建会话完成委托的句柄 */
	FDelegateHandle CreateSessionCompleteDelegateHandle;
	/** 查找会话完成的委托 */
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	/** 查找会话完成委托的句柄 */
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	/** 加入会话完成的委托 */
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	/** 加入会话完成委托的句柄 */
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	/** 销毁会话完成的委托 */
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;
	/** 销毁会话完成委托的句柄 */
	FDelegateHandle DestroySessionCompleteDelegateHandle;
	/** 开始会话完成的委托 */
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	/** 开始会话完成委托的句柄 */
	FDelegateHandle StartSessionCompleteDelegateHandle;

	/** 标记在销毁会话后是否需要创建新会话 */
	bool bCreateSessionOnDestroy{ false };
	/** 上一次的公共连接数量 */
	int32 LastNumPublicConnections;
	/** 上一次的匹配类型 */
	FString LastMatchType;
};
