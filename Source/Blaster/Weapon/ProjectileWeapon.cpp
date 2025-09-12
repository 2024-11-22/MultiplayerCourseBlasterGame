// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Projectile.h"

/**
 * 发射投射物的函数实现
 * @param HitTarget 击中目标的位置向量
 */
void AProjectileWeapon::Fire(const FVector& HitTarget)
{
	// 调用父类的Fire函数，处理通用的开火逻辑（如消耗弹药、播放动画等）
	Super::Fire(HitTarget);

	// 获取武器所有者（发射者）
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	// 获取枪口闪光插座位置，用于确定投射物的生成位置
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	// 获取当前世界指针
	UWorld* World = GetWorld();
	
	// 确保插座存在且世界有效
	if (MuzzleFlashSocket && World)
	{
		// 获取枪口插座的变换信息（位置和旋转）
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// 计算从枪口到目标的向量
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		// 计算投射物的旋转方向（朝向目标）
		FRotator TargetRotation = ToTarget.Rotation();

		// 设置投射物生成参数
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = GetOwner();           // 设置投射物的所有者
		SpawnParams.Instigator = InstigatorPawn;  // 设置投射物的发起者（用于伤害系统）

		// 声明投射物指针
		AProjectile* SpawnedProjectile = nullptr;
		
		// 使用服务器端重绕（SSR）的情况
		if (bUseServerSideRewind)
		{
			// 服务器端处理逻辑
			if (InstigatorPawn->HasAuthority())
			{
				// 服务器端且是本地控制（主机）的情况
				if (InstigatorPawn->IsLocallyControlled())
				{
					// 生成可复制的普通投射物
					SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
					// 设置伤害值
					SpawnedProjectile->Damage = Damage;
					SpawnedProjectile->HeadShotDamage = HeadShotDamage;
				}
				// 服务器端但不是本地控制的情况（远程客户端控制的角色）
				else
				{
					// 生成支持服务器端重绕的投射物
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = true;
				}
			}
			// 客户端处理逻辑
			else
			{
				// 客户端且是本地控制的情况
				if (InstigatorPawn->IsLocallyControlled())
				{
					// 生成支持服务器端重绕的投射物
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = true;
					// 设置轨迹起点和初始速度，用于服务器端重绕计算
					SpawnedProjectile->TraceStart = SocketTransform.GetLocation();
					SpawnedProjectile->InitialVelocity = SpawnedProjectile->GetActorForwardVector() * SpawnedProjectile->InitialSpeed;
				}
				// 客户端但不是本地控制的情况（其他玩家的角色）
				else
				{
					// 生成不支持服务器端重绕的投射物
					SpawnedProjectile = World->SpawnActor<AProjectile>(ServerSideRewindProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
					SpawnedProjectile->bUseServerSideRewind = false;
				}
			}
		}
		// 不使用服务器端重绕的情况
		else
		{
			// 只有服务器有权限生成投射物
			if (InstigatorPawn->HasAuthority())
			{
				// 生成普通投射物
				SpawnedProjectile = World->SpawnActor<AProjectile>(ProjectileClass, SocketTransform.GetLocation(), TargetRotation, SpawnParams);
				SpawnedProjectile->bUseServerSideRewind = false;
				// 设置伤害值
				SpawnedProjectile->Damage = Damage;
				SpawnedProjectile->HeadShotDamage = HeadShotDamage;
			}
		}
	}
}