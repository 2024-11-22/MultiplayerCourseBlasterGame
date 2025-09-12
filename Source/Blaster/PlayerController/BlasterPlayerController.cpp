// Copyright 2023 Blaster Game. All Rights Reserved.

/**
 * BlasterPlayerController类的实现文件
 * 扩展标准PlayerController，处理游戏内玩家控制、HUD显示和网络同步等核心功能
 */
#include "BlasterPlayerController.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/HUD/Announcement.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Components/Image.h"
#include "Blaster/HUD/ReturnToMainMenu.h"
#include "Blaster/BlasterTypes/Announcement.h"

/**
 * 广播玩家淘汰消息给所有客户端
 * @param Attacker - 攻击者的PlayerState
 * @param Victim - 受害者的PlayerState
 */
void ABlasterPlayerController::BroadcastElim(APlayerState* Attacker, APlayerState* Victim)
{
	// 调用客户端RPC函数，在所有客户端上显示淘汰公告
	ClientElimAnnouncement(Attacker, Victim);
}

/**
 * 在客户端显示淘汰公告
 * @param Attacker - 攻击者的PlayerState
 * @param Victim - 受害者的PlayerState
 */
void ABlasterPlayerController::ClientElimAnnouncement_Implementation(APlayerState* Attacker, APlayerState* Victim)
{
	// 获取本地玩家状态
	APlayerState* Self = GetPlayerState<APlayerState>();
	if (Attacker && Victim && Self)
	{
		// 确保HUD引用有效
		BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
		if (BlasterHUD)
		{
			// 根据攻击者、受害者和本地玩家的关系，显示不同的淘汰消息
			if (Attacker == Self && Victim != Self)
			{
				// 本地玩家淘汰了其他玩家
				BlasterHUD->AddElimAnnouncement("You", Victim->GetPlayerName());
				return;
			}
			if (Victim == Self && Attacker != Self)
			{
				// 本地玩家被其他玩家淘汰
				BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "you");
				return;
			}
			if (Attacker == Victim && Attacker == Self)
			{
				// 本地玩家自裁
				BlasterHUD->AddElimAnnouncement("You", "yourself");
				return;
			}
			if (Attacker == Victim && Attacker != Self)
			{
				// 其他玩家自裁
				BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), "themselves");
				return;
			}
			// 普通的淘汰消息
			BlasterHUD->AddElimAnnouncement(Attacker->GetPlayerName(), Victim->GetPlayerName());
		}
	}
}

/**
 * 控制器开始时调用，初始化HUD引用并检查比赛状态
 */
void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// 初始化HUD引用
	BlasterHUD = Cast<ABlasterHUD>(GetHUD());
	// 请求服务器检查当前比赛状态
	ServerCheckMatchState();
}

/**
 * 设置需要在网络上复制的属性
 * @param OutLifetimeProps - 输出复制属性列表
 */
void ABlasterPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 复制比赛状态属性
	DOREPLIFETIME(ABlasterPlayerController, MatchState);
	// 复制是否显示队伍分数的属性
	DOREPLIFETIME(ABlasterPlayerController, bShowTeamScores);
}

/**
 * 隐藏HUD上显示的队伍分数
 */
void ABlasterPlayerController::HideTeamScores()
{
	// 确保HUD引用有效
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	// 检查HUD组件是否有效
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->RedTeamScore &&
		BlasterHUD->CharacterOverlay->BlueTeamScore &&
		BlasterHUD->CharacterOverlay->ScoreSpacerText;
	if (bHUDValid)
	{
		// 清空所有队伍分数相关文本
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText());
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText());
		BlasterHUD->CharacterOverlay->ScoreSpacerText->SetText(FText());
	}
}

/**
 * 初始化HUD上显示的队伍分数
 */
void ABlasterPlayerController::InitTeamScores()
{
	// 确保HUD引用有效
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	// 检查HUD组件是否有效
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->RedTeamScore &&
		BlasterHUD->CharacterOverlay->BlueTeamScore &&
		BlasterHUD->CharacterOverlay->ScoreSpacerText;
	if (bHUDValid)
	{
		// 初始化为0分和分隔符
		FString Zero("0");
		FString Spacer("|");
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(Zero));
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(Zero));
		BlasterHUD->CharacterOverlay->ScoreSpacerText->SetText(FText::FromString(Spacer));
	}
}

/**
 * 设置HUD上显示的红队分数
 * @param RedScore - 红队当前分数
 */
