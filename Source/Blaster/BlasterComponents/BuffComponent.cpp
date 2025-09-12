// Fill out your copyright notice in the Description page of Project Settings.


#include "BuffComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

/**
 * UBuffComponent构造函数：初始化组件的默认状态
 */
UBuffComponent::UBuffComponent()
{
	// 允许组件每帧更新
	PrimaryComponentTick.bCanEverTick = true;

}


/**
 * 治疗函数：设置治疗状态和参数，实际的治疗效果会在TickComponent中通过HealRampUp函数逐渐应用
 * @param HealAmount 需要回复的总生命值
 * @param HealingTime 完成治疗所需的时间（秒）
 */
void UBuffComponent::Heal(float HealAmount, float HealingTime)
{
	// 标记为正在治疗状态
	bHealing = true;
	// 计算每秒治疗量
	HealingRate = HealAmount / HealingTime;
	// 累加需要治疗的总量
	AmountToHeal += HealAmount;
}

/**
 * 护盾回复函数：设置护盾回复状态和参数，实际的护盾回复会在TickComponent中通过ShieldRampUp函数逐渐应用
 * @param ShieldAmount 需要回复的总护盾值
 * @param ReplenishTime 完成护盾回复所需的时间（秒）
 */
void UBuffComponent::ReplenishShield(float ShieldAmount, float ReplenishTime)
{
	// 标记为正在回复护盾状态
	bReplenishingShield = true;
	// 计算每秒护盾回复量
	ShieldReplenishRate = ShieldAmount / ReplenishTime;
	// 累加需要回复的护盾总量
	ShieldReplenishAmount += ShieldAmount;
}

/**
 * 治疗渐进函数：每帧更新治疗进度，逐渐增加角色的生命值
 * @param DeltaTime 帧时间（秒）
 */
void UBuffComponent::HealRampUp(float DeltaTime)
{
	// 检查是否可以进行治疗（治疗状态、角色存在且未被淘汰）
	if (!bHealing || Character == nullptr || Character->IsElimmed()) return;

	// 计算本帧应该治疗的生命值
	const float HealThisFrame = HealingRate * DeltaTime;
	// 应用治疗并确保生命值不超过最大值
	Character->SetHealth(FMath::Clamp(Character->GetHealth() + HealThisFrame, 0.f, Character->GetMaxHealth()));
	// 更新UI显示的生命值
	Character->UpdateHUDHealth();
	// 减少还需要治疗的总量
	AmountToHeal -= HealThisFrame;

	// 检查治疗是否完成（已达到所需治疗量或生命值已满）
	if (AmountToHeal <= 0.f || Character->GetHealth() >= Character->GetMaxHealth())
	{
		// 重置治疗状态
		bHealing = false;
		AmountToHeal = 0.f;
	}
}

/**
 * 护盾渐进函数：每帧更新护盾回复进度，逐渐增加角色的护盾值
 * @param DeltaTime 帧时间（秒）
 */
void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	// 检查是否可以进行护盾回复（回复状态、角色存在且未被淘汰）
	if (!bReplenishingShield || Character == nullptr || Character->IsElimmed()) return;

	// 计算本帧应该回复的护盾值
	const float ReplenishThisFrame = ShieldReplenishRate * DeltaTime;
	// 应用护盾回复并确保护盾值不超过最大值
	Character->SetShield(FMath::Clamp(Character->GetShield() + ReplenishThisFrame, 0.f, Character->GetMaxShield()));
	// 更新UI显示的护盾值
	Character->UpdateHUDShield();
	// 减少还需要回复的护盾总量
	ShieldReplenishAmount -= ReplenishThisFrame;

	// 检查护盾回复是否完成（已达到所需回复量或护盾已满）
	if (ShieldReplenishAmount <= 0.f || Character->GetShield() >= Character->GetMaxShield())
	{
		// 重置护盾回复状态
		bReplenishingShield = false;
		ShieldReplenishAmount = 0.f;
	}
}

/**
 * 开始游戏函数：在组件被创建时调用，初始化组件状态
 */
void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

}

/**
 * 设置初始速度函数：保存角色的原始移动速度，用于增益效果结束后恢复
 * @param BaseSpeed 原始基础移动速度
 * @param CrouchSpeed 原始蹲伏移动速度
 */
void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed)
{
	// 保存原始基础速度
	InitialBaseSpeed = BaseSpeed;
	// 保存原始蹲伏速度
	InitialCrouchSpeed = CrouchSpeed;
}

/**
 * 设置初始跳跃速度函数：保存角色的原始跳跃速度，用于增益效果结束后恢复
 * @param Velocity 原始跳跃速度
 */
