// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Net/UnrealNetwork.h"
#include "Animation/AnimationAsset.h"
#include "Components/SkeletalMeshComponent.h"
#include "Casing.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Kismet/KismetMathLibrary.h"

/**
 * 构造函数：初始化武器的基本属性和组件
 * 设置网络复制、武器网格、碰撞和交互组件
 */
AWeapon::AWeapon()
{
	// 禁用Actor的Tick更新，武器不需要每帧更新
	PrimaryActorTick.bCanEverTick = false;
	// 启用武器的网络复制
	bReplicates = true; 
	// 启用移动复制
	SetReplicateMovement(true); 

	// 创建武器网格组件
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	// 设置武器网格为根组件
	SetRootComponent(WeaponMesh);

	// 设置武器网格的碰撞响应
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	// 忽略与角色的碰撞
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	// 初始禁用碰撞
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 启用自定义深度渲染并设置为蓝色标记
	EnableCustomDepth(true);
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();

	// 创建区域球组件，用于检测可拾取范围
	AreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AreaSphere"));
	AreaSphere->SetupAttachment(RootComponent);
	// 初始设置区域球对所有通道的碰撞响应为忽略
	AreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 创建拾取提示界面组件
	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidget"));
	PickupWidget->SetupAttachment(RootComponent);
}

/**
 * 启用或禁用自定义深度渲染
 * @param bEnable - 是否启用自定义深度
 */
void AWeapon::EnableCustomDepth(bool bEnable)
{
	if (WeaponMesh)
	{
		// 根据参数设置是否启用自定义深度渲染
		WeaponMesh->SetRenderCustomDepth(bEnable);
	}
}

/**
 * 开始游戏时的初始化函数
 * 设置碰撞和UI组件的初始状态
 */
void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	// 启用区域球的碰撞检测，用于检测玩家接近武器
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// 设置区域球对角色通道的碰撞响应为重叠
	AreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	// 绑定区域球的重叠开始事件到OnSphereOverlap函数
	AreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnSphereOverlap);
	// 绑定区域球的重叠结束事件到OnSphereEndOverlap函数
	AreaSphere->OnComponentEndOverlap.AddDynamic(this, &AWeapon::OnSphereEndOverlap);

	// 初始隐藏拾取提示界面
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}
}

/**
 * 每帧更新函数
 * @param DeltaTime - 帧间隔时间
 */
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

/**
 * 设置需要网络复制的属性
 * @param OutLifetimeProps - 输出参数，用于存储需要复制的属性
 */
void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 复制武器状态属性到所有客户端
	DOREPLIFETIME(AWeapon, WeaponState);
	// 仅向武器所有者复制服务器端倒带设置
	DOREPLIFETIME_CONDITION(AWeapon, bUseServerSideRewind, COND_OwnerOnly);
}

/**
 * 区域球重叠开始时的回调
 * 当有Actor进入武器的可拾取范围时触发
 * @param OverlappedComponent - 重叠的组件
 * @param OtherActor - 重叠的其他Actor
 * @param OtherComp - 重叠的其他组件
 * @param OtherBodyIndex - 重叠的身体索引
 * @param bFromSweep - 是否来自扫描
 * @param SweepResult - 扫描结果
 */
void AWeapon::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 尝试将重叠的Actor转换为游戏角色
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);

	if (BlasterCharacter)
	{
		// 旗帜武器特殊处理：不允许同一队伍的玩家拾取
		if (WeaponType == EWeaponType::EWT_Flag && BlasterCharacter->GetTeam() == Team) return;
		// 不允许已经持有旗帜的玩家拾取武器
		if (BlasterCharacter->IsHoldingTheFlag()) return;
		// 设置角色的重叠武器引用，使玩家可以看到拾取提示
		BlasterCharacter->SetOverlappingWeapon(this);
	}
}

/**
 * 区域球重叠结束时的回调
 * 当Actor离开武器的可拾取范围时触发
 * @param OverlappedComponent - 重叠的组件
 * @param OtherActor - 重叠的其他Actor
 * @param OtherComp - 重叠的其他组件
 * @param OtherBodyIndex - 重叠的身体索引
 */
