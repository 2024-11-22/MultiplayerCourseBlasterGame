// Fill out your copyright notice in the Description page of Project Settings.


#include "HitScanWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "WeaponTypes.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"

/**
 * 发射武器的函数，使用射线追踪实现即时命中检测
 * @param HitTarget 击中目标的位置向量
 */
void AHitScanWeapon::Fire(const FVector& HitTarget)
{
	// 调用父类的Fire函数，执行基础逻辑
	Super::Fire(HitTarget);

	// 获取武器所有者（角色）
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr)
	{
		return;
	}
	
	// 获取武器所有者的控制器
	AController* InstigatorController = OwnerPawn->GetController();

	// 获取武器网格体上的枪口闪光Socket
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		// 获取Socket的变换信息
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// 设置射线起点为Socket的位置
		FVector Start = SocketTransform.GetLocation();

		// 存储射线命中结果
		FHitResult FireHit;
		// 执行射线追踪检测
		WeaponTraceHit(Start, HitTarget, FireHit);

		// 尝试将命中的Actor转换为BlasterCharacter
		ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
		if (BlasterCharacter && InstigatorController)
		{
			// 判断是否应该造成权威伤害：不使用服务器端重绕 或 武器所有者受本地控制
			bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
			// 如果有服务器权限且应该造成权威伤害
			if (HasAuthority() && bCauseAuthDamage)
			{
				// 根据命中的骨骼判断是普通伤害还是爆头伤害
				const float DamageToCause = FireHit.BoneName.ToString() == FString("head") ? HeadShotDamage : Damage;

				// 应用伤害
				UGameplayStatics::ApplyDamage(
					BlasterCharacter,
					DamageToCause,
					InstigatorController,
					this,
					UDamageType::StaticClass()
				);
			}
			// 如果没有服务器权限但使用了服务器端重绕
			if (!HasAuthority() && bUseServerSideRewind)
			{
				// 缓存所有者角色和控制器引用
				BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
				BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerController;
				// 如果所有必要引用都有效且所有者受本地控制
				if (BlasterOwnerController && BlasterOwnerCharacter && BlasterOwnerCharacter->GetLagCompensation() && BlasterOwnerCharacter->IsLocallyControlled())
				{
					// 向服务器发送得分请求，包含命中信息和时间戳
					BlasterOwnerCharacter->GetLagCompensation()->ServerScoreRequest(
						BlasterCharacter,
						Start,
						HitTarget,
						BlasterOwnerController->GetServerTime() - BlasterOwnerController->SingleTripTime
					);
				}
			}
		}
		// 如果设置了命中粒子效果，在命中点生成效果
		if (ImpactParticles)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				ImpactParticles,
				FireHit.ImpactPoint,
				FireHit.ImpactNormal.Rotation()
			);
		}
		// 如果设置了命中音效，在命中点播放音效
		if (HitSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				HitSound,
				FireHit.ImpactPoint
			);
		}

		// 如果设置了枪口闪光粒子效果，在枪口生成效果
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(
				GetWorld(),
				MuzzleFlash,
				SocketTransform
			);
		}
		// 如果设置了开火音效，在武器位置播放音效
		if (FireSound)
		{
			UGameplayStatics::PlaySoundAtLocation(
				this,
				FireSound,
				GetActorLocation()
			);
		}
	}
}

/**
 * 执行武器射线追踪，检测命中结果
 * @param TraceStart 射线起点
 * @param HitTarget 射线终点
 * @param OutHit 输出参数，用于存储命中结果
 */
void AHitScanWeapon::WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit)
{
	// 获取当前世界指针
	UWorld* World = GetWorld();
	if (World)
	{
		// 设置射线终点，稍微超出目标点以确保命中
		FVector End = TraceStart + (HitTarget - TraceStart) * 1.25f;

		// 执行射线追踪，使用可见性通道
		World->LineTraceSingleByChannel(
			OutHit,
			TraceStart,
			End,
			ECollisionChannel::ECC_Visibility
		);
		// 设置光束粒子的终点
		FVector BeamEnd = End;
		if (OutHit.bBlockingHit)
		{
			// 如果射线命中了物体，光束终点设为命中点
			BeamEnd = OutHit.ImpactPoint;
		}
		else
		{
			// 如果射线没有命中物体，命中点设为射线终点
			OutHit.ImpactPoint = End;
		}

		// 如果设置了光束粒子效果，生成光束
		if (BeamParticles)
		{
			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(
				World,
				BeamParticles,
				TraceStart,
				FRotator::ZeroRotator,
				true
			);
			if (Beam)
			{
				// 设置光束粒子的目标参数
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}
	}
}