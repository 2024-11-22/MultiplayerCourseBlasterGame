// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ElimAnnouncement.generated.h"

/**
 * 击杀公告UI类：显示游戏中的击杀信息，告知玩家谁击败了谁
 */
UCLASS()
class BLASTER_API UElimAnnouncement : public UUserWidget
{
	GENERATED_BODY()
public:
	/**
	 * 设置击杀公告文本
	 * @param AttackerName - 攻击者名称
	 * @param VictimName - 受害者名称
	 */
	void SetElimAnnouncementText(FString AttackerName, FString VictimName);

	/** 公告框容器，用于调整位置和布局 */
	UPROPERTY(meta = (BindWidget))
	class UHorizontalBox* AnnouncementBox;

	/** 公告文本显示框，用于显示击杀信息 */
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AnnouncementText;

};