void ABlasterPlayerController::SetHUDRedTeamScore(int32 RedScore)
{
	// 确保HUD引用有效
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	// 检查HUD组件是否有效
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->RedTeamScore;
	if (bHUDValid)
	{
		// 格式化分数文本并更新HUD
		FString ScoreText = FString::Printf(TEXT("%d"), RedScore);
		BlasterHUD->CharacterOverlay->RedTeamScore->SetText(FText::FromString(ScoreText));
	}
}

/**
 * 设置HUD上显示的蓝队分数
 * @param BlueScore - 蓝队当前分数
 */
void ABlasterPlayerController::SetHUDBlueTeamScore(int32 BlueScore)
{
	// 确保HUD引用有效
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	// 检查HUD组件是否有效
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->BlueTeamScore;
	if (bHUDValid)
	{
		// 格式化分数文本并更新HUD
		FString ScoreText = FString::Printf(TEXT("%d"), BlueScore);
		BlasterHUD->CharacterOverlay->BlueTeamScore->SetText(FText::FromString(ScoreText));
	}
}

/**
 * 每帧更新函数，处理HUD时间更新、时间同步检查、初始化轮询和延迟检查
 * @param DeltaTime - 帧时间
 */
void ABlasterPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 更新HUD上显示的当前游戏时间
	SetHUDTime();
	// 检查是否需要进行时间同步
	CheckTimeSync(DeltaTime);
	// 轮询初始化状态
	PollInit();
	// 检查玩家延迟
	CheckPing(DeltaTime);
}

/**
 * 检查玩家延迟
 * @param DeltaTime - 帧时间
 */
void ABlasterPlayerController::CheckPing(float DeltaTime)
{
	// 仅在客户端执行
	if (HasAuthority()) return;
	// 更新延迟检查运行时间
	HighPingRunningTime += DeltaTime;
	// 按照设定的频率检查延迟
	if (HighPingRunningTime > CheckPingFrequency)
	{
		// 确保PlayerState有效
		PlayerState = PlayerState == nullptr ? GetPlayerState<APlayerState>() : PlayerState;
		if (PlayerState)
		{
			// 注意：Ping值在Unreal中被压缩，实际值需要乘以4
			if (PlayerState->GetPing() * 4 > HighPingThreshold)
			{
				// 显示高延迟警告
				HighPingWarning();
				PingAnimationRunningTime = 0.f;
				// 向服务器报告高延迟状态
				ServerReportPingStatus(true);
			}
			else
			{
				// 向服务器报告正常延迟状态
				ServerReportPingStatus(false);
			}
		}
		// 重置延迟检查运行时间
		HighPingRunningTime = 0.f;
	}
	// 检查高延迟警告动画是否在播放
	bool bHighPingAnimationPlaying = 
		BlasterHUD && BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingAnimation &&
		BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation);
	if (bHighPingAnimationPlaying)
	{
		// 更新动画播放时间
		PingAnimationRunningTime += DeltaTime;
		// 如果动画播放时间超过设定的高延迟持续时间，则停止警告
		if (PingAnimationRunningTime > HighPingDuration)
		{
			StopHighPingWarning();
		}
	}
}

/**
 * 显示返回主菜单的UI
 */
void ABlasterPlayerController::ShowReturnToMainMenu()
{
	// 确保返回主菜单的Widget类已设置
	if (ReturnToMainMenuWidget == nullptr) return;
	// 如果返回主菜单的Widget实例不存在，则创建一个
	if (ReturnToMainMenu == nullptr)
	{
		ReturnToMainMenu = CreateWidget<UReturnToMainMenu>(this, ReturnToMainMenuWidget);
	}
	if (ReturnToMainMenu)
	{
		// 切换返回主菜单UI的显示状态
		bReturnToMainMenuOpen = !bReturnToMainMenuOpen;
		if (bReturnToMainMenuOpen)
		{
			// 显示返回主菜单UI
			ReturnToMainMenu->MenuSetup();
		}
		else
		{
			// 隐藏返回主菜单UI
			ReturnToMainMenu->MenuTearDown();
		}
	}
}

/**
 * 当bShowTeamScores属性在网络上复制时被调用
 * 控制是否显示队伍分数
 */
void ABlasterPlayerController::OnRep_ShowTeamScores()
{
	if (bShowTeamScores)
	{
		InitTeamScores();
	}
	else
	{
		HideTeamScores();
	}
}

