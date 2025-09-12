// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "BlasterAnimInstance.generated.h"

/**
 * 角色动画实例类：负责控制角色的动画状态和骨骼变换
 */
UCLASS()
class BLASTER_API UBlasterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	/**
	* 初始化动画实例：在动画实例创建时调用，设置角色引用
	*/
	virtual void NativeInitializeAnimation() override;
	/**
	* 更新动画实例：每帧调用，更新动画状态和骨骼变换
	* @param DeltaTime 帧间隔时间
	*/
	virtual void NativeUpdateAnimation(float DeltaTime) override;

private:
	/**
	* 角色引用：指向拥有此动画实例的角色
	*/
	UPROPERTY(BlueprintReadOnly, Category = Character, meta = (AllowPrivateAccess = "true"))
	class ABlasterCharacter* BlasterCharacter;

	/**
	* 角色移动速度：水平方向上的移动速度
	*/
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Speed;

	/**
	* 是否在空中：角色当前是否处于跳跃或下落状态
	*/
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	/**
	* 是否在加速：角色当前是否正在加速移动
	*/
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	/**
	* 是否装备武器：角色当前是否装备了武器
	*/
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquipped;

	/**
	* 已装备的武器：角色当前装备的武器引用
	*/
	class AWeapon* EquippedWeapon;

	/**
	* 是否蹲下：角色当前是否处于蹲下状态
	*/
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsCrouched;

	/**
	* 是否在瞄准：角色当前是否处于瞄准状态
	*/
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	/**
	* 偏航偏移：角色移动方向与瞄准方向之间的水平夹角
	*/
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float YawOffset;

	/**
	* 倾斜角度：角色转向时的身体倾斜角度
	*/
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Lean;

	/**
	* 上一帧角色旋转：用于计算角色旋转速度
	*/
	FRotator CharacterRotationLastFrame;
	/**
	* 当前角色旋转：角色当前的旋转状态
	*/
	FRotator CharacterRotation;
	/**
	* 旋转增量：角色旋转的变化量
	*/
	FRotator DeltaRotation;

	/**
	* 瞄准偏航角：角色的水平瞄准偏移角
	*/
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Yaw;

	/**
	* 瞄准俯仰角：角色的垂直瞄准偏移角
	*/
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Pitch;

	/**
	* 左手变换：控制左手骨骼的位置和旋转
	*/
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform;

	/**
	* 原地旋转状态：角色当前的原地旋转状态
	*/
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	ETurningInPlace TurningInPlace;

	/**
	* 右手旋转：控制右手骨骼的旋转，用于武器瞄准
	*/
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FRotator RightHandRotation;

	/**
	* 是否本地控制：角色是否由本地玩家控制
	*/
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bLocallyControlled;

	/**
	* 是否旋转根骨：是否应该旋转角色的根骨骼
	*/
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bRotateRootBone;

	/**
	* 是否已被淘汰：角色当前是否处于淘汰状态
	*/
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bElimmed;

	/**
	* 是否使用FABRIK：是否使用FABRIK算法进行骨骼IK计算
	*/
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bUseFABRIK;

	/**
	* 是否使用瞄准偏移：是否应用瞄准偏移动画
	*/
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bUseAimOffsets;

	/**
	* 是否变换右手：是否变换右手骨骼以跟随瞄准目标
	*/
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bTransformRightHand;

	/**
	* 是否持有旗帜：角色当前是否持有旗帜
	*/
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bHoldingTheFlag;
};
