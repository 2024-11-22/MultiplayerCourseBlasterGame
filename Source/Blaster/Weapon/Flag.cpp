// Fill out your copyright notice in the Description page of Project Settings.


#include "Flag.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Blaster/Character/BlasterCharacter.h"

/**
 * 构造函数：初始化旗帜的核心组件和碰撞设置
 */
AFlag::AFlag()
{
	// 创建并初始化旗帜的静态网格组件
	FlagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FlagMesh"));
	// 将旗帜网格组件设置为根组件
	SetRootComponent(FlagMesh);
	// 设置区域球体（用于拾取检测）附加到旗帜网格
	GetAreaSphere()->SetupAttachment(FlagMesh);
	// 设置拾取UI组件附加到旗帜网格
	GetPickupWidget()->SetupAttachment(FlagMesh);
	// 设置旗帜网格对所有通道的碰撞响应为忽略
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	// 禁用旗帜网格的碰撞检测
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

/**
 * 处理旗帜被掉落的逻辑
 */
void AFlag::Dropped()
{
	// 设置武器状态为掉落状态
	SetWeaponState(EWeaponState::EWS_Dropped);
	// 设置分离变换规则，保持世界空间位置
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	// 从之前的附加组件分离旗帜网格
	FlagMesh->DetachFromComponent(DetachRules);
	// 清除所有者信息
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;
}

/**
 * 重置旗帜到初始位置和状态
 */
void AFlag::ResetFlag()
{
	// 获取当前持有旗帜的角色
	ABlasterCharacter* FlagBearer = Cast<ABlasterCharacter>(GetOwner());
	if (FlagBearer)
	{
		// 更新持有旗帜的角色状态
		FlagBearer->SetHoldingTheFlag(false);
		FlagBearer->SetOverlappingWeapon(nullptr);
		FlagBearer->UnCrouch();
	}

	// 只有服务器有权限重置旗帜
	if (!HasAuthority()) return;

	// 设置分离变换规则，保持世界空间位置
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	// 从附加的组件分离旗帜网格
	FlagMesh->DetachFromComponent(DetachRules);
	// 设置武器状态为初始状态
	SetWeaponState(EWeaponState::EWS_Initial);
	// 启用区域球体的碰撞检测
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// 设置对Pawn通道的碰撞响应为重叠
	GetAreaSphere()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	// 清除所有者信息
	SetOwner(nullptr);
	BlasterOwnerCharacter = nullptr;
	BlasterOwnerController = nullptr;

	// 将旗帜重置到初始位置和旋转
	SetActorTransform(InitialTransform);
}

/**
 * 当旗帜被装备时的处理函数
 */
void AFlag::OnEquipped()
{
	// 隐藏拾取UI
	ShowPickupWidget(false);
	// 禁用区域球体的碰撞检测
	GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 禁用物理模拟和重力
	FlagMesh->SetSimulatePhysics(false);
	FlagMesh->SetEnableGravity(false);
	// 设置碰撞检测为仅查询模式
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	// 设置对动态世界物体的碰撞响应为重叠
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	// 禁用自定义深度渲染
	EnableCustomDepth(false);
}

/**
 * 当旗帜被掉落时的处理函数
 */
void AFlag::OnDropped()
{
	// 在服务器端启用区域球体的碰撞检测
	if (HasAuthority())
	{
		GetAreaSphere()->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
	// 启用物理模拟和重力
	FlagMesh->SetSimulatePhysics(true);
	FlagMesh->SetEnableGravity(true);
	// 设置碰撞检测为查询和物理模式
	FlagMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	// 设置对所有通道的碰撞响应为阻挡
	FlagMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	// 除外：对Pawn和Camera通道的碰撞响应为忽略
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	FlagMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	// 设置自定义深度模板值为蓝色（用于标识掉落状态）
	FlagMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_BLUE);
	FlagMesh->MarkRenderStateDirty();
	// 启用自定义深度渲染
	EnableCustomDepth(true);
}

/**
 * 游戏开始时的初始化函数
 */
void AFlag::BeginPlay()
{
	Super::BeginPlay();
	// 记录旗帜的初始位置和旋转信息，用于后续重置
	InitialTransform = GetActorTransform();
}