void AWeapon::OnSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 尝试将离开的Actor转换为游戏角色
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		// 旗帜武器特殊处理：不允许同一队伍的玩家拾取
		if (WeaponType == EWeaponType::EWT_Flag && BlasterCharacter->GetTeam() == Team) return;
		// 不允许已经持有旗帜的玩家拾取武器
		if (BlasterCharacter->IsHoldingTheFlag()) return;
		// 清除角色的重叠武器引用
		BlasterCharacter->SetOverlappingWeapon(nullptr);
	}
}

/**
 * 设置HUD上的弹药显示
 * 更新玩家HUD上的当前弹药数量
 */
void AWeapon::SetHUDAmmo()
{
	// 获取或更新武器所有者角色引用
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter)
	{
		// 获取或更新玩家控制器引用
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController)
		{
			// 通过控制器更新HUD上的弹药显示
			BlasterOwnerController->SetHUDWeaponAmmo(Ammo);
		}
	}
}

/**
 * 消耗一发弹药
 * 减少弹药数量并更新UI和客户端
 */
void AWeapon::SpendRound()
{
	// 减少弹药数量并确保不小于0且不超过弹夹容量
	Ammo = FMath::Clamp(Ammo - 1, 0, MagCapacity);
	// 更新HUD弹药显示
	SetHUDAmmo();
	
	// 如果是服务器，同步弹药数量到客户端
	if (HasAuthority())
	{
		ClientUpdateAmmo(Ammo);
	}
	// 如果是客户端，增加序列号（用于处理网络延迟和同步问题）
	else
	{
		++Sequence;
	}
}

/**
 * 客户端更新弹药数量
 * 从服务器接收弹药数量更新
 * @param ServerAmmo - 服务器上的弹药数量
 */
void AWeapon::ClientUpdateAmmo_Implementation(int32 ServerAmmo)
{
	// 服务器不需要执行此函数
	if (HasAuthority()) return;
	// 更新本地弹药数量为服务器发送的值
	Ammo = ServerAmmo;
	// 减少序列号，处理可能的网络延迟导致的未确认请求
	--Sequence;
	// 调整弹药数量以考虑未处理的请求
	Ammo -= Sequence;
	// 更新HUD弹药显示
	SetHUDAmmo();
}

/**
 * 添加弹药
 * 增加弹药数量并同步到客户端
 * @param AmmoToAdd - 要添加的弹药数量
 */
void AWeapon::AddAmmo(int32 AmmoToAdd)
{
	// 增加弹药数量并确保不超过弹夹容量
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	// 更新HUD弹药显示
	SetHUDAmmo();
	// 通知客户端添加弹药
	ClientAddAmmo(AmmoToAdd);
}

/**
 * 客户端添加弹药
 * 从服务器接收添加弹药的通知
 * @param AmmoToAdd - 要添加的弹药数量
 */
void AWeapon::ClientAddAmmo_Implementation(int32 AmmoToAdd)
{
	// 服务器不需要执行此函数
	if (HasAuthority()) return;
	// 增加本地弹药数量
	Ammo = FMath::Clamp(Ammo + AmmoToAdd, 0, MagCapacity);
	// 获取或更新武器所有者角色引用
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	// 如果是霰弹枪且弹药已满，跳到上膛动画结束
	if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetCombat() && IsFull())
	{
		BlasterOwnerCharacter->GetCombat()->JumpToShotgunEnd();
	}
	// 更新HUD弹药显示
	SetHUDAmmo();
}

/**
 * 所有者变更时的网络回调
 * 当武器的所有者通过网络变更时调用
 */
void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();
	if (Owner == nullptr)
	{
		// 清除所有者引用
		BlasterOwnerCharacter = nullptr;
		BlasterOwnerController = nullptr;
	}
	else
	{
		// 更新所有者引用
		BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(Owner) : BlasterOwnerCharacter;
		// 如果当前武器是角色装备的武器，更新HUD弹药显示
		if (BlasterOwnerCharacter && BlasterOwnerCharacter->GetEquippedWeapon() && BlasterOwnerCharacter->GetEquippedWeapon() == this)
		{
			SetHUDAmmo();
		}
	}
}

/**
 * 设置武器状态
 * @param State - 新的武器状态
 */
