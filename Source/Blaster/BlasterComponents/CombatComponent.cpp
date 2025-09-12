// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Blaster/Character/BlasterAnimInstance.h"
#include "Blaster/Weapon/Projectile.h"
#include "Blaster/Weapon/Shotgun.h"

/**
 * 战斗组件构造函数
 * 初始化组件基本属性和默认值
 */
UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f; // 基础移动速度
	AimWalkSpeed = 450.f; // 瞄准状态下的移动速度
}

/**
 * 设置组件的网络复制属性
 * @param OutLifetimeProps 输出参数，用于存储需要复制的属性
 */
void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon); // 复制当前装备的武器
	DOREPLIFETIME(UCombatComponent, SecondaryWeapon); // 复制次要武器
	DOREPLIFETIME(UCombatComponent, bAiming); // 复制瞄准状态
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly); // 仅向所有者复制携带的弹药
	DOREPLIFETIME(UCombatComponent, CombatState); // 复制战斗状态
	DOREPLIFETIME(UCombatComponent, Grenades); // 复制手榴弹数量
	DOREPLIFETIME(UCombatComponent, bHoldingTheFlag); // 复制是否持有旗帜状态
}

/**
 * 霰弹枪单颗子弹装填逻辑
 * 由服务器执行霰弹枪的单颗子弹装填，并更新弹药数量
 */
void UCombatComponent::ShotgunShellReload()
{
	if (Character && Character->HasAuthority())
	{
		UpdateShotgunAmmoValues(); // 更新霰弹枪的弹药值
	}
}

/**
 * 拾取弹药函数
 * @param WeaponType 武器类型
 * @param AmmoAmount 拾取的弹药数量
 */
void UCombatComponent::PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount)
{
	if (CarriedAmmoMap.Contains(WeaponType)) // 检查是否支持该武器类型的弹药
	{
		// 更新携带的弹药数量，确保不超过最大携带量
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmount, 0, MaxCarriedAmmo);
		UpdateCarriedAmmo(); // 更新HUD上显示的弹药数量
	}
	// 如果当前装备的武器为空且与拾取的弹药类型匹配，则自动装填
	if (EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}
}

/**
 * 组件初始化函数
 * 在游戏开始时执行，设置角色移动速度、相机FOV和初始弹药
 */
void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed; // 设置角色基础移动速度

		// 初始化相机FOV设置
		if (Character->GetFollowCamera())
		{
			DefaultFOV = Character->GetFollowCamera()->FieldOfView; // 保存默认FOV
			CurrentFOV = DefaultFOV; // 设置当前FOV为默认值
		}
		// 仅服务器端初始化携带的弹药
		if (Character->HasAuthority())
		{
			InitializeCarriedAmmo();
		}
	}
}

/**
 * 组件每帧更新函数
 * @param DeltaTime 帧间隔时间
 * @param TickType 帧类型
 * @param ThisTickFunction 帧函数指针
 */
void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 仅本地控制的角色执行以下操作
	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult); // 执行准星下的射线追踪
		HitTarget = HitResult.ImpactPoint; // 保存命中点

		SetHUDCrosshairs(DeltaTime); // 更新HUD准星
		InterpFOV(DeltaTime); // 插值更新相机FOV
	}
}

/**
 * 处理开火按钮按下事件
 * @param bPressed 是否按下按钮
 */
void UCombatComponent::FireButtonPressed(bool bPressed)
{
	bFireButtonPressed = bPressed; // 更新按钮状态
	if (bFireButtonPressed)
	{
		Fire(); // 执行开火
	}
}

/**
 * 通用开火函数
 * 根据当前装备武器的类型执行相应的开火逻辑
 */
void UCombatComponent::Fire()
{
	if (CanFire()) // 检查是否可以开火
	{
		bCanFire = false; // 暂时禁用开火，防止连续快速开火
		if (EquippedWeapon)
		{
			CrosshairShootingFactor = .75f; // 设置射击时准星扩散因子

			// 根据武器的射击类型执行不同的开火逻辑
			switch (EquippedWeapon->FireType)
			{
			case EFireType::EFT_Projectile: // 投射物武器
				FireProjectileWeapon();
				break;
			case EFireType::EFT_HitScan: // 射线武器
				FireHitScanWeapon();
				break;
			case EFireType::EFT_Shotgun: // 霰弹枪
				FireShotgun();
				break;
			}
		}
		StartFireTimer(); // 启动开火冷却计时器
	}
}

