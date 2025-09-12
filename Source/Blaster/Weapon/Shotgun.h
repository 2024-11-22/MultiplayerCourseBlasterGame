// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HitScanWeapon.h"
#include "Shotgun.generated.h"

/**
 * 霰弹枪类：继承自HitScanWeapon，实现霰弹枪特有的发射和散射逻辑
 * 霰弹枪会发射多个弹丸并具有较强的散射效果
 */
UCLASS()
class BLASTER_API AShotgun : public AHitScanWeapon
{
	GENERATED_BODY()
public:
	/**
	 * 霰弹枪发射函数
	 * 处理霰弹枪的发射逻辑，包括多弹丸追踪、伤害计算和网络同步
	 * @param HitTargets - 命中目标位置数组，每个位置对应一个弹丸的目标点
	 */
	virtual void FireShotgun(const TArray<FVector_NetQuantize>& HitTargets);
	
	/**
	 * 霰弹散射计算函数
	 * 计算霰弹枪发射时多个弹丸的散射轨迹终点
	 * @param HitTarget - 原始瞄准目标位置
	 * @param HitTargets - 输出参数，用于存储散射后的多个弹丸目标位置
	 */
	void ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets);
private:
	/**
	 * 霰弹枪一次发射的弹丸数量
	 * 默认为10个弹丸，可在编辑器中调整
	 */
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	uint32 NumberOfPellets = 10;
};
