// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectile.h"
#include "ProjectileRocket.generated.h"

/**
 * 火箭投射物类：代表游戏中的火箭，继承自基础投射物类AProjectile
 * 负责处理火箭特有的行为，如推进音效、特殊的碰撞处理等
 */
UCLASS()
class BLASTER_API AProjectileRocket : public AProjectile
{
	GENERATED_BODY()
public:
	/**
	 * 构造函数：初始化火箭的核心组件和基本属性
	 */
	AProjectileRocket();
	/**
	 * 火箭销毁时的回调函数：处理火箭销毁时的音效停止和清理工作
	 */
	virtual void Destroyed() override;
protected:
	/**
	 * 碰撞事件回调：当火箭与其他物体碰撞时调用，处理碰撞逻辑和爆炸效果
	 * @param HitComp 碰撞组件
	 * @param OtherActor 被碰撞的Actor
	 * @param OtherComp 被碰撞的组件
	 * @param NormalImpulse 碰撞产生的法线冲量
	 * @param Hit 碰撞结果信息
	 */
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit) override;
	/**
	 * 游戏开始时的初始化函数：设置火箭的初始状态和启动音效
	 */
	virtual void BeginPlay() override;

	/**
	 * 火箭飞行时的循环音效
	 */
	UPROPERTY(EditAnywhere)
	USoundCue* ProjectileLoop;

	/**
	 * 循环音效组件：用于播放火箭飞行时的循环音效
	 */
	UPROPERTY()
	UAudioComponent* ProjectileLoopComponent;

	/**
	 * 循环音效衰减设置：控制火箭音效随距离的衰减效果
	 */
	UPROPERTY(EditAnywhere)
	USoundAttenuation* LoopingSoundAttenuation;

	/**
	 * 火箭移动组件：控制火箭的物理运动和推进效果
	 */
	UPROPERTY(VisibleAnywhere)
	class URocketMovementComponent* RocketMovementComponent;

private:
};
