// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileBullet.generated.h"

/**
 * 子弹类：代表游戏中的子弹投射物，继承自基础投射物类
 * 负责处理子弹的物理运动、碰撞检测和伤害逻辑
 */
UCLASS()
class BLASTER_API AProjectileBullet : public AProjectile
{
	GENERATED_BODY()
public:
	/**
	 * 构造函数：初始化子弹的核心组件和属性设置
	 */
	AProjectileBullet();

#if WITH_EDITOR
	/**
	 * 在编辑器中修改属性时的回调函数
	 * @param Event 属性变更事件，包含被修改的属性信息
	 */
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& Event) override;
#endif

protected:
	/**
	 * 处理子弹碰撞事件，包含伤害计算和服务器端重绕逻辑
	 * @param HitComp 碰撞的组件
	 * @param OtherActor 被碰撞的Actor
	 * @param OtherComp 被碰撞的组件
	 * @param NormalImpulse 碰撞的法向冲量
	 * @param Hit 碰撞结果信息
	 */
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	
	/**
	 * 游戏开始时的初始化函数
	 */
	virtual void BeginPlay() override;
};
