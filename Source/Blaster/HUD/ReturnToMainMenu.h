// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ReturnToMainMenu.generated.h"

/**
 * 返回主菜单UI类：提供返回主菜单的功能，处理游戏退出和会话管理
 */
UCLASS()
class BLASTER_API UReturnToMainMenu : public UUserWidget
{
	GENERATED_BODY()
public:
	/** 设置菜单：初始化菜单组件和功能 */
	void MenuSetup();
	/** 关闭菜单：清理菜单资源和取消注册的事件 */
	void MenuTearDown();

protected:
	/**
	 * 初始化UI组件
	 * 重写父类方法，设置按钮点击事件和获取玩家控制器
	 * @return 初始化是否成功
	 */
	virtual bool Initialize() override;

	/**
	 * 销毁会话回调函数
	 * 当会话销毁时调用
	 * @param bWasSuccessful 会话销毁是否成功
	 */
	UFUNCTION()
	void OnDestroySession(bool bWasSuccessful);

	/** 玩家离开游戏后的回调函数 */
	UFUNCTION()
	void OnPlayerLeftGame();

private:
	/** 返回按钮组件 */
	UPROPERTY(meta = (BindWidget))
	class UButton* ReturnButton;

	/** 返回按钮点击事件处理函数 */
	UFUNCTION()
	void ReturnButtonClicked();

	/** 多人会话子系统引用：用于处理会话创建、加入和销毁 */
	UPROPERTY()
	class UMultiplayerSessionsSubsystem* MultiplayerSessionsSubsystem;

	/** 玩家控制器引用 */
	UPROPERTY()
	class APlayerController* PlayerController;
};