/**
 * 向服务器报告客户端的延迟状态
 * @param bHighPing - 是否存在高延迟
 */
void ABlasterPlayerController::ServerReportPingStatus_Implementation(bool bHighPing)
{
	HighPingDelegate.Broadcast(bHighPing);
}

/**
 * 检查并同步客户端与服务器之间的时间
 * @param DeltaTime - 帧时间
 */
void ABlasterPlayerController::CheckTimeSync(float DeltaTime)
{
	// 更新时间同步运行时间
	TimeSyncRunningTime += DeltaTime;
	// 按照设定的频率请求服务器时间
	if (IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

/**
 * 显示高延迟警告UI
 */
void ABlasterPlayerController::HighPingWarning()
{
	// 确保HUD引用有效
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	// 检查高延迟警告UI组件是否有效
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingImage &&
		BlasterHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid)
	{
		// 显示高延迟警告图标并播放动画
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(1.f);
		BlasterHUD->CharacterOverlay->PlayAnimation(
			BlasterHUD->CharacterOverlay->HighPingAnimation,
			0.f,
			5);
	}
}

/**
 * 停止高延迟警告UI
 */
void ABlasterPlayerController::StopHighPingWarning()
{
	// 确保HUD引用有效
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	// 检查高延迟警告UI组件是否有效
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HighPingImage &&
		BlasterHUD->CharacterOverlay->HighPingAnimation;
	if (bHUDValid)
	{
		// 隐藏高延迟警告图标
		BlasterHUD->CharacterOverlay->HighPingImage->SetOpacity(0.f);
		// 如果动画正在播放，则停止动画
		if (BlasterHUD->CharacterOverlay->IsAnimationPlaying(BlasterHUD->CharacterOverlay->HighPingAnimation))
		{
			BlasterHUD->CharacterOverlay->StopAnimation(BlasterHUD->CharacterOverlay->HighPingAnimation);
		}
	}
}

/**
 * 服务器端检查当前比赛状态
 */
void ABlasterPlayerController::ServerCheckMatchState_Implementation()
{
	// 获取游戏模式引用
	ABlasterGameMode* GameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
	if (GameMode)
	{
		// 获取游戏模式中的各种时间设置
		WarmupTime = GameMode->WarmupTime;
		MatchTime = GameMode->MatchTime;
		CooldownTime = GameMode->CooldownTime;
		LevelStartingTime = GameMode->LevelStartingTime;
		// 获取当前比赛状态
		MatchState = GameMode->GetMatchState();
		// 通知客户端当前比赛状态
		ClientJoinMidgame(MatchState, WarmupTime, MatchTime, CooldownTime, LevelStartingTime);
	}
}

/**
 * 客户端加入正在进行的游戏时调用
 * @param StateOfMatch - 当前比赛状态
 * @param Warmup - 热身时间
 * @param Match - 比赛时间
 * @param Cooldown - 冷却时间
 * @param StartingTime - 关卡开始时间
 */
void ABlasterPlayerController::ClientJoinMidgame_Implementation(FName StateOfMatch, float Warmup, float Match, float Cooldown, float StartingTime)
{
	// 设置各种时间参数
	WarmupTime = Warmup;
	MatchTime = Match;
	CooldownTime = Cooldown;
	LevelStartingTime = StartingTime;
	// 设置比赛状态
	MatchState = StateOfMatch;
	// 处理比赛状态变化
	OnMatchStateSet(MatchState);
	// 如果比赛还在等待开始，则添加公告UI
	if (BlasterHUD && MatchState == MatchState::WaitingToStart)
	{
		BlasterHUD->AddAnnouncement();
	}
}

/**
 * 当控制器控制一个角色时调用
 * @param InPawn - 被控制的角色
 */
void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	// 尝试将被控制的角色转换为BlasterCharacter类型
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (BlasterCharacter)
	{
		// 更新HUD上显示的角色生命值
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
	}
}

/**
 * 设置HUD上显示的生命值
 * @param Health - 当前生命值
 * @param MaxHealth - 最大生命值
 */
void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	// 确保HUD引用有效
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	// 检查生命值UI组件是否有效
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->HealthBar &&
		BlasterHUD->CharacterOverlay->HealthText;
	if (bHUDValid)
	{
		// 计算生命值百分比并更新进度条
		const float HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		// 格式化生命值文本并更新
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
	else
	{
		// 如果HUD还未初始化，则记录生命值数据，等待初始化后使用
		bInitializeHealth = true;
		HUDHealth = Health;
		HUDMaxHealth = MaxHealth;
	}
}

