// Copyright 2023 Blaster Game. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 高延迟警告委托，当玩家延迟过高时触发
 * @param bPingTooHigh - 是否延迟过高
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FHighPingDelegate, bool, bPingTooHigh);

/**
 * 扩展了标准PlayerController，处理游戏内玩家控制、HUD显示和网络同步等核心功能
 * 负责同步服务器时间、显示玩家状态信息、处理高延迟警告、显示游戏公告等
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	/**
	 * 设置HUD上显示的健康值
	 * @param Health - 当前健康值
	 * @param MaxHealth - 最大健康值
	 */
	void SetHUDHealth(float Health, float MaxHealth);
	
	/**
	 * 设置HUD上显示的护盾值
	 * @param Shield - 当前护盾值
	 * @param MaxShield - 最大护盾值
	 */
	void SetHUDShield(float Shield, float MaxShield);
	
	/**
	 * 设置HUD上显示的分数
	 * @param Score - 当前分数
	 */
	void SetHUDScore(float Score);
	
	/**
	 * 设置HUD上显示的击败数
	 * @param Defeats - 当前击败数
	 */
	void SetHUDDefeats(int32 Defeats);
	
	/**
	 * 设置HUD上显示的武器弹药数量
	 * @param Ammo - 当前武器弹药数量
	 */
	void SetHUDWeaponAmmo(int32 Ammo);
	
	/**
	 * 设置HUD上显示的携带弹药数量
	 * @param Ammo - 当前携带弹药数量
	 */
	void SetHUDCarriedAmmo(int32 Ammo);
	
	/**
	 * 设置HUD上显示的比赛倒计时
	 * @param CountdownTime - 倒计时时间（秒）
	 */
	void SetHUDMatchCountdown(float CountdownTime);
	
	/**
	 * 设置HUD上显示的公告倒计时
	 * @param CountdownTime - 公告倒计时时间（秒）
	 */
	void SetHUDAnnouncementCountdown(float CountdownTime);
	
	/**
	 * 设置HUD上显示的手榴弹数量
	 * @param Grenades - 当前手榴弹数量
	 */
	void SetHUDGrenades(int32 Grenades);
	/**
	 * 当控制器获得对一个Pawn的控制权时调用
	 * @param InPawn - 被控制的Pawn
	 */
	virtual void OnPossess(APawn* InPawn) override;
	
	/**
	 * 每帧更新函数，处理HUD时间更新、时间同步检查、初始化轮询和延迟检查
	 * @param DeltaTime - 帧时间
	 */
	virtual void Tick(float DeltaTime) override;
	
	/**
	 * 设置需要在网络上复制的属性
	 * @param OutLifetimeProps - 输出复制属性列表
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	/**
	 * 隐藏HUD上显示的队伍分数
	 */
	void HideTeamScores();
	
	/**
	 * 初始化HUD上显示的队伍分数
	 */
	void InitTeamScores();
	
	/**
	 * 设置HUD上显示的红队分数
	 * @param RedScore - 红队当前分数
	 */
	void SetHUDRedTeamScore(int32 RedScore);
	
	/**
	 * 设置HUD上显示的蓝队分数
	 * @param BlueScore - 蓝队当前分数
	 */
	void SetHUDBlueTeamScore(int32 BlueScore);

	/**
	 * 获取同步后的服务器时间
	 * @return 与服务器世界时钟同步的时间
	 */
	virtual float GetServerTime();
	
	/**
	 * 当玩家连接到服务器时调用，尽快与服务器时钟同步
	 */
	virtual void ReceivedPlayer() override;
	
	/**
	 * 当比赛状态改变时调用
	 * @param State - 新的比赛状态
	 * @param bTeamsMatch - 是否为队伍比赛
	 */
	void OnMatchStateSet(FName State, bool bTeamsMatch = false);
	
	/**
	 * 处理比赛开始逻辑
	 * @param bTeamsMatch - 是否为队伍比赛
	 */
	void HandleMatchHasStarted(bool bTeamsMatch = false);
	
	/**
	 * 处理比赛冷却阶段逻辑
	 */
	void HandleCooldown();

	/**
	 * 单程时间，用于计算客户端和服务器之间的网络延迟
	 */
	float SingleTripTime = 0.f;

	/**
	 * 高延迟警告委托，当检测到玩家延迟过高时触发
	 */
	FHighPingDelegate HighPingDelegate;

	/**
	 * 广播玩家淘汰消息给所有客户端
	 * @param Attacker - 攻击者的PlayerState
	 * @param Victim - 受害者的PlayerState
	 */
	void BroadcastElim(APlayerState* Attacker, APlayerState* Victim);
