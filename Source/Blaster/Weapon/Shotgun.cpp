// Fill out your copyright notice in the Description page of Project Settings.


#include "Shotgun.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "Kismet/GameplayStatics.h"
#include "particles/ParticleSystemComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"

/**
 * 霰弹枪发射函数
 * 处理霰弹枪的发射逻辑，包括多弹丸追踪、伤害计算和网络同步
 * @param HitTargets - 命中目标位置数组，每个位置对应一个弹丸的目标点
 */
void AShotgun::FireShotgun(const TArray<FVector_NetQuantize>& HitTargets)
{
	// 调用基类的Fire函数，播放射击动画、生成弹壳并消耗弹药
	AWeapon::Fire(FVector());
	// 获取武器所有者
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	if (OwnerPawn == nullptr) return;
	// 获取控制器引用
	AController* InstigatorController = OwnerPawn->GetController();

	// 获取枪口闪光插槽
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket)
	{
		// 获取插槽的变换（位置和旋转）
		const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// 设置射线起点为枪口位置
		const FVector Start = SocketTransform.GetLocation();

		// 创建两个映射，分别记录普通命中和爆头命中的角色及次数
		TMap<ABlasterCharacter*, uint32> HitMap;
		TMap<ABlasterCharacter*, uint32> HeadShotHitMap;
		// 遍历每个弹丸的目标位置
		for (FVector_NetQuantize HitTarget : HitTargets)
		{
			// 执行射线检测
			FHitResult FireHit;
			WeaponTraceHit(Start, HitTarget, FireHit);

			// 尝试将命中的Actor转换为游戏角色
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(FireHit.GetActor());
			if (BlasterCharacter)
			{
				// 判断是否击中头部
				const bool bHeadShot = FireHit.BoneName.ToString() == FString("head");

				// 根据命中部位将结果记录到对应的映射中
				if (bHeadShot)
				{
					if (HeadShotHitMap.Contains(BlasterCharacter)) 
						HeadShotHitMap[BlasterCharacter]++; // 已存在则增加计数
					else 
						HeadShotHitMap.Emplace(BlasterCharacter, 1); // 不存在则添加新条目
				}
				else
				{
					if (HitMap.Contains(BlasterCharacter)) 
						HitMap[BlasterCharacter]++; // 已存在则增加计数
					else 
						HitMap.Emplace(BlasterCharacter, 1); // 不存在则添加新条目
				}

				// 播放命中特效
				if (ImpactParticles)
				{
					UGameplayStatics::SpawnEmitterAtLocation(
						GetWorld(),
						ImpactParticles,
						FireHit.ImpactPoint, // 特效生成位置
						FireHit.ImpactNormal.Rotation() // 特效旋转方向
					);
				}
				// 播放命中音效
				if (HitSound)
				{
					UGameplayStatics::PlaySoundAtLocation(
						this,
						HitSound,
						FireHit.ImpactPoint, // 音效播放位置
						.5f, // 音量
						FMath::FRandRange(-.5f, .5f) // 随机音高
					);
				}
			}
		}
		// 存储所有被命中的角色
		TArray<ABlasterCharacter*> HitCharacters;

		// 创建映射，记录每个角色受到的总伤害
		TMap<ABlasterCharacter*, float> DamageMap;

		// 计算普通命中的伤害并存储
		for (auto HitPair : HitMap)
		{
			if (HitPair.Key)
			{
				// 计算伤害：命中次数 × 普通伤害值
				DamageMap.Emplace(HitPair.Key, HitPair.Value * Damage);
				// 将角色添加到命中列表中（确保唯一）
				HitCharacters.AddUnique(HitPair.Key);
			}
		}

		// 计算爆头命中的伤害并存储
		for (auto HeadShotHitPair : HeadShotHitMap)
		{
			if (HeadShotHitPair.Key)
			{
				// 如果角色已经在映射中，添加爆头伤害
				if (DamageMap.Contains(HeadShotHitPair.Key)) 
					DamageMap[HeadShotHitPair.Key] += HeadShotHitPair.Value * HeadShotDamage;
				// 如果角色不在映射中，创建新条目
				else 
					DamageMap.Emplace(HeadShotHitPair.Key, HeadShotHitPair.Value * HeadShotDamage);
				// 将角色添加到命中列表中（确保唯一）
				HitCharacters.AddUnique(HeadShotHitPair.Key);
			}
		}

		// 应用计算好的伤害
		for (auto DamagePair : DamageMap)
		{
			if (DamagePair.Key && InstigatorController)
			{
				// 判断是否应该由当前实例造成权威伤害
				// 当不使用服务器端倒带或所有者是本地控制时，允许造成权威伤害
				bool bCauseAuthDamage = !bUseServerSideRewind || OwnerPawn->IsLocallyControlled();
				// 服务器端应用伤害
				if (HasAuthority() && bCauseAuthDamage)
				{
					UGameplayStatics::ApplyDamage(
						DamagePair.Key, // 被击中的角色
						DamagePair.Value, // 计算的总伤害
						InstigatorController,
						this,
						UDamageType::StaticClass()
					);
				}
			}
		}

		// 客户端处理（使用服务器端倒带）
		if (!HasAuthority() && bUseServerSideRewind)
		{
			// 获取或更新武器所有者角色引用
			BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(OwnerPawn) : BlasterOwnerCharacter;
			// 获取或更新玩家控制器引用
			BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(InstigatorController) : BlasterOwnerController;
			// 如果控制器、角色和滞后补偿组件都有效，且角色是本地控制的
			if (BlasterOwnerController && BlasterOwnerCharacter && BlasterOwnerCharacter->GetLagCompensation() && BlasterOwnerCharacter->IsLocallyControlled())
			{
				// 发送服务器端得分请求，包含滞后补偿信息
				BlasterOwnerCharacter->GetLagCompensation()->ShotgunServerScoreRequest(
					HitCharacters, // 被命中的角色
					Start, // 射线起点
					HitTargets, // 命中目标位置
					BlasterOwnerController->GetServerTime() - BlasterOwnerController->SingleTripTime // 补偿后的服务器时间
				);
			}
		}
	}
}

/**
 * 霰弹散射计算函数
 * 计算霰弹枪发射时多个弹丸的散射轨迹终点
 * @param HitTarget - 原始瞄准目标位置
 * @param HitTargets - 输出参数，用于存储散射后的多个弹丸目标位置
 */
void AShotgun::ShotgunTraceEndWithScatter(const FVector& HitTarget, TArray<FVector_NetQuantize>& HitTargets)
{
	// 获取枪口闪光插槽，用于确定射线起点
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return;

	// 获取插槽的变换（位置和旋转）
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	// 设置射线起点为枪口位置
	const FVector TraceStart = SocketTransform.GetLocation();

	// 计算从枪口到目标的归一化向量
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	// 计算散射球的中心位置（在射线方向上的指定距离）
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;

	// 为每颗弹丸生成一个随机的散射终点
	for (uint32 i = 0; i < NumberOfPellets; i++)
	{
		// 生成随机向量用于散射
		// 随机单位向量 × 随机半径（0到SphereRadius之间）
		const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
		// 计算弹丸的终点位置（球心位置 + 随机向量）
		const FVector EndLoc = SphereCenter + RandVec;
		// 计算从起点到终点的向量
		FVector ToEndLoc = EndLoc - TraceStart;
		// 调整终点位置以确保射线长度一致
		ToEndLoc = TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size();

		// 将计算好的终点位置添加到结果数组中
		HitTargets.Add(ToEndLoc);
	}
}
