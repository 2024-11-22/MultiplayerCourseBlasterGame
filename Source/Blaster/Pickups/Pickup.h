// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Pickup.generated.h"

/**
 * 拾取物基类：所有可拾取物品的基础类，提供碰撞检测、旋转效果和拾取逻辑的框架
 */
UCLASS()
class BLASTER_API APickup : public AActor
{
	GENERATED_BODY()
	
public: 	
	/** 构造函数：初始化拾取物的组件和属性 */
	APickup();
	/** 每帧更新函数：处理拾取物的旋转等动态效果 */
	virtual void Tick(float DeltaTime) override;
	/** 销毁时调用的函数：播放拾取音效和特效 */
	virtual void Destroyed() override;
protected:
	/** 开始播放时调用的函数：设置重叠检测的延迟绑定 */
	virtual void BeginPlay() override;

	/**
	 * 当球体组件与其他Actor重叠时调用的函数
	 * @param OverlappedComponent - 发生重叠的组件
	 * @param OtherActor - 重叠的其他Actor
	 * @param OtherComp - 重叠的其他组件
	 * @param OtherBodyIndex - 重叠的身体索引
	 * @param bFromSweep - 是否来自扫描
	 * @param SweepResult - 扫描结果
	 */
	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

	/** 基础旋转速度，控制拾取物旋转的快慢 */
	UPROPERTY(EditAnywhere)
	float BaseTurnRate = 45.f;

private:

	/** 重叠检测球体组件，用于检测玩家接近 */
	UPROPERTY(EditAnywhere)
	class USphereComponent* OverlapSphere;

	/** 拾取音效，当拾取物品时播放 */
	UPROPERTY(EditAnywhere)
	class USoundCue* PickupSound;

	/** 拾取物的静态网格组件，用于显示物品模型 */
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* PickupMesh;

	/** 粒子效果组件，用于显示拾取物的特效 */
	UPROPERTY(VisibleAnywhere)
	class UNiagaraComponent* PickupEffectComponent;

	/** 粒子效果系统，用于在销毁时播放 */
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* PickupEffect;

	/** 延迟绑定重叠事件的定时器句柄 */
	FTimerHandle BindOverlapTimer;
	/** 延迟绑定重叠事件的时间（秒） */
	float BindOverlapTime = 0.25f;
	/** 延迟绑定重叠事件的完成函数 */
	void BindOverlapTimerFinished();

public: 	

};
