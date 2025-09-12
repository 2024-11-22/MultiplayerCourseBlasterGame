// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileGrenade.generated.h"

/**
 * 手榴弹投射物类：代表游戏中的手榴弹，继承自基础投射物类AProjectile
 * 负责处理手榴弹特有的行为，如反弹物理效果、碰撞音效等
 */
UCLASS()
class BLASTER_API AProjectileGrenade : public AProjectile
{
	GENERATED_BODY()
public:
	/**
	 * 构造函数：初始化手榴弹的核心组件和基本属性
	 */
	AProjectileGrenade();
	/**
	 * 手榴弹销毁时的回调函数：处理手榴弹销毁时的特效播放和清理工作
	 */
	virtual void Destroyed() override;
protected:
	/**
	 * 游戏开始时的初始化函数：设置手榴弹的初始状态和启动必要的组件
	 */
	virtual void BeginPlay() override;

	/**
	 * 反弹事件回调：当手榴弹与其他物体碰撞并反弹时调用
	 * @param ImpactResult 碰撞结果信息
	 * @param ImpactVelocity 碰撞时的速度
	 */
	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);
private:

	/**
	 * 反弹音效：手榴弹碰撞并反弹时播放的音效
	 */
	UPROPERTY(EditAnywhere)
	USoundCue* BounceSound;
};