protected:
	/**
	 * 控制器开始时调用，初始化HUD引用并检查比赛状态
	 */
	virtual void BeginPlay() override;
	
	/**
	 * 设置HUD上显示的当前游戏时间
	 */
	void SetHUDTime();
	
	/**
	 * 轮询初始化状态，确保所有必要组件已加载完成
	 */
	void PollInit();
	
	/**
	 * 设置玩家输入组件，绑定输入事件
	 */
	virtual void SetupInputComponent() override;
	/**
	* 客户端和服务器之间的时间同步相关功能
	*/

	/**
	 * 请求当前服务器时间，传递客户端发送请求时的时间
	 * @param TimeOfClientRequest - 客户端发送请求时的本地时间
	 */
	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	/**
	 * 向客户端报告当前服务器时间，响应ServerRequestServerTime
	 * @param TimeOfClientRequest - 客户端发送请求时的时间
	 * @param TimeServerReceivedClientRequest - 服务器收到请求时的时间
	 */
	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);

	/**
	 * 客户端和服务器之间的时间差
	 */
	float ClientServerDelta = 0.f;

	/**
	 * 时间同步频率（秒），控制客户端请求服务器时间的频率
	 */
	UPROPERTY(EditAnywhere, Category = Time)
	float TimeSyncFrequency = 5.f;

	/**
	 * 时间同步运行时间计数器
	 */
	float TimeSyncRunningTime = 0.f;
	
	/**
	 * 检查是否需要进行时间同步
	 * @param DeltaTime - 帧时间
	 */
	void CheckTimeSync(float DeltaTime);

	/**
	 * 请求服务器检查当前比赛状态
	 */
	UFUNCTION(Server, Reliable)
	void ServerCheckMatchState();

	/**
	 * 当玩家中途加入游戏时，向客户端报告当前比赛状态
	 * @param StateOfMatch - 当前比赛状态
	 * @param Warmup - 热身时间
	 * @param Match - 比赛时间
	 * @param Cooldown - 冷却时间
	 * @param StartingTime - 开始时间
	 */
	UFUNCTION(Client, Reliable)
	void ClientJoinMidgame(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime);

	/**
	 * 显示高延迟警告
	 */
	void HighPingWarning();
	
	/**
	 * 停止高延迟警告
	 */
	void StopHighPingWarning();
	
	/**
	 * 检查玩家延迟
	 * @param DeltaTime - 帧时间
	 */
	void CheckPing(float DeltaTime);

	/**
	 * 显示返回主菜单的UI
	 */
	void ShowReturnToMainMenu();

	/**
	 * 在客户端显示淘汰公告
	 * @param Attacker - 攻击者的PlayerState
	 * @param Victim - 受害者的PlayerState
	 */
	UFUNCTION(Client, Reliable)
	void ClientElimAnnouncement(APlayerState* Attacker, APlayerState* Victim);

	/**
	 * 是否显示队伍分数
	 * 当此属性在客户端复制时，会调用OnRep_ShowTeamScores
	 */
	UPROPERTY(ReplicatedUsing = OnRep_ShowTeamScores)
	bool bShowTeamScores = false;

	/**
	 * 当bShowTeamScores属性在客户端复制时调用
	 */
	UFUNCTION()
	void OnRep_ShowTeamScores();

	/**
	 * 获取玩家信息文本，用于显示游戏结束时的统计信息
	 * @param Players - 玩家状态数组
	 * @return 格式化的玩家信息文本
	 */
	FString GetInfoText(const TArray<class ABlasterPlayerState*>& Players);
	
	/**
	 * 获取队伍信息文本，用于显示游戏结束时的队伍统计信息
	 * @param BlasterGameState - 游戏状态
	 * @return 格式化的队伍信息文本
	 */
	FString GetTeamsInfoText(class ABlasterGameState* BlasterGameState);
