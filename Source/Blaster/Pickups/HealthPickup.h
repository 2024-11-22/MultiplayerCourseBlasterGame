// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

/**
 * 生命值拾取物类：提供玩家生命值回复功能
 */
UCLASS()
class BLASTER_API AHealthPickup : public APickup
{
	GENERATED_BODY()
public:
	/** 构造函数：初始化生命值拾取物的基本属性 */
	AHealthPickup();
protected:
	/**
	 * 当玩家接触到生命值拾取物时触发的重叠事件处理函数
	 * @param OverlappedComponent 重叠的组件（通常是碰撞球体）
	 * @param OtherActor 与拾取物重叠的其他Actor（通常是玩家角色）
	 * @param OtherComp 其他Actor上的重叠组件
	 * @param OtherBodyIndex 重叠组件的BodyIndex
	 * @param bFromSweep 是否通过扫描触发
	 * @param SweepResult 扫描结果信息
	 */
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
private:
	/** 治疗量：每次拾取可回复的生命值总量 */
	UPROPERTY(EditAnywhere)
	float HealAmount = 100.f;

	/** 治疗时间：完成全部治疗量所需的时间（秒） */
	UPROPERTY(EditAnywhere)
	float HealingTime = 5.f;
};
