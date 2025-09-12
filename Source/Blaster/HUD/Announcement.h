// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcement.generated.h"

/**
 * 公告UI类：用于显示游戏中的各种公告信息，如热身时间、游戏状态通知等
 */
UCLASS()
class BLASTER_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()
public:

	/** 热身时间显示文本框 */
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* WarmupTime;

	/** 公告文本显示框，用于显示主要的公告信息 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* AnnouncementText;

	/** 信息文本显示框，用于显示补充说明信息 */
	UPROPERTY(meta = (BindWidget))
	UTextBlock* InfoText;
	
};
