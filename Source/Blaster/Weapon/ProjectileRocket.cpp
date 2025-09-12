// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileRocket.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "Sound/SoundCue.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "Components/AudioComponent.h"
#include "RocketMovementComponent.h"

/**
 * 构造函数：初始化火箭的核心组件和基本属性
 */
AProjectileRocket::AProjectileRocket()
{
	// 创建并初始化火箭的静态网格组件
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Rocket Mesh"));
	// 将网格组件附加到根组件（碰撞盒）
	ProjectileMesh->SetupAttachment(RootComponent);
	// 禁用网格组件的碰撞（碰撞检测由根组件的碰撞盒处理）
	ProjectileMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 创建并初始化火箭移动组件
	RocketMovementComponent = CreateDefaultSubobject<URocketMovementComponent>(TEXT("RocketMovementComponent"));
	// 设置火箭旋转跟随速度方向
	RocketMovementComponent->bRotationFollowsVelocity = true;
	// 启用火箭移动组件的网络复制
	RocketMovementComponent->SetIsReplicated(true);
}

/**
 * 游戏开始时的初始化函数：设置火箭的初始状态和启动音效
 */
void AProjectileRocket::BeginPlay()
{
	// 调用父类的BeginPlay()方法
	Super::BeginPlay();

	// 在客户端也绑定碰撞事件（服务器端的碰撞事件在父类已经绑定）
	if (!HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AProjectileRocket::OnHit);
	}

	// 生成火箭的尾迹系统效果
	SpawnTrailSystem();

	// 如果设置了循环音效和音效衰减设置，创建并播放循环音效
	if (ProjectileLoop && LoopingSoundAttenuation)
	{
		ProjectileLoopComponent = UGameplayStatics::SpawnSoundAttached(
			ProjectileLoop,
			GetRootComponent(),
			FName(),
			GetActorLocation(),
			EAttachLocation::KeepWorldPosition,
			false,
			1.f,
			1.f,
			0.f,
			LoopingSoundAttenuation,
			(USoundConcurrency*)nullptr,
			false
		);
	}
}

/**
 * 碰撞事件回调：当火箭与其他物体碰撞时调用，处理碰撞逻辑和爆炸效果
 * @param HitComp 碰撞组件
 * @param OtherActor 被碰撞的Actor
 * @param OtherComp 被碰撞的组件
 * @param NormalImpulse 碰撞产生的法线冲量
 * @param Hit 碰撞结果信息
 */
void AProjectileRocket::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 避免火箭击中自己
	if (OtherActor == GetOwner())
	{
		return;
	}
	// 执行爆炸伤害计算
	ExplodeDamage();

	// 启动销毁定时器
	StartDestroyTimer();

	// 如果设置了碰撞粒子效果，在碰撞位置播放粒子效果
	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	// 如果设置了碰撞音效，在碰撞位置播放音效
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
	// 隐藏火箭网格
	if (ProjectileMesh)
	{
		ProjectileMesh->SetVisibility(false);
	}
	// 禁用碰撞盒，防止重复触发碰撞
	if (CollisionBox)
	{
		CollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	// 停止尾迹特效
	if (TrailSystemComponent && TrailSystemComponent->GetSystemInstance())
	{
		TrailSystemComponent->GetSystemInstance()->Deactivate();
	}
	// 停止循环音效
	if (ProjectileLoopComponent && ProjectileLoopComponent->IsPlaying())
	{
		ProjectileLoopComponent->Stop();
	}
}

/**
 * 火箭销毁时的回调函数：当前为空实现，可根据需要添加清理逻辑
 */
void AProjectileRocket::Destroyed()
{

}