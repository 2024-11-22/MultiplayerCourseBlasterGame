// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "AmmoPickup.generated.h"

/**
 * 弹药拾取物类：提供特定武器类型的弹药补给功能
 */
UCLASS()
class BLASTER_API AAmmoPickup : public APickup
{
	GENERATED_BODY()
protected:
	/**
	 * 当玩家接触到弹药拾取物时触发的重叠事件处理函数
	 * @param OverlappedComponent 重叠的组件（通常是碰撞球体）
	 * @param OtherActor 与拾取物重叠的其他Actor（通常是玩家角色）
	 * @param OtherComp 其他Actor上的重叠组件
	 * @param OtherBodyIndex 重叠组件的BodyIndex
	 * @param bFromSweep 是否通过扫描触发
	 * @param SweepResult 扫描结果信息
	 */
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);
private:
	/** 弹药数量：每次拾取可获得的弹药数目 */
	UPROPERTY(EditAnywhere)
	int32 AmmoAmount = 30;

	/** 武器类型：此弹药包对应的武器类型 */
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;
};
