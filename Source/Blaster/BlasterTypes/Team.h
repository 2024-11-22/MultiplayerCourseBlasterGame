// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

/**
 * 定义游戏中玩家和队伍的所属关系
 * 用于团队对战模式中的队伍识别和分数统计
 */
UENUM(BlueprintType)
enum class ETeam : uint8
{
	/** 红色队伍 */
	ET_RedTeam UMETA(DisplayName = "RedTeam"),
	/** 蓝色队伍 */
	ET_BlueTeam UMETA(DisplayName = "BlueTeam"),
	/** 无队伍状态，通常用于自由模式或观察者 */
	ET_NoTeam UMETA(DisplayName = "NoTeam"),

	/** 枚举最大值，仅用于内部数组大小计算，不应在代码中直接使用 */
	ET_MAX UMETA(DisplayName = "DefaultMAX")
};