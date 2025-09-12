// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 头顶信息显示UI类：显示在角色头顶的信息，如玩家名称、网络角色等
 */
UCLASS()
class BLASTER_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	/** 显示文本的文本框组件 */
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DisplayText;

	/**
	 * 设置显示文本内容
	 * @param TextToDisplay - 要显示的文本字符串
	 */
	void SetDisplayText(FString TextToDisplay);

	/**
	 * 显示玩家的网络角色类型
	 * 可以在蓝图中调用
	 * @param InPawn - 要显示网络角色的 pawn
	 */
	UFUNCTION(BlueprintCallable)
	void ShowPlayerNetRole(APawn* InPawn);

protected:
	/**
	 * 当关卡从世界中移除时调用的函数
	 * 重写父类方法，实现清理逻辑
	 */
	virtual void OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld) override;

};
