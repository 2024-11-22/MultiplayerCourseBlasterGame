// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


/**
 * UBuffComponent类：管理角色的各种增益效果，包括生命值回复、护盾回复、移动速度提升和跳跃能力增强
 * 该组件可附加到角色身上，处理各种临时和持续性的属性增益
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public: 
	/**
	 * 构造函数：初始化UBuffComponent的默认值
	 */
	UBuffComponent();
	
	/**
	 * 友元类：允许ABlasterCharacter直接访问UBuffComponent的私有成员
	 */
	friend class ABlasterCharacter;
	
	/**
	 * 治疗函数：在指定时间内逐渐回复角色的生命值
	 * @param HealAmount 需要回复的总生命值
	 * @param HealingTime 完成治疗所需的时间（秒）
	 */
	void Heal(float HealAmount, float HealingTime);
	
	/**
	 * 护盾回复函数：在指定时间内逐渐回复角色的护盾值
	 * @param ShieldAmount 需要回复的总护盾值
	 * @param ReplenishTime 完成护盾回复所需的时间（秒）
	 */
	void ReplenishShield(float ShieldAmount, float ReplenishTime);
	
	/**
	 * 速度增益函数：临时提升角色的移动速度
	 * @param BuffBaseSpeed 增益后的基础移动速度
	 * @param BuffCrouchSpeed 增益后的蹲伏移动速度
	 * @param BuffTime 增益效果持续时间（秒）
	 */
	void BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime);
	
	/**
	 * 跳跃增益函数：临时提升角色的跳跃高度
	 * @param BuffJumpVelocity 增益后的跳跃速度
	 * @param BuffTime 增益效果持续时间（秒）
	 */
	void BuffJump(float BuffJumpVelocity, float BuffTime);
	
	/**
	 * 设置初始速度：保存角色的原始移动速度用于增益效果结束后恢复
	 * @param BaseSpeed 原始基础移动速度
	 * @param CrouchSpeed 原始蹲伏移动速度
	 */
	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);
	
	/**
	 * 设置初始跳跃速度：保存角色的原始跳跃速度用于增益效果结束后恢复
	 * @param Velocity 原始跳跃速度
	 */
	void SetInitialJumpVelocity(float Velocity);
protected:
	/**
	 * 开始游戏：在组件被创建时调用，初始化组件状态
	 */
	virtual void BeginPlay() override;
	
	/**
	 * 治疗渐进：每帧更新治疗进度
	 * @param DeltaTime 帧时间（秒）
	 */
	void HealRampUp(float DeltaTime);
	
	/**
	 * 护盾渐进：每帧更新护盾回复进度
	 * @param DeltaTime 帧时间（秒）
	 */
	void ShieldRampUp(float DeltaTime);
private:
	/**
	 * 角色引用：指向拥有此组件的角色
	 */
	UPROPERTY()
	class ABlasterCharacter* Character;

	/** 
	* 治疗增益相关变量
	*/
	
	/** 是否正在进行治疗 */
	bool bHealing = false;
	/** 每秒治疗量 */
	float HealingRate = 0.f;
	/** 还需要治疗的总量 */
	float AmountToHeal = 0.f;

	/** 
	* 护盾增益相关变量
	*/
	
	/** 是否正在回复护盾 */
	bool bReplenishingShield = false;
	/** 每秒护盾回复量 */
	float ShieldReplenishRate = 0.f;
	/** 还需要回复的护盾总量 */
	float ShieldReplenishAmount = 0.f;

	/** 
	* 速度增益相关变量和函数
	*/
	
	/** 速度增益定时器句柄 */
	FTimerHandle SpeedBuffTimer;
	/** 重置速度：将角色速度恢复到原始值 */
	void ResetSpeeds();
	/** 保存的原始基础速度 */
	float InitialBaseSpeed;
	/** 保存的原始蹲伏速度 */
	float InitialCrouchSpeed;

	/**
	 * 多播速度增益：在所有客户端上应用速度增益效果
	 * @param BaseSpeed 增益后的基础移动速度
	 * @param CrouchSpeed 增益后的蹲伏移动速度
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastSpeedBuff(float BaseSpeed, float CrouchSpeed);

	/** 
	* 跳跃增益相关变量和函数
	*/
	/** 跳跃增益定时器句柄 */
	FTimerHandle JumpBuffTimer;
	/** 重置跳跃：将角色跳跃能力恢复到原始值 */
	void ResetJump();
	/** 保存的原始跳跃速度 */
	float InitialJumpVelocity;

	/**
	 * 多播跳跃增益：在所有客户端上应用跳跃增益效果
	 * @param JumpVelocity 增益后的跳跃速度
	 */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastJumpBuff(float JumpVelocity);

public: 
	/**
	 * 每帧更新：处理正在进行的增益效果
	 * @param DeltaTime 帧时间（秒）
	 * @param TickType 帧类型
	 * @param ThisTickFunction 该组件的tick函数
	 */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
