// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "ShieldPickup.generated.h"

/**
 * 护盾拾取物类：提供玩家护盾值回复功能
 */
UCLASS()
class BLASTER_API AShieldPickup : public APickup
{
	GENERATED_BODY()
protected:
	/**
	 * 当玩家接触到护盾拾取物时触发的重叠事件处理函数
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
	/** 护盾回复量：每次拾取可回复的护盾值总量 */
	UPROPERTY(EditAnywhere)
	float ShieldReplenishAmount = 100.f;

	/** 护盾回复时间：完成全部护盾回复所需的时间（秒） */
	UPROPERTY(EditAnywhere)
	float ShieldReplenishTime = 5.f;
};