/**
 * 投射物武器开火函数
 * 处理投射物类型武器（如火箭发射器）的开火逻辑
 */
void UCombatComponent::FireProjectileWeapon()
{
	if (EquippedWeapon && Character)
	{
		// 如果武器使用散射，则计算散射后的目标点
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		// 非服务器端执行本地开火效果
		if (!Character->HasAuthority())
		{
			// 本地预测
			LocalFire(HitTarget);
		}
		// 向服务器发送开火请求
		ServerFire(HitTarget, EquippedWeapon->FireDelay);
	}
}

/**
 * 射线武器开火函数
 * 处理射线类型武器（如突击步枪）的开火逻辑
 */
void UCombatComponent::FireHitScanWeapon()
{
	if (EquippedWeapon && Character)
	{
		// 如果武器使用散射，则计算散射后的目标点
		HitTarget = EquippedWeapon->bUseScatter ? EquippedWeapon->TraceEndWithScatter(HitTarget) : HitTarget;
		// 非服务器端执行本地开火效果
		if (!Character->HasAuthority()) LocalFire(HitTarget);
		// 向服务器发送开火请求
		ServerFire(HitTarget, EquippedWeapon->FireDelay);
	}
}

/**
 * 霰弹枪开火函数
 * 处理霰弹枪的开火逻辑，计算多个弹丸的命中点
 */
void UCombatComponent::FireShotgun()
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun && Character)
	{
		TArray<FVector_NetQuantize> HitTargets; // 存储霰弹枪多个弹丸的命中点
		// 计算霰弹散射后的多个命中点
		Shotgun->ShotgunTraceEndWithScatter(HitTarget, HitTargets);
		// 非服务器端执行本地霰弹枪开火效果
		if (!Character->HasAuthority()) ShotgunLocalFire(HitTargets);
		// 向服务器发送霰弹枪开火请求
		ServerShotgunFire(HitTargets, EquippedWeapon->FireDelay);
	}
}

/**
 * 启动开火冷却计时器
 * 设置一个计时器，在冷却时间后重新启用开火功能
 */
void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	// 设置计时器，在武器的射击延迟后调用FireTimerFinished函数
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

/**
 * 开火计时器结束回调函数
 * 重新启用开火功能，并处理自动武器的连续射击和空弹夹自动装填
 */
void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;
	bCanFire = true; // 重新启用开火功能
	// 如果开火按钮仍被按住且武器是自动的，则继续射击
	if (bFireButtonPressed && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
	ReloadEmptyWeapon(); // 检查并装填空武器
}

/**
 * 服务器端开火实现函数
 * @param TraceHitTarget 命中目标位置
 * @param FireDelay 射击延迟时间
 */
void UCombatComponent::ServerFire_Implementation(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	// 向所有客户端广播开火事件
	MulticastFire(TraceHitTarget);
}

/**
 * 服务器端开火验证函数
 * 验证客户端发送的开火请求是否有效
 * @param TraceHitTarget 命中目标位置
 * @param FireDelay 射击延迟时间
 * @return 是否验证通过
 */
bool UCombatComponent::ServerFire_Validate(const FVector_NetQuantize& TraceHitTarget, float FireDelay)
{
	if (EquippedWeapon)
	{
		// 验证客户端发送的射击延迟是否与服务器端的匹配
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}

/**
 * 多播开火函数
 * 向所有客户端广播开火事件，执行本地开火效果
 * @param TraceHitTarget 命中目标位置
 */
void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	// 避免在非服务器端的本地客户端重复执行
	// 因为已经进行了本地预测，所以不需要再进行一次
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority())
	{
		return;
	}
	
	// ROLE_SimulatedProxy执行本地开火效果
	LocalFire(TraceHitTarget);
}

/**
 * 服务器端霰弹枪开火实现函数
 * @param TraceHitTargets 霰弹枪多个弹丸的命中目标位置数组
 * @param FireDelay 射击延迟时间
 */
void UCombatComponent::ServerShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	// 向所有客户端广播霰弹枪开火事件
	MulticastShotgunFire(TraceHitTargets);
}

/**
 * 服务器端霰弹枪开火验证函数
 * 验证客户端发送的霰弹枪开火请求是否有效
 * @param TraceHitTargets 霰弹枪多个弹丸的命中目标位置数组
 * @param FireDelay 射击延迟时间
 * @return 是否验证通过
 */
