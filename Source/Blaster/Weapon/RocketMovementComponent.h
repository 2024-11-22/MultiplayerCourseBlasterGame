// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "RocketMovementComponent.generated.h"

/**
 * 火箭弹专用移动组件类
 * 扩展标准 ProjectileMovementComponent，为火箭弹提供特殊的物理行为和碰撞处理
 */
UCLASS()
class BLASTER_API URocketMovementComponent : public UProjectileMovementComponent
{
	GENERATED_BODY()
protected:
	/**
	 * 处理火箭弹与物体的阻挡碰撞
	 * 重写父类方法，确保火箭弹在碰撞时不会停止移动
	 * @param Hit - 碰撞结果信息
	 * @param TimeTick - 当前时间刻度
	 * @param MoveDelta - 移动增量
	 * @param SubTickTimeRemaining - 剩余子刻度时间
	 * @return 碰撞处理结果，指定下一步操作
	 */
	virtual EHandleBlockingHitResult HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining) override;

	/**
	 * 处理火箭弹的碰撞影响
	 * 重写父类方法，确保火箭弹不会因为碰撞而停止
	 * @param Hit - 碰撞结果信息
	 * @param TimeSlice - 时间片段
	 * @param MoveDelta - 移动增量
	 */
	virtual void HandleImpact(const FHitResult& Hit, float TimeSlice = 0.f, const FVector& MoveDelta = FVector::ZeroVector) override;
};
