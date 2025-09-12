// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Blaster.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

/**
 * 构造函数：初始化投射物的核心组件和基本属性
 */
AProjectile::AProjectile()
{
	// 启用Actor的每帧更新
	PrimaryActorTick.bCanEverTick = true;
	// 启用Actor的网络复制
	bReplicates = true;

	// 创建并初始化碰撞盒组件
	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	// 设置碰撞盒为根组件
	SetRootComponent(CollisionBox);
	// 设置碰撞对象类型为世界动态物体
	CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	// 启用碰撞查询和物理响应
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// 默认忽略所有碰撞通道
	CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	// 设置对可见性通道的碰撞响应为阻挡
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	// 设置对世界静态物体的碰撞响应为阻挡
	CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
	// 设置对骨骼网格物体的碰撞响应为阻挡
	CollisionBox->SetCollisionResponseToChannel(ECC_SkeletalMesh, ECollisionResponse::ECR_Block);
}

/**
 * 游戏开始时的初始化函数：设置投射物的初始状态和启动必要的组件
 */
void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	
	// 如果设置了追踪粒子效果，生成追踪粒子
	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition
		);
	}

	// 只有服务器端有权限绑定碰撞事件
	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectile::OnHit);
	}
}

/**
 * 碰撞事件回调：当投射物与其他物体碰撞时调用
 * @param HitComp 碰撞组件
 * @param OtherActor 被碰撞的Actor
 * @param OtherComp 被碰撞的组件
 * @param NormalImpulse 碰撞产生的法线冲量
 * @param Hit 碰撞结果信息
 */
void AProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 默认的碰撞处理是销毁投射物，子类可以重写此方法以实现特定的碰撞行为
	Destroy();
}

/**
 * 生成尾迹系统：在投射物飞行时生成视觉尾迹效果
 */
void AProjectile::SpawnTrailSystem()
{
	// 如果设置了尾迹系统，生成尾迹效果
	if (TrailSystem)
	{
		TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			TrailSystem,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition,
			false
		);
	}
}

/**
 * 爆炸伤害计算：处理投射物爆炸时的范围伤害
 */
void AProjectile::ExplodeDamage()
{
	// 获取发射者（通常是玩家角色）
	APawn* FiringPawn = GetInstigator();
	// 只有服务器端有权限应用伤害
	if (FiringPawn && HasAuthority())
	{
		// 获取发射者的控制器
		AController* FiringController = FiringPawn->GetController();
		if (FiringController)
		{
			// 应用带衰减的范围伤害
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				this, // 世界上下文对象
				Damage, // 基础伤害
				10.f, // 最小伤害
				GetActorLocation(), // 爆炸原点
				DamageInnerRadius, // 伤害内半径
				DamageOuterRadius, // 伤害外半径
				1.f, // 伤害衰减系数
				UDamageType::StaticClass(), // 伤害类型
				TArray<AActor*>(), // 忽略的Actor列表
				this, // 伤害造成者
				FiringController // 伤害发起者的控制器
			);
		}
	}
}

/**
 * 每帧更新函数：处理投射物的帧更新逻辑
 * @param DeltaTime 帧间隔时间
 */
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

/**
 * 启动销毁定时器：设置投射物在一定时间后自动销毁
 */
void AProjectile::StartDestroyTimer()
{
	// 设置定时器，在DestroyTime秒后调用DestroyTimerFinished方法
	GetWorldTimerManager().SetTimer(
		DestroyTimer,
		this,
		&AProjectile::DestroyTimerFinished,
		DestroyTime
	);
}

/**
 * 销毁定时器结束回调：当销毁定时器结束时调用，销毁投射物
 */
void AProjectile::DestroyTimerFinished()
{
	// 销毁投射物
	Destroy();
}

/**
 * 投射物销毁时的回调函数：处理投射物销毁时的特效播放和清理工作
 */
void AProjectile::Destroyed()
{
	Super::Destroyed();

	// 如果设置了碰撞粒子效果，在投射物销毁位置播放粒子效果
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	// 如果设置了碰撞音效，在投射物销毁位置播放音效
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}

