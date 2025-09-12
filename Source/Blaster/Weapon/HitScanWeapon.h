// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "HitScanWeapon.generated.h"

/**
 * 即时命中武器类：代表游戏中使用射线追踪进行即时命中检测的武器，继承自基础武器类
 * 负责处理射线发射、命中检测和视觉/音效反馈
 */
UCLASS()
class BLASTER_API AHitScanWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	/**
	 * 发射武器的函数，使用射线追踪实现即时命中检测
	 * @param HitTarget 击中目标的位置向量
	 */
	virtual void Fire(const FVector& HitTarget) override;
protected:

	/**
	 * 执行武器射线追踪，检测命中结果
	 * @param TraceStart 射线起点
	 * @param HitTarget 射线终点
	 * @param OutHit 输出参数，用于存储命中结果
	 */
	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);

	/**
	 * 命中物体时播放的粒子效果
	 */
	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	/**
	 * 命中物体时播放的音效
	 */
	UPROPERTY(EditAnywhere)
	USoundCue* HitSound;
private:

	/**
	 * 从枪口到命中点的光束粒子效果
	 */
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	/**
	 * 开火时枪口闪光的粒子效果
	 */
	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	/**
	 * 开火时播放的音效
	 */
	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;

};
