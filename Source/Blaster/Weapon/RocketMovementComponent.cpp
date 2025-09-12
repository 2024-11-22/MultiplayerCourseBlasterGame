// Fill out your copyright notice in the Description page of Project Settings.


#include "RocketMovementComponent.h"

/**
 * 处理火箭弹与物体的阻挡碰撞
 * 重写父类方法，确保火箭弹在碰撞时继续前进，而不是停止
 * @param Hit - 碰撞结果信息
 * @param TimeTick - 当前时间刻度
 * @param MoveDelta - 移动增量
 * @param SubTickTimeRemaining - 剩余子刻度时间
 * @return EHandleBlockingHitResult::AdvanceNextSubstep - 指示继续下一步子步骤而不停止移动
 */
URocketMovementComponent::EHandleBlockingHitResult URocketMovementComponent::HandleBlockingHit(const FHitResult& Hit, float TimeTick, const FVector& MoveDelta, float& SubTickTimeRemaining)
{
	// 调用父类的处理函数，但覆盖其返回值，确保火箭弹不会停止
	Super::HandleBlockingHit(Hit, TimeTick, MoveDelta, SubTickTimeRemaining);
	// 返回AdvanceNextSubstep指示继续移动到下一个子步骤
	return EHandleBlockingHitResult::AdvanceNextSubstep;
}

/**
 * 处理火箭弹的碰撞影响
 * 重写父类方法，提供空实现以确保火箭弹不会因为碰撞而停止
 * 火箭弹的爆炸逻辑由其自身的CollisionBox组件在检测到碰撞时处理
 * @param Hit - 碰撞结果信息
 * @param TimeSlice - 时间片段
 * @param MoveDelta - 移动增量
 */
void URocketMovementComponent::HandleImpact(const FHitResult& Hit, float TimeSlice, const FVector& MoveDelta)
{
	// 火箭弹不应停止；只有当它们的CollisionBox检测到碰撞时才会爆炸
}