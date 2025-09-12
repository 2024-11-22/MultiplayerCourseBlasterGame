// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "ProjectileWeapon.generated.h"

/**
 * 投射物武器类：代表游戏中发射实体投射物的武器，继承自基础武器类
 * 负责处理投射物的生成、发射轨迹计算和服务器端重绕(SSR)逻辑
 * 示例：火箭发射器、榴弹发射器等
 */
UCLASS()
class BLASTER_API AProjectileWeapon : public AWeapon
{
	GENERATED_BODY()
public:
	/**
	 * 发射投射物的函数
	 * @param HitTarget 击中目标的位置向量，用于计算投射物的飞行方向
	 */
	virtual void Fire(const FVector& HitTarget) override;
	
private:
	/**
	 * 普通投射物类，用于创建标准的投射物实例（无网络延迟补偿）
	 * 通常用于服务器端直接生成的投射物
	 */
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<class AProjectile> ProjectileClass;

	/**
	 * 支持服务器端重绕的投射物类，用于创建带网络延迟补偿的投射物实例
	 * 通常用于客户端生成的投射物，需要服务器验证命中位置
	 */
	UPROPERTY(EditAnywhere, Category = "Projectile")
	TSubclassOf<AProjectile> ServerSideRewindProjectileClass;
};
