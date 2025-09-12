// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Flag.generated.h"

/**
 * 旗帜类：代表夺旗模式中的旗帜道具，继承自武器类
 * 负责处理旗帜的装备、掉落和重置逻辑
 */
UCLASS()
class BLASTER_API AFlag : public AWeapon
{
	GENERATED_BODY()
public:
	/**
	 * 构造函数：初始化旗帜的核心组件和属性设置
	 */
	AFlag();
	
	/**
	 * 重写的掉落函数，处理旗帜被掉落的逻辑
	 */
	virtual void Dropped() override;
	
	/**
	 * 重置旗帜到初始位置
	 */
	void ResetFlag();
protected:
	/**
	 * 当旗帜被装备时的处理函数
	 */
	virtual void OnEquipped() override;
	
	/**
	 * 当旗帜被掉落时的处理函数
	 */
	virtual void OnDropped() override;
	
	/**
	 * 游戏开始时的初始化函数，记录旗帜的初始位置和旋转
	 */
	virtual void BeginPlay() override;
private:

	/**
	 * 旗帜的静态网格组件，用于显示旗帜的视觉效果
	 */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* FlagMesh;

	/**
	 * 旗帜的初始变换信息（位置和旋转），用于重置旗帜
	 */
	FTransform InitialTransform;
public:
	/**
	 * 获取旗帜的初始变换信息
	 * @return 旗帜的初始变换（位置和旋转）
	 */
	FORCEINLINE FTransform GetInitialTransform() const { return InitialTransform; }
};