/**
 * 设置HUD上显示的护盾值
 * @param Shield - 当前护盾值
 * @param MaxShield - 最大护盾值
 */
void ABlasterPlayerController::SetHUDShield(float Shield, float MaxShield)
{
	// 确保HUD引用有效
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	// 检查护盾UI组件是否有效
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ShieldBar &&
		BlasterHUD->CharacterOverlay->ShieldText;
	if (bHUDValid)
	{
		// 计算护盾百分比并更新进度条
		const float ShieldPercent = Shield / MaxShield;
		BlasterHUD->CharacterOverlay->ShieldBar->SetPercent(ShieldPercent);
		// 格式化护盾文本并更新
		FString ShieldText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Shield), FMath::CeilToInt(MaxShield));
		BlasterHUD->CharacterOverlay->ShieldText->SetText(FText::FromString(ShieldText));
	}
	else
	{
		// 如果HUD还未初始化，则记录护盾数据，等待初始化后使用
		bInitializeShield = true;
		HUDShield = Shield;
		HUDMaxShield = MaxShield;
	}
}

/**
 * 设置HUD上显示的分数
 * @param Score - 当前分数
 */
void ABlasterPlayerController::SetHUDScore(float Score)
{
	// 确保HUD引用有效
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	// 检查分数UI组件是否有效
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->ScoreAmount;

	if (bHUDValid)
	{
		// 格式化分数文本并更新
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
	else
	{
		// 如果HUD还未初始化，则记录分数数据，等待初始化后使用
		bInitializeScore = true;
		HUDScore = Score;
	}
}

/**
 * 设置HUD上显示的淘汰数
 * @param Defeats - 当前淘汰数
 */
void ABlasterPlayerController::SetHUDDefeats(int32 Defeats)
{
	// 确保HUD引用有效
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	// 检查淘汰数UI组件是否有效
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->DefeatsAmount;
	if (bHUDValid)
	{
		// 格式化淘汰数文本并更新
		FString DefeatsText = FString::Printf(TEXT("%d"), Defeats);
		BlasterHUD->CharacterOverlay->DefeatsAmount->SetText(FText::FromString(DefeatsText));
	}
	else
	{
		// 如果HUD还未初始化，则记录淘汰数数据，等待初始化后使用
		bInitializeDefeats = true;
		HUDDefeats = Defeats;
	}
}

/**
 * 设置HUD上显示的武器弹药数量
 * @param Ammo - 当前武器弹药数量
 */
void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	// 确保HUD引用有效
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	// 检查武器弹药UI组件是否有效
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bHUDValid)
	{
		// 格式化弹药数量文本并更新
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		// 如果HUD还未初始化，则记录弹药数据，等待初始化后使用
		bInitializeWeaponAmmo = true;
		HUDWeaponAmmo = Ammo;
	}
}

/**
 * 设置HUD上显示的携带弹药数量
 * @param Ammo - 当前携带弹药数量
 */
void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	// 确保HUD引用有效
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	// 检查携带弹药UI组件是否有效
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bHUDValid)
	{
		// 格式化携带弹药数量文本并更新
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
	else
	{
		// 如果HUD还未初始化，则记录携带弹药数据，等待初始化后使用
		bInitializeCarriedAmmo = true;
		HUDCarriedAmmo = Ammo;
	}
}

/**
 * 设置HUD上显示的比赛倒计时
 * @param CountdownTime - 倒计时时间（秒）
 */
void ABlasterPlayerController::SetHUDMatchCountdown(float CountdownTime)
{
	// 确保HUD引用有效
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	// 检查倒计时UI组件是否有效
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->MatchCountdownText;
	if (bHUDValid)
	{
		// 如果倒计时时间小于0，则清空倒计时文本
		if (CountdownTime < 0.f)
		{
			BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText());
			return;
		}

		// 计算分钟和秒
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		// 格式化倒计时文本并更新
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchCountdownText->SetText(FText::FromString(CountdownText));
	}
}

/**
 * 设置HUD公告上显示的倒计时
 * @param CountdownTime - 倒计时时间（秒）
 */