bool UCombatComponent::ServerShotgunFire_Validate(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay)
{
	if (EquippedWeapon)
	{
		// 验证客户端发送的射击延迟是否与服务器端的匹配
		bool bNearlyEqual = FMath::IsNearlyEqual(EquippedWeapon->FireDelay, FireDelay, 0.001f);
		return bNearlyEqual;
	}
	return true;
}

/**
 * 多播霰弹枪开火函数
 * 向所有客户端广播霰弹枪开火事件，执行本地霰弹枪开火效果
 * @param TraceHitTargets 霰弹枪多个弹丸的命中目标位置数组
 */
void UCombatComponent::MulticastShotgunFire_Implementation(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	// 避免在非服务器端的本地客户端重复执行
	if (Character && Character->IsLocallyControlled() && !Character->HasAuthority()) return;
	// 执行本地霰弹枪开火效果
	ShotgunLocalFire(TraceHitTargets);
}

/**
 * 本地开火函数
 * 执行武器的本地开火效果，包括播放动画和武器特效
 * @param TraceHitTarget 命中目标位置
 */
void UCombatComponent::LocalFire(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	// 只有当角色处于未占用状态时才能开火
	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming); // 播放开火动画蒙太奇
		EquippedWeapon->Fire(TraceHitTarget); // 执行武器开火逻辑
	}
}

/**
 * 本地霰弹枪开火函数
 * 执行霰弹枪的本地开火效果
 * @param TraceHitTargets 霰弹枪多个弹丸的命中目标位置数组
 */
void UCombatComponent::ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets)
{
	AShotgun* Shotgun = Cast<AShotgun>(EquippedWeapon);
	if (Shotgun == nullptr || Character == nullptr) return;
	// 霰弹枪可以在装填状态或未占用状态下开火
	if (CombatState == ECombatState::ECS_Reloading || CombatState == ECombatState::ECS_Unoccupied)
	{
		bLocallyReloading = false;
		Character->PlayFireMontage(bAiming); // 播放开火动画蒙太奇
		Shotgun->FireShotgun(TraceHitTargets); // 执行霰弹枪开火逻辑
		CombatState = ECombatState::ECS_Unoccupied; // 设置战斗状态为未占用
	}
}

/**
 * 装备武器函数
 * 根据武器类型和当前装备状态装备武器
 * @param WeaponToEquip 要装备的武器
 */
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return; // 只有未占用状态才能装备武器

	// 特殊处理旗帜武器
	if (WeaponToEquip->GetWeaponType() == EWeaponType::EWT_Flag)
	{
		Character->Crouch(); // 装备旗帜时角色下蹲
		bHoldingTheFlag = true; // 设置持有旗帜状态
		WeaponToEquip->SetWeaponState(EWeaponState::EWS_Equipped); // 设置武器状态
		AttachFlagToLeftHand(WeaponToEquip); // 附加旗帜到左手
		WeaponToEquip->SetOwner(Character); // 设置武器所有者
		TheFlag = WeaponToEquip; // 保存旗帜引用
	}
	else
	{
		// 根据当前装备情况决定装备为主武器还是副武器
		if (EquippedWeapon != nullptr && SecondaryWeapon == nullptr)
		{
			EquipSecondaryWeapon(WeaponToEquip); // 装备为副武器
		}
		else
		{
			EquipPrimaryWeapon(WeaponToEquip); // 装备为主武器
		}

		// 设置角色旋转方式
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

/**
 * 交换武器函数
 * 交换主武器和副武器
 */
void UCombatComponent::SwapWeapons()
{
	// 只有在未占用状态、角色存在且有权威的情况下才能交换武器
	if (CombatState != ECombatState::ECS_Unoccupied || Character == nullptr || !Character->HasAuthority()) return;

	Character->PlaySwapMontage(); // 播放武器交换动画
	CombatState = ECombatState::ECS_SwappingWeapons; // 设置战斗状态为交换武器中
	Character->bFinishedSwapping = false; // 重置交换完成标志
	if (SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(false); // 禁用副武器的自定义深度
}

/**
 * 装备主武器函数
 * 将武器装备为主武器
 * @param WeaponToEquip 要装备的武器
 */
void UCombatComponent::EquipPrimaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	DropEquippedWeapon(); // 放下当前装备的武器
	EquippedWeapon = WeaponToEquip; // 设置新武器为主武器
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped); // 设置武器状态
	AttachActorToRightHand(EquippedWeapon); // 将武器附加到右手
	EquippedWeapon->SetOwner(Character); // 设置武器所有者
	EquippedWeapon->SetHUDAmmo(); // 更新HUD上的弹药显示
	UpdateCarriedAmmo(); // 更新携带的弹药显示
	PlayEquipWeaponSound(WeaponToEquip); // 播放装备武器音效
	ReloadEmptyWeapon(); // 如果武器为空则自动装填
}

