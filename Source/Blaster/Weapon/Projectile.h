// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectile.generated.h"

/**
 * 基础投射物类：代表游戏中的所有投射物（子弹、手榴弹、火箭等），继承自AActor
 * 负责处理投射物的物理运动、碰撞检测、伤害计算和视觉/音效反馈
 */
UCLASS()
class BLASTER_API AProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	/**
	 * 构造函数：初始化投射物的核心组件和基本属性
	 */
	AProjectile();
	
	/**
	 * 每帧更新函数：处理投射物的帧更新逻辑
	 * @param DeltaTime 帧间隔时间
	 */
	virtual void Tick(float DeltaTime) override;
	
	/**
	 * 投射物销毁时的回调函数：处理投射物销毁时的清理工作
	 */
	virtual void Destroyed() override;

	/** 
	* 服务器端重绕标志：指示是否使用服务器端重绕技术
	*/
	bool bUseServerSideRewind = false;
	
	/**
	* 射线起点：用于服务器端重绕计算的射线起点
	*/
	FVector_NetQuantize TraceStart;
	
	/**
	* 初始速度：投射物发射时的初始速度
	*/
	FVector_NetQuantize100 InitialVelocity;

	/**
	* 初始速度值：投射物的移动速度
	*/
	UPROPERTY(EditAnywhere)
	float InitialSpeed = 15000;

	/**
	* 基础伤害：投射物造成的基础伤害值（主要用于手榴弹和火箭）
	*/
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	/**
	* 爆头伤害：击中头部时造成的伤害值（对手榴弹和火箭影响不大）
	*/
	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;

protected:
	/**
	* 游戏开始时的初始化函数：设置投射物的初始状态和启动必要的定时器
	*/
	virtual void BeginPlay() override;
	
	/**
	* 启动销毁定时器：设置投射物在一定时间后自动销毁
	*/
	void StartDestroyTimer();
	
	/**
	* 销毁定时器结束回调：当销毁定时器结束时调用，销毁投射物
	*/
	void DestroyTimerFinished();
	
	/**
	* 生成尾迹系统：在投射物飞行时生成视觉尾迹效果
	*/
	void SpawnTrailSystem();
	
	/**
	* 爆炸伤害计算：处理投射物爆炸时的范围伤害
	*/
	void ExplodeDamage();

	/**
	* 碰撞事件回调：当投射物与其他物体碰撞时调用
	* @param HitComp 碰撞组件
	* @param OtherActor 被碰撞的Actor
	* @param OtherComp 被碰撞的组件
	* @param NormalImpulse 碰撞产生的法线冲量
	* @param Hit 碰撞结果信息
	*/
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	/**
	* 碰撞粒子效果：投射物碰撞时播放的粒子效果
	*/
	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	/**
	* 碰撞音效：投射物碰撞时播放的音效
	*/
	UPROPERTY(EditAnywhere)
	class USoundCue* ImpactSound;

	/**
	* 碰撞盒：投射物的碰撞组件，用于检测碰撞
	*/
	UPROPERTY(EditAnywhere)
	class UBoxComponent* CollisionBox;

	/**
	* 尾迹系统：投射物飞行时的尾迹特效系统
	*/
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* TrailSystem;

	/**
	* 尾迹系统组件：尾迹特效的实例化组件
	*/
	UPROPERTY()
	class UNiagaraComponent* TrailSystemComponent;

	/**
	* 投射物移动组件：控制投射物的物理运动
	*/
	UPROPERTY(VisibleAnywhere)
	class UProjectileMovementComponent* ProjectileMovementComponent;

	/**
	* 投射物网格：投射物的视觉表示
	*/
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* ProjectileMesh;

	/**
	* 伤害内半径：爆炸伤害的内半径，此范围内受到全额伤害
	*/
	UPROPERTY(EditAnywhere)
	float DamageInnerRadius = 200.f;

	/**
	* 伤害外半径：爆炸伤害的外半径，此范围内伤害逐渐递减
	*/
	UPROPERTY(EditAnywhere)
	float DamageOuterRadius = 500.f;

private:

	/**
	* 追踪粒子效果：投射物飞行时的追踪粒子效果
	*/
	UPROPERTY(EditAnywhere)
	UParticleSystem* Tracer;

	/**
	* 追踪粒子组件：追踪粒子效果的实例化组件
	*/
	UPROPERTY()
	class UParticleSystemComponent* TracerComponent;

	/**
	* 销毁定时器句柄：用于管理投射物的自动销毁定时器
	*/
	FTimerHandle DestroyTimer;

	/**
	* 销毁时间：投射物从生成到自动销毁的时间（秒）
	*/
	UPROPERTY(EditAnywhere)
	float DestroyTime = 3.f;
public:	
};