void ABlasterPlayerController::SetHUDAnnouncementCountdown(float CountdownTime)
{
	// 确保HUD引用有效
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	// 检查公告倒计时UI组件是否有效
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->Announcement &&
		BlasterHUD->Announcement->WarmupTime;
	if (bHUDValid)
	{
		// 如果倒计时时间小于0，则清空倒计时文本
		if (CountdownTime < 0.f)
		{
			BlasterHUD->Announcement->WarmupTime->SetText(FText());
			return;
		}

		// 计算分钟和秒
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = CountdownTime - Minutes * 60;

		// 格式化倒计时文本并更新
		FString CountdownText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->Announcement->WarmupTime->SetText(FText::FromString(CountdownText));
	}
}

/**
 * 设置HUD上显示的手榴弹数量
 * @param Grenades - 当前手榴弹数量
 */
void ABlasterPlayerController::SetHUDGrenades(int32 Grenades)
{
	// 确保HUD引用有效
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	// 检查手榴弹数量UI组件是否有效
	bool bHUDValid = BlasterHUD &&
		BlasterHUD->CharacterOverlay &&
		BlasterHUD->CharacterOverlay->GrenadesText;
	if (bHUDValid)
	{
		// 格式化手榴弹数量文本并更新
		FString GrenadesText = FString::Printf(TEXT("%d"), Grenades);
		BlasterHUD->CharacterOverlay->GrenadesText->SetText(FText::FromString(GrenadesText));
	}
	else
	{
		// 如果HUD还未初始化，则记录手榴弹数据，等待初始化后使用
		bInitializeGrenades = true;
		HUDGrenades = Grenades;
	}
}

/**
 * 更新HUD上显示的游戏时间
 */
void ABlasterPlayerController::SetHUDTime()
{
	float TimeLeft = 0.f;
	// 根据当前比赛状态计算剩余时间
	if (MatchState == MatchState::WaitingToStart) TimeLeft = WarmupTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::InProgress) TimeLeft = WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	else if (MatchState == MatchState::Cooldown) TimeLeft = CooldownTime + WarmupTime + MatchTime - GetServerTime() + LevelStartingTime;
	uint32 SecondsLeft = FMath::CeilToInt(TimeLeft);
	
	// 在服务器端，从游戏模式获取更准确的倒计时时间
	if (HasAuthority())
	{
		if (BlasterGameMode == nullptr)
		{
			BlasterGameMode = Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this));
			LevelStartingTime = BlasterGameMode->LevelStartingTime;
		}
		BlasterGameMode = BlasterGameMode == nullptr ? Cast<ABlasterGameMode>(UGameplayStatics::GetGameMode(this)) : BlasterGameMode;
		if (BlasterGameMode)
		{
			SecondsLeft = FMath::CeilToInt(BlasterGameMode->GetCountdownTime() + LevelStartingTime);
		}
	}

	// 当剩余时间改变时，更新相应的倒计时UI
	if (CountdownInt != SecondsLeft)
	{
		if (MatchState == MatchState::WaitingToStart || MatchState == MatchState::Cooldown)
		{
			SetHUDAnnouncementCountdown(TimeLeft);
		}
		if (MatchState == MatchState::InProgress)
		{
			SetHUDMatchCountdown(TimeLeft);
		}
	}

	// 更新当前倒计时整数
	CountdownInt = SecondsLeft;
}

/**
 * 轮询初始化状态，确保HUD组件正确初始化
 */
void ABlasterPlayerController::PollInit()
{
	// 如果角色覆盖层尚未初始化
	if (CharacterOverlay == nullptr)
	{
		// 检查HUD和角色覆盖层是否有效
		if (BlasterHUD && BlasterHUD->CharacterOverlay)
		{
			// 设置角色覆盖层引用
			CharacterOverlay = BlasterHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				// 如果有未初始化的HUD数据，则进行初始化
				if (bInitializeHealth) SetHUDHealth(HUDHealth, HUDMaxHealth);
				if (bInitializeShield) SetHUDShield(HUDShield, HUDMaxShield);
				if (bInitializeScore) SetHUDScore(HUDScore);
				if (bInitializeDefeats) SetHUDDefeats(HUDDefeats);
				if (bInitializeCarriedAmmo) SetHUDCarriedAmmo(HUDCarriedAmmo);
				if (bInitializeWeaponAmmo) SetHUDWeaponAmmo(HUDWeaponAmmo);

				// 获取角色和战斗组件引用，初始化手榴弹数量
				ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
				if (BlasterCharacter && BlasterCharacter->GetCombat())
				{
					if (bInitializeGrenades) SetHUDGrenades(BlasterCharacter->GetCombat()->GetGrenades());
				}
			}
		}
	}
}