/**
 * 装备副武器函数
 * 将武器装备为副武器
 * @param WeaponToEquip 要装备的武器
 */
void UCombatComponent::EquipSecondaryWeapon(AWeapon* WeaponToEquip)
{
	if (WeaponToEquip == nullptr) return;
	SecondaryWeapon = WeaponToEquip; // 设置武器为副武器
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary); // 设置武器状态
	AttachActorToBackpack(WeaponToEquip); // 将武器附加到背包
	PlayEquipWeaponSound(WeaponToEquip); // 播放装备武器音效
	SecondaryWeapon->SetOwner(Character); // 设置武器所有者
}

/**
 * 瞄准状态复制回调函数
 * 当服务器复制瞄准状态到客户端时调用
 */
void UCombatComponent::OnRep_Aiming()
{
	if (Character && Character->IsLocallyControlled())
	{
		bAiming = bAimButtonPressed; // 确保本地客户端的瞄准状态与按钮状态一致
	}
}

/**
 * 放下已装备的武器函数
 * 处理放下当前装备的武器的逻辑
 */
void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped(); // 调用武器的放下方法
	}
}

/**
 * 将Actor附加到右手函数
 * 将指定的Actor附加到角色的右手骨骼套接字
 * @param ActorToAttach 要附加的Actor
 */
void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	// 获取右手套接字
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh()); // 将Actor附加到套接字
	}
}

/**
 * 将旗帜附加到左手函数
 * 将旗帜武器附加到角色的旗帜套接字
 * @param Flag 要附加的旗帜武器
 */
void UCombatComponent::AttachFlagToLeftHand(AWeapon* Flag)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || Flag == nullptr) return;
	// 获取旗帜套接字
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("FlagSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(Flag, Character->GetMesh()); // 将旗帜附加到套接字
	}
}

/**
 * 将Actor附加到左手函数
 * 根据当前装备的武器类型，将Actor附加到合适的左手套接字
 * @param ActorToAttach 要附加的Actor
 */
void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr || EquippedWeapon == nullptr) return;
	// 确定是否使用手枪套接字
	bool bUsePistolSocket = 
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol ||
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SubmachineGun;
	// 选择合适的套接字名称
	FName SocketName = bUsePistolSocket ? FName("PistolSocket") : FName("LeftHandSocket");
	// 获取选定的套接字
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(SocketName);
	if (HandSocket)
	{
		HandSocket->AttachActor(ActorToAttach, Character->GetMesh()); // 将Actor附加到套接字
	}
}

/**
 * 将Actor附加到背包函数
 * 将Actor附加到角色的背包套接字
 * @param ActorToAttach 要附加的Actor
 */
void UCombatComponent::AttachActorToBackpack(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	// 获取背包套接字
	const USkeletalMeshSocket* BackpackSocket = Character->GetMesh()->GetSocketByName(FName("BackpackSocket"));
	if (BackpackSocket)
	{
		BackpackSocket->AttachActor(ActorToAttach, Character->GetMesh()); // 将Actor附加到套接字
	}
}

/**
 * 更新携带的弹药函数
 * 更新当前携带的弹药数量并同步到HUD显示
 */
void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;
	// 如果弹药映射中包含当前武器类型的弹药
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()]; // 更新当前携带的弹药
	}

	// 确保控制器有效
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo); // 更新HUD上的携带弹药显示
	}
}

/**
 * 播放装备武器音效函数
 * 播放指定武器的装备音效
 * @param WeaponToEquip 要播放音效的武器
 */
void UCombatComponent::PlayEquipWeaponSound(AWeapon* WeaponToEquip)
{
	if (Character && WeaponToEquip && WeaponToEquip->EquipSound)
	{
		// 在角色位置播放装备音效
		UGameplayStatics::PlaySoundAtLocation(
			this,
			WeaponToEquip->EquipSound,
			Character->GetActorLocation()
		);
	}
}

