// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/SphereComponent.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"

/**
 * 构造函数：初始化拾取物的组件和属性
 * 设置根组件、重叠检测球体、网格组件和粒子效果组件
 */
APickup::APickup()
{
	// 允许组件每帧更新
	PrimaryActorTick.bCanEverTick = true;
	// 启用网络复制
	bReplicates = true;

	// 创建根场景组件
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	// 创建重叠检测球体组件
	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapSphere"));
	OverlapSphere->SetupAttachment(RootComponent);
	OverlapSphere->SetSphereRadius(150.f); // 设置球体半径
	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly); // 设置碰撞为查询模式
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); // 默认忽略所有碰撞
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap); // 只对Pawn类型启用重叠检测
	OverlapSphere->AddLocalOffset(FVector(0.f, 0.f, 85.f)); // 调整球体位置

	// 创建拾取物网格组件
	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PickupMesh"));
	PickupMesh->SetupAttachment(OverlapSphere);
	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 禁用网格的碰撞
	PickupMesh->SetRelativeScale3D(FVector(5.f, 5.f, 5.f)); // 设置网格缩放
	PickupMesh->SetRenderCustomDepth(true); // 启用自定义深度渲染
	PickupMesh->SetCustomDepthStencilValue(CUSTOM_DEPTH_PURPLE); // 设置自定义深度为紫色

	// 创建粒子效果组件
	PickupEffectComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("PickupEffectComponent"));
	PickupEffectComponent->SetupAttachment(RootComponent);
}

/**
 * 开始播放时调用的函数
 * 服务器端设置延迟绑定重叠事件，防止客户端过早触发
 */
void APickup::BeginPlay()
{
	Super::BeginPlay();

	// 只有在服务器端执行
	if (HasAuthority())
	{
		// 设置延迟绑定重叠事件的定时器
		GetWorldTimerManager().SetTimer(
			BindOverlapTimer,
			this,
			&APickup::BindOverlapTimerFinished,
			BindOverlapTime
		);
	}
}

/**
 * 当球体组件与其他Actor重叠时调用的函数
 * 基类中为空实现，由子类重写来实现具体的拾取逻辑
 * @param OverlappedComponent - 发生重叠的组件
 * @param OtherActor - 重叠的其他Actor
 * @param OtherComp - 重叠的其他组件
 * @param OtherBodyIndex - 重叠的身体索引
 * @param bFromSweep - 是否来自扫描
 * @param SweepResult - 扫描结果
 */
void APickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

}

/**
 * 延迟绑定重叠事件的完成函数
 * 绑定重叠检测球体的重叠开始事件到OnSphereOverlap函数
 */
void APickup::BindOverlapTimerFinished()
{
	// 绑定重叠开始事件
	OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &APickup::OnSphereOverlap);
}

/**
 * 每帧更新函数
 * 处理拾取物的旋转效果
 * @param DeltaTime - 帧时间
 */
void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 检查网格组件是否有效
	if (PickupMesh)
	{
		// 按基础旋转速度绕Y轴旋转网格
		PickupMesh->AddWorldRotation(FRotator(0.f, BaseTurnRate * DeltaTime, 0.f));
	}
}

/**
 * 销毁时调用的函数
 * 播放拾取音效和特效
 */
void APickup::Destroyed()
{
	Super::Destroyed();

	// 如果有拾取音效，播放音效
	if (PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			PickupSound,
			GetActorLocation()
		);
	}
	// 如果有拾取特效，播放特效
	if (PickupEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			PickupEffect,
			GetActorLocation(),
			GetActorRotation()
		);
	}
}