/**
 * 设置输入组件，绑定输入事件
 */
void ABlasterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (InputComponent == nullptr) return;

	// 绑定退出游戏动作到显示返回主菜单函数
	InputComponent->BindAction("Quit", IE_Pressed, this, &ABlasterPlayerController::ShowReturnToMainMenu);

}

/**
 * 服务器端处理客户端的时间同步请求
 * @param TimeOfClientRequest - 客户端请求时间
 */
void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	// 获取服务器接收请求的时间
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	// 向客户端报告服务器时间
	ClientReportServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

/**
 * 客户端处理服务器返回的时间信息，计算时间差
 * @param TimeOfClientRequest - 客户端请求时间
 * @param TimeServerReceivedClientRequest - 服务器接收请求的时间
 */
void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerReceivedClientRequest)
{
	// 计算往返时间
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	// 计算单程时间
	SingleTripTime = 0.5f * RoundTripTime;
	// 计算当前服务器时间
	float CurrentServerTime = TimeServerReceivedClientRequest + SingleTripTime;
	// 计算客户端与服务器的时间差
	ClientServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

/**
 * 获取服务器时间
 * @return 服务器当前时间
 */
float ABlasterPlayerController::GetServerTime()
{
	// 如果是服务器，则直接返回世界时间
	if (HasAuthority()) return GetWorld()->GetTimeSeconds();
	// 如果是客户端，则返回本地时间加上与服务器的时间差
	else return GetWorld()->GetTimeSeconds() + ClientServerDelta;
}

/**
 * 当玩家连接到游戏时调用
 */
void ABlasterPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	// 如果是本地控制器，则请求服务器时间进行同步
	if (IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

/**
 * 当比赛状态改变时调用
 * @param State - 新的比赛状态
 * @param bTeamsMatch - 是否为团队比赛
 */
void ABlasterPlayerController::OnMatchStateSet(FName State, bool bTeamsMatch)
{
	// 更新比赛状态
	MatchState = State;

	// 根据新的比赛状态执行相应的处理
	if (MatchState == MatchState::InProgress)
	{
		// 处理比赛开始
		HandleMatchHasStarted(bTeamsMatch);
	}
	else if (MatchState == MatchState::Cooldown)
	{
		// 处理比赛冷却阶段
		HandleCooldown();
	}
}

/**
 * 当MatchState属性在网络上复制时被调用
 */
void ABlasterPlayerController::OnRep_MatchState()
{
	// 根据新的比赛状态执行相应的处理
	if (MatchState == MatchState::InProgress)
	{
		// 处理比赛开始
		HandleMatchHasStarted();
	}
	else if (MatchState == MatchState::Cooldown)
	{
		// 处理比赛冷却阶段
		HandleCooldown();
	}
}

/**
 * 处理比赛开始的逻辑
 * @param bTeamsMatch - 是否为团队比赛
 */
void ABlasterPlayerController::HandleMatchHasStarted(bool bTeamsMatch)
{
	// 在服务器端，设置是否显示团队分数
	if (HasAuthority()) bShowTeamScores = bTeamsMatch;
	// 确保HUD引用有效
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		// 如果角色覆盖层不存在，则添加角色覆盖层
		if (BlasterHUD->CharacterOverlay == nullptr) BlasterHUD->AddCharacterOverlay();
		// 隐藏公告UI
		if (BlasterHUD->Announcement)
		{
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Hidden);
		}
		// 如果不是服务器，则返回
		if (!HasAuthority()) return;
		// 根据是否为团队比赛，初始化或隐藏团队分数
		if (bTeamsMatch)
		{
			InitTeamScores();
		}
		else
		{
			HideTeamScores();
		}
	}
}

/**
 * 处理比赛冷却阶段的逻辑
 */