/**
 * 空武器自动装填函数
 * 当当前装备的武器弹夹为空时自动触发装填
 */
void UCombatComponent::ReloadEmptyWeapon()
{
	if (EquippedWeapon && EquippedWeapon->IsEmpty()) // 如果武器弹夹为空
	{
		Reload(); // 执行装填
	}
}

/**
 * 通用装填函数
 * 处理武器装填逻辑，包括服务器验证和本地处理
 */
void UCombatComponent::Reload()
{
	// 检查是否满足装填条件：有弹药、处于未占用状态、武器存在、武器未满、不在本地装填中
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied && EquippedWeapon && !EquippedWeapon->IsFull() && !bLocallyReloading)
	{
		ServerReload(); // 向服务器发送装填请求
		HandleReload(); // 处理本地装填逻辑
		bLocallyReloading = true; // 设置本地装填标志
	}
}

/**
 * 服务器端装填实现函数
 * 处理服务器端的装填逻辑
 */
void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_Reloading; // 设置战斗状态为装填中
	if (!Character->IsLocallyControlled()) HandleReload(); // 非本地控制的角色执行本地装填逻辑
}

/**
 * 完成装填函数
 * 装填动画完成后调用，更新弹药数量并恢复战斗状态
 */
void UCombatComponent::FinishReloading()
{
	if (Character == nullptr) return;
	bLocallyReloading = false; // 重置本地装填标志
	if (Character->HasAuthority()) // 只有服务器有权限更新状态和弹药
	{
		CombatState = ECombatState::ECS_Unoccupied; // 恢复战斗状态为未占用
		UpdateAmmoValues(); // 更新弹药值
	}
	if (bFireButtonPressed) // 如果开火按钮仍被按住，则开火
	{
		Fire();
	}
}

/**
 * 完成武器交换函数
 * 武器交换动画完成后调用，恢复战斗状态
 */
void UCombatComponent::FinishSwap()
{
	if (Character && Character->HasAuthority()) // 只有服务器有权限更新状态
	{
		CombatState = ECombatState::ECS_Unoccupied; // 恢复战斗状态为未占用
	}
	if (Character) Character->bFinishedSwapping = true; // 设置交换完成标志
	if (SecondaryWeapon) SecondaryWeapon->EnableCustomDepth(true); // 启用副武器的自定义深度
}

/**
 * 完成武器交换并附加武器函数
 * 在武器交换动画的适当时间点调用，实际执行武器交换逻辑
 */
void UCombatComponent::FinishSwapAttachWeapons()
{
	PlayEquipWeaponSound(SecondaryWeapon); // 播放副武器的装备音效

	if (Character == nullptr || !Character->HasAuthority()) return; // 只有服务器有权限执行交换
	// 交换主武器和副武器
	AWeapon* TempWeapon = EquippedWeapon;
	EquippedWeapon = SecondaryWeapon;
	SecondaryWeapon = TempWeapon;

	// 更新主武器状态和位置
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();

	// 更新副武器状态和位置
	SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary);
	AttachActorToBackpack(SecondaryWeapon);
}

/**
 * 更新弹药值函数
 * 计算并更新武器和携带的弹药数量
 */
void UCombatComponent::UpdateAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	int32 ReloadAmount = AmountToReload(); // 计算需要装填的弹药数量
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmount; // 减少携带的弹药
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()]; // 更新当前携带的弹药
	}
	// 确保控制器有效
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo); // 更新HUD上的携带弹药显示
	}
	EquippedWeapon->AddAmmo(ReloadAmount); // 向武器添加弹药
}

/**
 * 更新霰弹枪弹药值函数
 * 处理霰弹枪单颗子弹装填的弹药更新
 */
void UCombatComponent::UpdateShotgunAmmoValues()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= 1; // 减少1颗携带的霰弹
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()]; // 更新当前携带的弹药
	}
	// 确保控制器有效
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo);
	}
	EquippedWeapon->AddAmmo(1);
	bCanFire = true;
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}
}

/**
 * 手榴弹数量复制回调函数
 * 当服务器复制手榴弹数量到客户端时调用
 */
void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenades(); // 更新HUD上的手榴弹显示
}

/**
 * 跳转到霰弹枪装填动画结尾函数
 * 在霰弹枪装填过程中需要立即开火时调用，跳过剩余装填动画
 */
