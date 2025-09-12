// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CharacterOverlay.generated.h"

/**
 * 角色状态叠加层类：显示玩家的各种状态信息，如生命值、护盾、分数、弹药等
 */
UCLASS()
class BLASTER_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()
public:
	/** 生命值进度条 */
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	/** 生命值文本显示 */
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* HealthText;

	/** 护盾值进度条 */
	UPROPERTY(meta = (BindWidget))
	UProgressBar* ShieldBar;

	/** 护盾值文本显示 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ShieldText;

	/** 玩家个人分数显示 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreAmount;

	/** 红队分数显示 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RedTeamScore;

	/** 蓝队分数显示 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* BlueTeamScore;

	/** 分数显示间隔文本 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ScoreSpacerText;

	/** 击败数显示 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* DefeatsAmount;

	/** 当前武器弹药量显示 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WeaponAmmoAmount;

	/** 携带弹药量显示 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CarriedAmmoAmount;

	/** 比赛倒计时显示 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchCountdownText;

	/** 手榴弹数量显示 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GrenadesText;

	/** 高延迟警告图标 */
	UPROPERTY(meta = (BindWidget))
	class UImage* HighPingImage;

	/** 高延迟警告动画 */
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* HighPingAnimation;
};
