// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * 定义玩家角色的转身状态
 * 用于动画系统中根据角色转身方向应用不同的旋转动画
 */
UENUM(BlueprintType)
enum class ETurningInPlace : uint8
{
	/** 角色正在向左转身 */
	ETIP_Left UMETA(DisplayName = "Turning Left"),
	/** 角色正在向右转身 */
	ETIP_Right UMETA(DisplayName = "Turning Right"),
	/** 角色没有转身 */
	ETIP_NotTurning UMETA(DisplayName = "Not Turning"),

	/** 枚举最大值，仅用于内部数组大小计算，不应在代码中直接使用 */
	ETIP_MAX UMETA(DisplayName = "DefaultMAX")
};