void AWeapon::SetWeaponState(EWeaponState State)
{
	// 更新武器状态
	WeaponState = State;
	// 调用状态变更处理函数
	OnWeaponStateSet();
}

/**
 * 武器状态变更时的处理函数
 * 根据新的武器状态执行相应的逻辑
 */
void AWeapon::OnWeaponStateSet()
{
	// 根据武器状态执行不同的处理逻辑
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		OnEquipped(); // 武器被装备
		break;
	case EWeaponState::EWS_EquippedSecondary:
		OnEquippedSecondary(); // 武器被装备为次要武器
		break;
	case EWeaponState::EWS_Dropped:
		OnDropped(); // 武器被丢弃
		break;
	default:
		break;
	}
}

/**
 * 当网络延迟过高时的回调
 * @param bPingTooHigh - 网络延迟是否过高
 */
void AWeapon::OnPingTooHigh(bool bPingTooHigh)
{
	// 网络延迟过高时禁用服务器端倒带功能，以避免网络同步问题
	bUseServerSideRewind = !bPingTooHigh;
}

/**
 * 武器状态通过网络复制后的回调
 * 当客户端收到武器状态变更时触发
 */
void AWeapon::OnRep_WeaponState()
{
	// 调用状态变更处理函数，确保客户端与服务器状态一致
	OnWeaponStateSet();
}

/**
 * 武器被装备时的处理函数
 * 设置武器为装备状态的各种属性
 */
