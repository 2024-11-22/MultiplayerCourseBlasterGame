// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileGrenade.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

/**
 * 构造函数：初始化手榴弹的核心组件和基本属性
 */
AProjectileGrenade::AProjectileGrenade()
{
	// 创建并初始化手榴弹的静态网格组件
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Grenade Mesh"));
	// 将网格组件附加到根组件（碰撞盒）
	ProjectileMesh->SetupAttachment(RootComponent);
	// 禁用网格组件的碰撞（碰撞检测由根组件的碰撞盒处理）
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 创建并初始化投射物移动组件
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	// 设置投射物旋转跟随速度方向
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	// 启用投射物移动组件的网络复制
	ProjectileMovementComponent->SetIsReplicated(true);
	// 启用投射物的反弹效果
	ProjectileMovementComponent->bShouldBounce = true;
}

/**
 * 游戏开始时的初始化函数：设置手榴弹的初始状态和启动必要的组件
 */
void AProjectileGrenade::BeginPlay()
{
	// 调用AActor的BeginPlay()
	AActor::BeginPlay();

	// 生成手榴弹的尾迹系统效果
	SpawnTrailSystem();
	// 启动销毁定时器，设置手榴弹在一定时间后爆炸
	StartDestroyTimer();

	// 绑定投射物的反弹事件到OnBounce方法
	ProjectileMovementComponent->OnProjectileBounce.AddDynamic(this, &AProjectileGrenade::OnBounce);
}

/**
 * 反弹事件回调：当手榴弹与其他物体碰撞并反弹时调用
 * @param ImpactResult 碰撞结果信息
 * @param ImpactVelocity 碰撞时的速度
 */
void AProjectileGrenade::OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity)
{
	// 如果设置了反弹音效，在碰撞位置播放音效
	if (BounceSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			BounceSound,
			GetActorLocation()
		);
	}
}

/**
 * 手榴弹销毁时的回调函数：处理手榴弹爆炸和特效播放
 */
void AProjectileGrenade::Destroyed()
{
	// 执行爆炸伤害计算
	ExplodeDamage();
	// 调用父类的Destroyed()方法播放碰撞特效和音效
	Super::Destroyed();
}