void UBuffComponent::SetInitialJumpVelocity(float Velocity)
{
	// 保存原始跳跃速度
	InitialJumpVelocity = Velocity;
}

/**
 * 速度增益函数：临时提升角色的移动速度，并设置定时器在指定时间后恢复
 * @param BuffBaseSpeed 增益后的基础移动速度
 * @param BuffCrouchSpeed 增益后的蹲伏移动速度
 * @param BuffTime 增益效果持续时间（秒）
 */
void UBuffComponent::BuffSpeed(float BuffBaseSpeed, float BuffCrouchSpeed, float BuffTime)
{
	// 检查角色是否存在
	if (Character == nullptr) return;

	// 设置定时器，在BuffTime秒后调用ResetSpeeds函数恢复原始速度
	Character->GetWorldTimerManager().SetTimer(
		SpeedBuffTimer,
		this,
		&UBuffComponent::ResetSpeeds,
		BuffTime
	);

	// 应用速度增益效果
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BuffBaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = BuffCrouchSpeed;
	}
	// 在所有客户端上同步速度增益效果
	MulticastSpeedBuff(BuffBaseSpeed, BuffCrouchSpeed);
}

/**
 * 重置速度函数：将角色的移动速度恢复到原始值
 */
void UBuffComponent::ResetSpeeds()
{
	// 检查角色是否存在及角色移动组件是否有效
	if (Character == nullptr || Character->GetCharacterMovement() == nullptr) return;

	// 恢复原始基础速度和蹲伏速度
	Character->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
	Character->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
	// 在所有客户端上同步速度恢复效果
	MulticastSpeedBuff(InitialBaseSpeed, InitialCrouchSpeed);
}

/**
 * 多播速度增益函数：在所有客户端上应用速度修改（包括服务器和所有客户端）
 * @param BaseSpeed 要设置的基础移动速度
 * @param CrouchSpeed 要设置的蹲伏移动速度
 */
void UBuffComponent::MulticastSpeedBuff_Implementation(float BaseSpeed, float CrouchSpeed)
{
	// 检查角色是否存在及角色移动组件是否有效
	if (Character && Character->GetCharacterMovement())
	{
		// 设置角色的移动速度
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
		Character->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
	}
}

/**
 * 跳跃增益函数：临时提升角色的跳跃高度，并设置定时器在指定时间后恢复
 * @param BuffJumpVelocity 增益后的跳跃速度
 * @param BuffTime 增益效果持续时间（秒）
 */
void UBuffComponent::BuffJump(float BuffJumpVelocity, float BuffTime)
{
	// 检查角色是否存在
	if (Character == nullptr) return;

	// 设置定时器，在BuffTime秒后调用ResetJump函数恢复原始跳跃速度
	Character->GetWorldTimerManager().SetTimer(
		JumpBuffTimer,
		this,
		&UBuffComponent::ResetJump,
		BuffTime
	);

	// 应用跳跃增益效果
	if (Character->GetCharacterMovement())
	{
		Character->GetCharacterMovement()->JumpZVelocity = BuffJumpVelocity;
	}
	// 在所有客户端上同步跳跃增益效果
	MulticastJumpBuff(BuffJumpVelocity);
}

/**
 * 多播跳跃增益函数：在所有客户端上应用跳跃速度修改（包括服务器和所有客户端）
 * @param JumpVelocity 要设置的跳跃速度
 */
void UBuffComponent::MulticastJumpBuff_Implementation(float JumpVelocity)
{
	// 检查角色是否存在及角色移动组件是否有效
	if (Character && Character->GetCharacterMovement())
	{
		// 设置角色的跳跃速度
		Character->GetCharacterMovement()->JumpZVelocity = JumpVelocity;
	}
}

/**
 * 重置跳跃函数：将角色的跳跃能力恢复到原始值
 */
void UBuffComponent::ResetJump()
{
	// 检查角色移动组件是否有效
	if (Character->GetCharacterMovement())
	{
		// 恢复原始跳跃速度
		Character->GetCharacterMovement()->JumpZVelocity = InitialJumpVelocity;
	}
	// 在所有客户端上同步跳跃恢复效果
	MulticastJumpBuff(InitialJumpVelocity);
}

/**
 * 每帧更新函数：处理正在进行的增益效果（治疗和护盾回复）
 * @param DeltaTime 帧时间（秒）
 * @param TickType 帧类型
 * @param ThisTickFunction 该组件的tick函数
 */
void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 处理治疗效果
	HealRampUp(DeltaTime);
	// 处理护盾回复效果
	ShieldRampUp(DeltaTime);
}

