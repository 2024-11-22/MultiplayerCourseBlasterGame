// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupSpawnPoint.h"
#include "Pickup.h"

/**
 * 构造函数：初始化拾取物生成点的基本属性
 * 禁用Tick以提高性能，并启用网络复制
 */
APickupSpawnPoint::APickupSpawnPoint()
{
	// 禁用Actor的Tick更新，因为此生成点不需要每帧更新
	PrimaryActorTick.bCanEverTick = false;
	// 启用网络复制，确保生成点在多人游戏中正确同步
	bReplicates = true;
}

/**
 * 游戏开始时调用：初始化生成点状态
 * 开始第一个拾取物的生成计时器
 */
void APickupSpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	// 开始生成拾取物的计时器（传入nullptr表示这是初始生成）
	StartSpawnPickupTimer((AActor*)nullptr);
}

/**
 * 生成拾取物：从配置的类型列表中随机选择并生成一个拾取物
 * 只有服务器端有生成权限
 */
void APickupSpawnPoint::SpawnPickup()
{
	// 获取可生成的拾取物类型数量
	int32 NumPickupClasses = PickupClasses.Num();
	if (NumPickupClasses > 0)
	{
		// 随机选择一个拾取物类型
		int32 Selection = FMath::RandRange(0, NumPickupClasses - 1);
		// 在生成点位置生成选中的拾取物类型
		SpawnedPickup = GetWorld()->SpawnActor<APickup>(PickupClasses[Selection], GetActorTransform());

		// 只在服务器端设置销毁事件回调
		if (HasAuthority() && SpawnedPickup)
		{
			// 绑定拾取物销毁事件到生成计时器启动函数
			SpawnedPickup->OnDestroyed.AddDynamic(this, &APickupSpawnPoint::StartSpawnPickupTimer);
		}
	}
}

/**
 * 生成计时器完成回调：当生成计时器结束时调用，触发拾取物生成
 * 只在服务器端执行生成逻辑
 */
void APickupSpawnPoint::SpawnPickupTimerFinished()
{
	// 只有服务器端有权限生成新的拾取物
	if (HasAuthority())
	{
		SpawnPickup();
	}
}

/**
 * 开始生成拾取物计时器
 * 设置一个随机延迟时间后再生成新的拾取物
 * @param DestroyedActor 被销毁的Actor（通常是之前生成的拾取物）
 */
void APickupSpawnPoint::StartSpawnPickupTimer(AActor* DestroyedActor)
{
	// 在最小和最大时间之间随机选择一个生成延迟时间
	const float SpawnTime = FMath::FRandRange(SpawnPickupTimeMin, SpawnPickupTimeMax);
	// 设置计时器，到时后调用SpawnPickupTimerFinished函数
	GetWorldTimerManager().SetTimer(
		SpawnPickupTimer,
		this,
		&APickupSpawnPoint::SpawnPickupTimerFinished,
		SpawnTime
	);
}

/**
 * 每帧更新函数：处理生成点的动态逻辑
 * 由于PrimaryActorTick.bCanEverTick设置为false，此函数实际上不会被调用
 */
void APickupSpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

