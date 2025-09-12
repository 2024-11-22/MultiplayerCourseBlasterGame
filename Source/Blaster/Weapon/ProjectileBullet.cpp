// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"

/**
 * 构造函数：初始化子弹项目的核心组件和属性
 */
AProjectileBullet::AProjectileBullet()
{
	// 创建并初始化投射物移动组件
	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	// 设置子弹旋转跟随速度方向
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	// 启用该组件的网络复制
	ProjectileMovementComponent->SetIsReplicated(true);
	// 设置初始速度和最大速度
	ProjectileMovementComponent->InitialSpeed = InitialSpeed;
	ProjectileMovementComponent->MaxSpeed = InitialSpeed;
}

#if WITH_EDITOR
/**
 * 在编辑器中修改属性时的回调函数
 * @param Event 属性变更事件，包含被修改的属性信息
 */
void AProjectileBullet::PostEditChangeProperty(FPropertyChangedEvent& Event)
{
	Super::PostEditChangeProperty(Event);

	// 获取被修改的属性名称
	FName PropertyName = Event.Property != nullptr ? Event.Property->GetFName() : NAME_None;
	// 如果修改的是InitialSpeed属性
	if (PropertyName == GET_MEMBER_NAME_CHECKED(AProjectileBullet, InitialSpeed))
	{
		// 确保移动组件存在
		if (ProjectileMovementComponent)
		{
			// 更新移动组件的初始速度和最大速度
			ProjectileMovementComponent->InitialSpeed = InitialSpeed;
			ProjectileMovementComponent->MaxSpeed = InitialSpeed;
		}
	}
}
#endif

/**
 * 处理子弹碰撞事件，包含伤害计算和服务器端重绕逻辑
 * @param HitComp 碰撞的组件
 * @param OtherActor 被碰撞的Actor
 * @param OtherComp 被碰撞的组件
 * @param NormalImpulse 碰撞的法向冲量
 * @param Hit 碰撞结果信息
 */
void AProjectileBullet::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 获取子弹所有者（发射者）角色
	ABlasterCharacter* OwnerCharacter = Cast<ABlasterCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		// 获取发射者的控制器
		ABlasterPlayerController* OwnerController = Cast<ABlasterPlayerController>(OwnerCharacter->Controller);
		if (OwnerController)
		{
			// 服务器端且不使用服务器端重绕的情况
			if (OwnerCharacter->HasAuthority() && !bUseServerSideRewind)
			{
				// 根据击中的骨骼判断是普通伤害还是爆头伤害
				const float DamageToCause = Hit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;

				// 应用伤害到被击中的Actor
				UGameplayStatics::ApplyDamage(OtherActor, DamageToCause, OwnerController, this, UDamageType::StaticClass());
				// 调用父类的OnHit函数处理剩余的碰撞逻辑
				Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
				return;
			}
			// 尝试将被击中的Actor转换为角色类型
			ABlasterCharacter* HitCharacter = Cast<ABlasterCharacter>(OtherActor);
			// 使用服务器端重绕的情况（本地控制的角色击中其他角色）
			if (bUseServerSideRewind && OwnerCharacter->GetLagCompensation() && OwnerCharacter->IsLocallyControlled() && HitCharacter)
			{
				// 向服务器发送得分请求，包含击中信息和时间信息以进行重绕处理
				OwnerCharacter->GetLagCompensation()->ProjectileServerScoreRequest(
					HitCharacter,        // 被击中的角色
					TraceStart,          // 轨迹起点
					InitialVelocity,     // 初始速度
					OwnerController->GetServerTime() - OwnerController->SingleTripTime  // 补偿网络延迟的时间戳
				);
			}
		}
	}

	// 如果上述逻辑都不满足，调用父类的OnHit函数
	Super::OnHit(HitComp, OtherActor, OtherComp, NormalImpulse, Hit);
}

/**
 * 游戏开始时的初始化函数
 */
void AProjectileBullet::BeginPlay()
{
	Super::BeginPlay();
	/*
	// 以下是弹道预测的注释代码，可用于实现子弹轨迹预测功能
	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithChannel = true;          // 使用通道进行追踪
	PathParams.bTraceWithCollision = true;        // 考虑碰撞
	PathParams.DrawDebugTime = 5.f;               // 调试轨迹显示时间
	PathParams.DrawDebugType = EDrawDebugTrace::ForDuration;  // 调试轨迹类型
	PathParams.LaunchVelocity = GetActorForwardVector() * InitialSpeed;  // 发射速度
	PathParams.MaxSimTime = 4.f;                  // 最大模拟时间
	PathParams.ProjectileRadius = 5.f;            // 投射物半径
	PathParams.SimFrequency = 30.f;               // 模拟频率
	PathParams.StartLocation = GetActorLocation();  // 起始位置
	PathParams.TraceChannel = ECollisionChannel::ECC_Visibility;  // 追踪通道
	PathParams.ActorsToIgnore.Add(this);          // 忽略自身碰撞

	FPredictProjectilePathResult PathResult;

	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
	*/
}
