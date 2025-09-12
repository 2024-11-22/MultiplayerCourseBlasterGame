// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * 定义玩家角色在战斗中的当前状态
 * 用于控制武器切换、装弹和投掷手榴弹等战斗行为的互斥逻辑
 */
UENUM(BlueprintType)
enum class ECombatState : uint8
{
	/** 无战斗状态，玩家可以执行任何战斗动作 */
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	/** 正在装弹状态，此时无法执行其他战斗动作 */
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	/** 正在投掷手榴弹状态，此时无法执行其他战斗动作 */
	ECS_ThrowingGrenade UMETA(DisplayName = "Throwing Grenade"),
	/** 正在切换武器状态，此时无法执行其他战斗动作 */
	ECS_SwappingWeapons UMETA(DisplayName = "Swapping Weapons"),

	/** 枚举最大值，仅用于内部数组大小计算，不应在代码中直接使用 */
	ECS_MAX UMETA(DisplayName = "DefaultMAX")
};