void AWeapon::OnEquipped()
{
	// 隐藏拾取提示界面
	ShowPickupWidget(false);
	// 禁用区域球碰撞，装备中的武器不需要被拾取
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 禁用物理模拟和重力，武器由角色控制
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 冲锋枪特殊处理：启用碰撞但忽略所有通道
	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	// 禁用自定义深度渲染
	EnableCustomDepth(false);

	// 获取或更新武器所有者角色引用
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	// 如果启用了服务器端倒带，绑定网络延迟回调
	if (BlasterOwnerCharacter && bUseServerSideRewind)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		if (BlasterOwnerController && HasAuthority() && !BlasterOwnerController->HighPingDelegate.IsBound())
		{
			// 绑定网络延迟回调函数，用于处理高延迟情况
			BlasterOwnerController->HighPingDelegate.AddDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

/**
 * 武器被丢弃时的处理函数
 * 设置武器为丢弃状态的各种属性
 */
void AWeapon::OnDropped()
{
	// 服务器端启用区域球碰撞，使武器可被再次拾取
	if (HasAuthority())
	{
		AreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	// 启用物理模拟和重力，使武器受物理影响
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetEnableGravity(true);
	// 启用碰撞检测
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	// 忽略与角色和相机的碰撞
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// 设置蓝色自定义深度标记，使丢弃的武器在UI上有特殊显示
	WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	WeaponMesh->MarkRenderStateDirty();
	EnableCustomDepth(true);

	// 获取或更新武器所有者角色引用
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		// 移除网络延迟回调
		if (BlasterOwnerController && HasAuthority() && BlasterOwnerController->HighPingDelegate.IsBound())
		{
			BlasterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

/**
 * 武器被装备为次要武器时的处理函数
 * 设置武器为次要武器状态的各种属性
 */
void AWeapon::OnEquippedSecondary()
{
	// 隐藏拾取提示界面
	ShowPickupWidget(false);
	// 禁用区域球碰撞
	AreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 禁用物理模拟和重力
	WeaponMesh->SetSimulatePhysics(false);
	WeaponMesh->SetEnableGravity(false);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	// 冲锋枪特殊处理：启用碰撞但忽略所有通道
	if (WeaponType == EWeaponType::EWT_SubmachineGun)
	{
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		WeaponMesh->SetEnableGravity(true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	}
	// 设置棕色自定义深度标记（次要武器的特殊标记）
	if (WeaponMesh)
	{
		WeaponMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_TAN);
		WeaponMesh->MarkRenderStateDirty();
	}
	// 获取或更新武器所有者角色引用
	BlasterOwnerCharacter = BlasterOwnerCharacter == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : BlasterOwnerCharacter;
	if (BlasterOwnerCharacter)
	{
		BlasterOwnerController = BlasterOwnerController == nullptr ? Cast<ABlasterPlayerController>(BlasterOwnerCharacter->Controller) : BlasterOwnerController;
		// 移除网络延迟回调
		if (BlasterOwnerController && HasAuthority() && BlasterOwnerController->HighPingDelegate.IsBound())
		{
			BlasterOwnerController->HighPingDelegate.RemoveDynamic(this, &AWeapon::OnPingTooHigh);
		}
	}
}

/**
 * 显示或隐藏拾取提示界面
 * @param bShowWidget - 是否显示拾取界面
 */
void AWeapon::ShowPickupWidget(bool bShowWidget)
{
	if (PickupWidget)
	{
		// 根据参数设置拾取界面的可见性
		PickupWidget->SetVisibility(bShowWidget);
	}
}

/**
 * 武器发射函数
 * 基类实现，播放射击动画、生成弹壳并消耗弹药
 * 派生类会重写此函数以实现不同类型武器的发射逻辑
 * @param HitTarget - 命中目标的位置
 */
void AWeapon::Fire(const FVector& HitTarget)
{
	// 播放射击动画
	if (FireAnimation)
	{
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
	// 生成弹壳
	if (CasingClass)
	{
		// 获取弹药弹出插槽
		const USkeletalMeshSocket* AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));
		if (AmmoEjectSocket)
		{
			// 获取插槽的变换（位置和旋转）
			FTransform SocketTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);

			UWorld* World = GetWorld();
			if (World)
			{
				// 在插槽位置生成弹壳Actor
				World->SpawnActor<ACasing>(
					CasingClass,
					SocketTransform.GetLocation(),
					SocketTransform.GetRotation().Rotator()
				);
			}
		}
	}
	// 消耗一发弹药
	SpendRound();
}

/**
 * 丢弃武器函数
 * 将武器从角色手中丢弃到地面
 */
void AWeapon::Dropped()
{
	// 设置武器状态为已丢弃
	SetWeaponState(EWeaponState::EWS_Dropped);
	// 从角色身上分离武器，保持世界坐标系
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	// 清除所有者引用
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}

/**
 * 检查武器是否为空（无弹药）
 * @return 武器是否为空
 */
bool AWeapon::IsEmpty()
{
	return Ammo <= 0;
}

/**
 * 检查武器弹药是否已满
 * @return 弹药是否已满
 */
bool AWeapon::IsFull()
{
	return Ammo == MagCapacity;
}

/**
 * 计算带散射的射线终点
 * 用于模拟武器子弹的散布效果
 * @param HitTarget - 原始命中目标位置
 * @return 散射后的射线终点位置
 */
FVector AWeapon::TraceEndWithScatter(const FVector& HitTarget)
{
	// 获取枪口闪光插槽，用于确定射线起点
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName("MuzzleFlash");
	if (MuzzleFlashSocket == nullptr) return FVector();

	// 获取插槽的变换和射线起点
	const FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
	const FVector TraceStart = SocketTransform.GetLocation();

	// 计算到目标的归一化向量
	const FVector ToTargetNormalized = (HitTarget - TraceStart).GetSafeNormal();
	// 计算球心位置（射线方向上的指定距离）
	const FVector SphereCenter = TraceStart + ToTargetNormalized * DistanceToSphere;
	// 生成随机向量用于散射
	const FVector RandVec = UKismetMathLibrary::RandomUnitVector() * FMath::FRandRange(0.f, SphereRadius);
	// 计算带有散射的终点位置
	const FVector EndLoc = SphereCenter + RandVec;
	const FVector ToEndLoc = EndLoc - TraceStart;

	/* 调试代码（已注释）
	DrawDebugSphere(GetWorld(), SphereCenter, SphereRadius, 12, FColor::Red, true);
	DrawDebugSphere(GetWorld(), EndLoc, 4.f, 12, FColor::Orange, true);
	DrawDebugLine(
		GetWorld(),
		TraceStart,
		FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size()),
		FColor::Cyan,
		true);*/

	// 返回指定长度的射线终点
	return FVector(TraceStart + ToEndLoc * TRACE_LENGTH / ToEndLoc.Size());
}