void UCombatComponent::JumpToShotgunEnd()
{
	// 获取角色的动画实例
	UAnimInstance* AnimInstance = Character->GetMesh()->GetAnimInstance();
	if (AnimInstance && Character->GetReloadMontage())
	{
		// 跳转到霰弹枪装填动画的结尾部分
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

/**
 * 投掷手榴弹完成函数
 * 手榴弹投掷动画完成后调用，恢复战斗状态并重新装备武器
 */
void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied; // 恢复战斗状态为未占用
	AttachActorToRightHand(EquippedWeapon); // 将武器重新附加到右手
}

/**
 * 发射手榴弹函数
 * 处理手榴弹的发射逻辑
 */
void UCombatComponent::LaunchGrenade()
{
	ShowAttachedGrenade(false); // 隐藏附加的手榴弹
	if (Character && Character->IsLocallyControlled())
	{
		ServerLaunchGrenade(HitTarget); // 向服务器发送发射手榴弹请求
	}
}

/**
 * 服务器端发射手榴弹实现函数
 * 处理服务器端的手榴弹发射逻辑
 * @param Target 手榴弹的目标位置
 */
void UCombatComponent::ServerLaunchGrenade_Implementation(const FVector_NetQuantize& Target)
{
	if (Character && GrenadeClass && Character->GetAttachedGrenade())
	{
		const FVector StartingLocation = Character->GetAttachedGrenade()->GetComponentLocation(); // 获取手榴弹起始位置
		FVector ToTarget = Target - StartingLocation; // 计算到目标的方向
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character; // 设置所有者
		SpawnParams.Instigator = Character; // 设置发起者
		UWorld* World = GetWorld();
		if (World)
		{
			// 生成手榴弹投射物
			World->SpawnActor<AProjectile>(
				GrenadeClass,
				StartingLocation,
				ToTarget.Rotation(),
				SpawnParams
				);
		}
	}
}

/**
 * 战斗状态复制回调函数
 * 当服务器复制战斗状态到客户端时调用
 */
void UCombatComponent::OnRep_CombatState()
{
	// 根据战斗状态执行相应操作
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading: // 装填状态
		if (Character && !Character->IsLocallyControlled()) HandleReload(); // 非本地控制的角色执行装填
		break;
	case ECombatState::ECS_Unoccupied: // 未占用状态
		if (bFireButtonPressed) // 如果开火按钮被按住
		{
			Fire(); // 执行开火
		}
		break;
	case ECombatState::ECS_ThrowingGrenade: // 投掷手榴弹状态
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlayThrowGrenadeMontage(); // 播放投掷手榴弹动画
			AttachActorToLeftHand(EquippedWeapon); // 将武器附加到左手
			ShowAttachedGrenade(true); // 显示附加的手榴弹
		}
		break;
	case ECombatState::ECS_SwappingWeapons: // 交换武器状态
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlaySwapMontage(); // 播放武器交换动画
		}
		break;
	}
}

/**
 * 处理装填函数
 * 播放武器装填动画
 */
void UCombatComponent::HandleReload()
{
	if (Character)
	{
		Character->PlayReloadMontage(); // 播放装填动画
	}
}

/**
 * 计算需要装填的弹药数量函数
 * @return 需要装填的弹药数量
 */
int32 UCombatComponent::AmountToReload()
{
	if (EquippedWeapon == nullptr) return 0;
	int32 RoomInMag = EquippedWeapon->GetMagCapacity() - EquippedWeapon->GetAmmo(); // 弹夹中剩余空间

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		int32 AmountCarried = CarriedAmmoMap[EquippedWeapon->GetWeaponType()]; // 携带的弹药数量
		int32 Least = FMath::Min(RoomInMag, AmountCarried); // 取弹夹剩余空间和携带弹药的较小值
		return FMath::Clamp(RoomInMag, 0, Least); // 确保返回值在合理范围内
	}
	return 0;
}

/**
 * 投掷手榴弹函数
 * 处理手榴弹投掷的主逻辑
 */