void ABlasterPlayerController::HandleCooldown()
{
	// 确保HUD引用有效
	BlasterHUD = BlasterHUD == nullptr ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	if (BlasterHUD)
	{
		// 移除角色覆盖层
		BlasterHUD->CharacterOverlay->RemoveFromParent();
		// 检查公告UI组件是否有效
		bool bHUDValid = BlasterHUD->Announcement && 
			BlasterHUD->Announcement->AnnouncementText && 
			BlasterHUD->Announcement->InfoText;

		if (bHUDValid)
		{
			// 显示公告UI
			BlasterHUD->Announcement->SetVisibility(ESlateVisibility::Visible);
			// 设置公告文本
			FString AnnouncementText = Announcement::NewMatchStartsIn;
			BlasterHUD->Announcement->AnnouncementText->SetText(FText::FromString(AnnouncementText));

			// 获取游戏状态和玩家状态引用
			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));
			ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
			if (BlasterGameState && BlasterPlayerState)
			{
				// 获取得分最高的玩家列表
				TArray<ABlasterPlayerState*> TopPlayers = BlasterGameState->TopScoringPlayers;
				// 根据是否显示团队分数，获取相应的信息文本
				FString InfoTextString = bShowTeamScores ? GetTeamsInfoText(BlasterGameState) : GetInfoText(TopPlayers);

				// 设置信息文本
				BlasterHUD->Announcement->InfoText->SetText(FText::FromString(InfoTextString));
			}
		}
	}
	// 获取角色和战斗组件引用
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(GetPawn());
	if (BlasterCharacter && BlasterCharacter->GetCombat())
	{
		// 禁用游戏玩法
		BlasterCharacter->bDisableGameplay = true;
		// 重置开火按钮状态
		BlasterCharacter->GetCombat()->FireButtonPressed(false);
	}
}

/**
 * 获取显示给玩家的信息文本
 * @param Players - 玩家状态数组
 * @return 格式化的信息文本
 */
FString ABlasterPlayerController::GetInfoText(const TArray<class ABlasterPlayerState*>& Players)
{
	// 获取本地玩家状态
	ABlasterPlayerState* BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
	if (BlasterPlayerState == nullptr) return FString();
	FString InfoTextString;
	// 根据玩家数组的数量和内容，生成不同的信息文本
	if (Players.Num() == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (Players.Num() == 1 && Players[0] == BlasterPlayerState)
	{
		// 如果本地玩家是唯一的获胜者
		InfoTextString = Announcement::YouAreTheWinner;
	}
	else if (Players.Num() == 1)
	{
		// 如果有一个其他玩家是获胜者
		InfoTextString = FString::Printf(TEXT("Winner: \n%s"), *Players[0]->GetPlayerName());
	}
	else if (Players.Num() > 1)
	{
		// 如果有多个玩家并列获胜
		InfoTextString = Announcement::PlayersTiedForTheWin;
		InfoTextString.Append(FString("\n"));
		// 列出所有并列获胜的玩家
		for (auto TiedPlayer : Players)
		{
			InfoTextString.Append(FString::Printf(TEXT("%s\n"), *TiedPlayer->GetPlayerName()));
		}
	}

	return InfoTextString;
}

/**
 * 获取团队比赛的信息文本
 * @param BlasterGameState - 游戏状态引用
 * @return 格式化的团队信息文本
 */
FString ABlasterPlayerController::GetTeamsInfoText(ABlasterGameState* BlasterGameState)
{
	if (BlasterGameState == nullptr) return FString();
	FString InfoTextString;

	// 获取红队和蓝队的分数
	const int32 RedTeamScore = BlasterGameState->RedTeamScore;
	const int32 BlueTeamScore = BlasterGameState->BlueTeamScore;

	// 根据分数情况，生成不同的信息文本
	if (RedTeamScore == 0 && BlueTeamScore == 0)
	{
		InfoTextString = Announcement::ThereIsNoWinner;
	}
	else if (RedTeamScore == BlueTeamScore)
	{
		// 如果两队分数相同
		InfoTextString = FString::Printf(TEXT("%s\n"), *Announcement::TeamsTiedForTheWin);
		InfoTextString.Append(Announcement::RedTeam);
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(Announcement::BlueTeam);
		InfoTextString.Append(TEXT("\n"));
	}
	else if (RedTeamScore > BlueTeamScore)
	{
		// 如果红队获胜
		InfoTextString = Announcement::RedTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
	}
	else if (BlueTeamScore > RedTeamScore)
	{
		// 如果蓝队获胜
		InfoTextString = Announcement::BlueTeamWins;
		InfoTextString.Append(TEXT("\n"));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::BlueTeam, BlueTeamScore));
		InfoTextString.Append(FString::Printf(TEXT("%s: %d\n"), *Announcement::RedTeam, RedTeamScore));
	}

	return InfoTextString;
}
