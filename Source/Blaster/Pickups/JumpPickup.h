// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "JumpPickup.generated.h"

/**
 * 跳跃增益拾取物类：提供玩家跳跃能力增强功能
 */
UCLASS()
class BLASTER_API AJumpPickup : public APickup
{
	GENERATED_BODY()
protected:
	/**
	 * 当玩家接触到跳跃增益拾取物时触发的重叠事件处理函数
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
	/** 跳跃Z轴速度增益：玩家跳跃时的垂直速度增加量 */
	UPROPERTY(EditAnywhere)
	float JumpZVelocityBuff = 4000.f;

	/** 跳跃增益持续时间：增强效果的持续时间（秒） */
	UPROPERTY(EditAnywhere)
	float JumpBuffTime = 30.f;
};