void UCombatComponent::ThrowGrenade()
{
	if (Grenades == 0) return; // 无手榴弹可投掷
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) return; // 不在未占用状态或无装备武器
	CombatState = ECombatState::ECS_ThrowingGrenade; // 设置战斗状态为投掷手榴弹中
	if (Character)
	{
		Character->PlayThrowGrenadeMontage(); // 播放投掷手榴弹动画
		AttachActorToLeftHand(EquippedWeapon); // 将武器附加到左手
		ShowAttachedGrenade(true); // 显示附加的手榴弹
	}
	if (Character && !Character->HasAuthority()) // 非服务器端
	{
		ServerThrowGrenade(); // 向服务器发送投掷手榴弹请求
	}
	if (Character && Character->HasAuthority()) // 服务器端
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades); // 减少手榴弹数量
		UpdateHUDGrenades(); // 更新HUD上的手榴弹显示
	}
}

/**
 * 服务器端投掷手榴弹实现函数
 * 处理服务器端的手榴弹投掷逻辑
 */
void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades == 0) return; // 无手榴弹可投掷
	CombatState = ECombatState::ECS_ThrowingGrenade; // 设置战斗状态为投掷手榴弹中
	if (Character)
	{
		Character->PlayThrowGrenadeMontage(); // 播放投掷手榴弹动画
		AttachActorToLeftHand(EquippedWeapon); // 将武器附加到左手
		ShowAttachedGrenade(true); // 显示附加的手榴弹
	}
	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades); // 减少手榴弹数量
	UpdateHUDGrenades(); // 更新HUD上的手榴弹显示
}

/**
 * 更新HUD上手榴弹显示函数
 * 更新控制器HUD上的手榴弹数量显示
 */
void UCombatComponent::UpdateHUDGrenades()
{
	// 确保控制器有效
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDGrenades(Grenades); // 更新HUD上的手榴弹数量显示
	}
}

/**
 * 判断是否应该交换武器函数
 * @return 是否同时装备了主武器和副武器，可以进行交换
 */
bool UCombatComponent::ShouldSwapWeapons()
{
	// 如果同时装备了主武器和副武器，则可以交换
	return (EquippedWeapon != nullptr && SecondaryWeapon != nullptr);
}

/**
 * 显示/隐藏附加的手榴弹函数
 * @param bShowGrenade 是否显示手榴弹
 */
void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade); // 设置手榴弹的可见性
	}
}

/**
 * 已装备武器复制回调函数
 * 当服务器复制已装备武器到客户端时调用
 */
void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped); // 设置武器状态为已装备
		AttachActorToRightHand(EquippedWeapon); // 将武器附加到右手
		Character->GetCharacterMovement()->bOrientRotationToMovement = false; // 禁用移动旋转
		Character->bUseControllerRotationYaw = true; // 启用控制器旋转
		PlayEquipWeaponSound(EquippedWeapon); // 播放装备武器音效
		EquippedWeapon->EnableCustomDepth(false); // 禁用自定义深度
		EquippedWeapon->SetHUDAmmo(); // 更新HUD上的弹药显示
	}
}

/**
 * 副武器复制回调函数
 * 当服务器复制副武器到客户端时调用
 */
void UCombatComponent::OnRep_SecondaryWeapon()
{
	if (SecondaryWeapon && Character)
	{
		SecondaryWeapon->SetWeaponState(EWeaponState::EWS_EquippedSecondary); // 设置武器状态为已装备副武器
		AttachActorToBackpack(SecondaryWeapon); // 将武器附加到背包
		PlayEquipWeaponSound(EquippedWeapon); // 播放装备武器音效
	}
}

/**
 * 准星下射线追踪函数
 * 从屏幕准星位置发射射线，检测命中目标
 * @param TraceHitResult 输出参数，存储射线追踪的命中结果
 */
void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize); // 获取视口大小
	}

	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f); // 计算准星在屏幕上的位置（屏幕中心）
	FVector CrosshairWorldPosition; // 准星在世界空间中的位置
	FVector CrosshairWorldDirection; // 准星在世界空间中的方向
	// 将屏幕坐标转换为世界坐标和方向
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		if (Character)
		{
			// 调整起始点，考虑相机与武器之间的距离
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}

		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH; // 计算射线终点

		// 执行射线追踪
		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);
		// 根据命中的Actor类型设置准星颜色
		if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairsInterface>())
		{
			HUDPackage.CrosshairsColor = FLinearColor::Red; // 可交互对象显示红色准星
		}
		else
		{
			HUDPackage.CrosshairsColor = FLinearColor::White; // 普通对象显示白色准星
		}
	}
}

/**
 * 设置HUD准星函数
 * 根据武器类型、角色状态和移动速度更新HUD上的准星显示
 * @param DeltaTime 帧间隔时间，用于平滑插值
 */