private:
	/**
	 * HUD引用，用于更新玩家界面显示
	 */
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	/**
	 * 游戏模式引用，用于访问游戏规则和逻辑
	 */
	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

	/** 
	* 返回主菜单相关
	*/

	/**
	 * 返回主菜单窗口的类类型，用于创建返回主菜单的UI
	 */
	UPROPERTY(EditAnywhere, Category = HUD)
	TSubclassOf<class UUserWidget> ReturnToMainMenuWidget;

	/**
	 * 返回主菜单窗口的实例，用于管理返回主菜单UI的显示和交互
	 */
	UPROPERTY()
	class UReturnToMainMenu* ReturnToMainMenu;

	/**
	 * 是否打开了返回主菜单UI
	 */
	bool bReturnToMainMenuOpen = false;

	/**
	 * 关卡开始时间，用于计算游戏进行时间
	 */
	float LevelStartingTime = 0.f;
	/**
	 * 比赛总时间，用于倒计时显示
	 */
	float MatchTime = 0.f;
	/**
	 * 热身阶段时间，用于倒计时显示
	 */
	float WarmupTime = 0.f;
	/**
	 * 冷却阶段时间，用于倒计时显示
	 */
	float CooldownTime = 0.f;
	/**
	 * 倒计时整数，用于UI显示
	 */
	uint32 CountdownInt = 0;

	/**
	 * 当前比赛状态，当此属性在客户端复制时，会调用OnRep_MatchState
	 */
	UPROPERTY(ReplicatedUsing = OnRep_MatchState)
	FName MatchState;

	/**
	 * 当MatchState属性在客户端复制时调用
	 */
	UFUNCTION()
	void OnRep_MatchState();

	/**
	 * 角色叠加层UI引用，用于显示玩家状态信息
	 */
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	/**
	 * 当前HUD显示的健康值
	 */
	float HUDHealth;
	/**
	 * 是否初始化了健康值显示
	 */
	bool bInitializeHealth = false;
	/**
	 * 当前HUD显示的最大健康值
	 */
	float HUDMaxHealth;
	/**
	 * 当前HUD显示的分数
	 */
	float HUDScore;
	/**
	 * 是否初始化了分数显示
	 */
	bool bInitializeScore = false;
	/**
	 * 当前HUD显示的击败数
	 */
	int32 HUDDefeats;
	/**
	 * 是否初始化了击败数显示
	 */
	bool bInitializeDefeats = false;
	/**
	 * 当前HUD显示的手榴弹数量
	 */
	int32 HUDGrenades;
	/**
	 * 是否初始化了手榴弹数量显示
	 */
	bool bInitializeGrenades = false;
	/**
	 * 当前HUD显示的护盾值
	 */
	float HUDShield;
	/**
	 * 是否初始化了护盾值显示
	 */
	bool bInitializeShield = false;
	/**
	 * 当前HUD显示的最大护盾值
	 */
	float HUDMaxShield;
	/**
	 * 当前HUD显示的携带弹药数量
	 */
	float HUDCarriedAmmo;
	/**
	 * 是否初始化了携带弹药数量显示
	 */
	bool bInitializeCarriedAmmo = false;
	/**
	 * 当前HUD显示的武器弹药数量
	 */
	float HUDWeaponAmmo;
	/**
	 * 是否初始化了武器弹药数量显示
	 */
	bool bInitializeWeaponAmmo = false;

	/**
	 * 高延迟警告运行时间计数器
	 */
	float HighPingRunningTime = 0.f;

	/**
	 * 高延迟警告持续时间阈值（秒），超过此时间将显示高延迟警告
	 */
	UPROPERTY(EditAnywhere)
	float HighPingDuration = 5.f;

	/**
	 * 延迟动画运行时间计数器
	 */
	float PingAnimationRunningTime = 0.f;

	/**
	 * 检查延迟的频率（秒），控制客户端检查延迟的频率
	 */
	UPROPERTY(EditAnywhere)
	float CheckPingFrequency = 20.f;

	/**
	 * 向服务器报告客户端延迟状态
	 * @param bHighPing - 客户端是否检测到高延迟
	 */
	UFUNCTION(Server, Reliable)
	void ServerReportPingStatus(bool bHighPing);

	/**
	 * 高延迟阈值（毫秒），超过此值将被认为是高延迟
	 */
	UPROPERTY(EditAnywhere)
	float HighPingThreshold = 50.f;
};
