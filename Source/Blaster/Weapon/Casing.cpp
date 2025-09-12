// Fill out your copyright notice in the Description page of Project Settings.


#include "Casing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

/**
 * 构造函数：初始化弹壳的核心组件和物理属性
 */
ACasing::ACasing()
{
	// 禁用Actor的Tick更新，弹壳不需要每帧更新
	PrimaryActorTick.bCanEverTick = false;

	// 创建并初始化弹壳的静态网格组件
	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	// 将网格组件设置为根组件
	SetRootComponent(CasingMesh);
	// 设置对相机通道的碰撞响应为忽略，避免弹壳遮挡玩家视角
	CasingMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	// 启用物理模拟，使弹壳受物理影响
	CasingMesh->SetSimulatePhysics(true);
	// 启用重力效果
	CasingMesh->SetEnableGravity(true);
	// 启用刚体碰撞通知，以便触发OnHit事件
	CasingMesh->SetNotifyRigidBodyCollision(true);
	// 设置弹壳抛出的初始冲量值
	ShellEjectionImpulse = 10.f;
}

/**
 * 游戏开始时的初始化函数，设置弹壳的碰撞事件和抛出冲量
 */
void ACasing::BeginPlay()
{
	Super::BeginPlay();
	
	// 绑定碰撞事件到OnHit函数
	CasingMesh->OnComponentHit.AddDynamic(this, &ACasing::OnHit);
	// 应用抛出冲量，使弹壳从武器中弹出
	CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
}

/**
 * 处理弹壳碰撞事件的回调函数
 * @param HitComp 碰撞的组件（通常是弹壳自身）
 * @param OtherActor 被碰撞的Actor
 * @param OtherComp 被碰撞的组件
 * @param NormalImpulse 碰撞的法向冲量
 * @param Hit 碰撞结果信息
 */
void ACasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 检查是否有碰撞音效，如果有则播放
	if (ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}
	// 播放音效后销毁弹壳Actor
	Destroy();
}