void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	// 确保控制器有效
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		// 确保HUD有效
		HUD = HUD == nullptr ? Cast<ABlasterHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			// 根据当前装备的武器设置准星纹理
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairsCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairsLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairsRight;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairsBottom;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairsTop;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
			}
			// 计算准星扩散

			// 将速度范围映射到[0,1]区间
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f; // 忽略垂直速度

			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			// 根据角色状态计算各种扩散因子
			if (Character->GetCharacterMovement()->IsFalling()) // 下落状态
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else // 非下落状态
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			if (bAiming) // 瞄准状态
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			else // 非瞄准状态
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			// 重置射击因子
			CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 40.f);

			// 计算总的准星扩散值
			HUDPackage.CrosshairSpread = 
				0.5f + 
				CrosshairVelocityFactor + 
				CrosshairInAirFactor - 
				CrosshairAimFactor + 
				CrosshairShootingFactor;

			// 更新HUD显示
			HUD->SetHUDPackage(HUDPackage);
		}
	}
}

/**
 * 相机FOV插值函数
 * 根据瞄准状态平滑调整相机的视野角度
 * @param DeltaTime 帧间隔时间，用于平滑插值
 */
void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	if (bAiming) // 瞄准状态下的FOV
	{
		// 插值到武器的缩放FOV
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomInterpSpeed());
	}
	else // 默认FOV
	{
		// 插值回默认FOV
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}
	// 设置相机FOV
	if (Character && Character->GetFollowCamera())
	{
		Character->GetFollowCamera()->SetFieldOfView(CurrentFOV);
	}
}

/**
 * 设置瞄准状态函数
 * 根据按钮状态设置角色的瞄准状态和移动速度
 * @param bIsAiming 是否进入瞄准状态
 */
void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	bAiming = bIsAiming; // 更新瞄准状态
	ServerSetAiming(bIsAiming); // 向服务器发送瞄准请求
	if (Character)
	{
		// 根据瞄准状态调整移动速度
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
	// 狙击枪特殊处理：显示狙击镜
	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSniperScopeWidget(bIsAiming);
	}
	// 更新本地按钮状态
	if (Character->IsLocallyControlled()) bAimButtonPressed = bIsAiming;
}

/**
 * 服务器端设置瞄准状态实现函数
 * 处理服务器端的瞄准状态更新
 * @param bIsAiming 是否进入瞄准状态
 */
void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming; // 更新瞄准状态
	if (Character)
	{
		// 根据瞄准状态调整移动速度
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

/**
 * 判断是否可以开火函数
 * 检查是否满足开火条件
 * @return 是否可以开火
 */
bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false; // 无装备武器
	// 霰弹枪特殊情况：在装填状态也可以开火
	if (!EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Reloading && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun) return true;
	if (bLocallyReloading) return false; // 本地装填中
	// 常规开火条件：有弹药、可以开火、战斗状态为未占用
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

/**
 * 携带弹药数量复制回调函数
 * 当服务器复制携带弹药数量到客户端时调用
 */
void UCombatComponent::OnRep_CarriedAmmo()
{
	// 确保控制器有效
	Controller = Controller == nullptr ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDCarriedAmmo(CarriedAmmo); // 更新HUD上的携带弹药显示
	}
	// 霰弹枪特殊处理：如果在装填状态且弹药耗尽，跳转到装填动画结尾
	bool bJumpToShotgunEnd = 
		CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon != nullptr &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun &&
		CarriedAmmo == 0;
	if (bJumpToShotgunEnd)
	{
		JumpToShotgunEnd();
	}
}

/**
 * 初始化携带的弹药函数
 * 在游戏开始时初始化各种武器类型的弹药数量
 */
void UCombatComponent::InitializeCarriedAmmo()
{
	// 初始化各种武器类型的弹药映射
	CarriedAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, StartingARAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, StartingRocketAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Pistol, StartingPistolAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SubmachineGun, StartingSMGAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_Shotgun, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, StartingShotgunAmmo);
	CarriedAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, StartingGrenadeLauncherAmmo);
}

/**
 * 持有旗帜状态复制回调函数
 * 当服务器复制持有旗帜状态到客户端时调用
 */
void UCombatComponent::OnRep_HoldingTheFlag()
{
	if (bHoldingTheFlag && Character && Character->IsLocallyControlled()) // 如果持有旗帜
	{
		Character->Crouch(); // 角色下蹲
	}
}