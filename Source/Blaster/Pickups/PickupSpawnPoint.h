// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PickupSpawnPoint.generated.h"

/**
 * 拾取物生成点类：管理游戏中拾取物的生成、计时和重生逻辑
 */
UCLASS()
class BLASTER_API APickupSpawnPoint : public AActor
{
	GENERATED_BODY()
	
public: 	
	/** 构造函数：初始化拾取物生成点的基本属性 */
	APickupSpawnPoint();
	/** 每帧更新函数：处理生成点的动态逻辑 */
	virtual void Tick(float DeltaTime) override;

protected:
	/** 游戏开始时调用：初始化生成点状态 */
	virtual void BeginPlay() override;

	/** 可生成的拾取物类型列表：在编辑器中配置可在此处生成的拾取物类型 */
	UPROPERTY(EditAnywhere)
	TArray<TSubclassOf<class APickup>> PickupClasses;

	/** 当前生成的拾取物引用：指向当前在此生成点生成的拾取物 */
	UPROPERTY()
	APickup* SpawnedPickup;

	/** 生成拾取物：从配置的类型列表中随机选择并生成一个拾取物 */
	void SpawnPickup();
	/** 生成计时器完成回调：当生成计时器结束时调用，触发拾取物生成 */
	void SpawnPickupTimerFinished();

	/**
	 * 开始生成拾取物计时器
	 * @param DestroyedActor 被销毁的Actor（通常是之前生成的拾取物）
	 */
	UFUNCTION()
	void StartSpawnPickupTimer(AActor* DestroyedActor);
private:
	/** 生成拾取物计时器句柄：用于管理拾取物生成的延迟时间 */
	FTimerHandle SpawnPickupTimer;

	/** 生成拾取物最小时间：两次生成之间的最小间隔（秒） */
	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMin;

	/** 生成拾取物最大时间：两次生成之间的最大间隔（秒） */
	UPROPERTY(EditAnywhere)
	float SpawnPickupTimeMax;
public: 	


};
