// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Casing.generated.h"

/**
 * 弹壳类：代表武器发射时抛出的弹壳，负责处理弹壳的物理效果和碰撞逻辑
 */
UCLASS()
class BLASTER_API ACasing : public AActor
{
	GENERATED_BODY()
	
public:	
	/**
	 * 构造函数：初始化弹壳的核心组件和属性设置
	 */
	ACasing();

protected:
	/**
	 * 游戏开始时的初始化函数，设置弹壳的物理属性和碰撞事件
	 */
	virtual void BeginPlay() override;

	/**
	 * 处理弹壳碰撞事件的回调函数
	 * @param HitComp 碰撞的组件
	 * @param OtherActor 被碰撞的Actor
	 * @param OtherComp 被碰撞的组件
	 * @param NormalImpulse 碰撞的法向冲量
	 * @param Hit 碰撞结果信息
	 */
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	/**
	 * 弹壳的静态网格组件，用于显示弹壳的视觉效果
	 */
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CasingMesh;

	/**
	 * 弹壳抛出时的冲量大小，控制弹壳被抛出的力度
	 */
	UPROPERTY(EditAnywhere)
	float ShellEjectionImpulse;

	/**
	 * 弹壳碰撞时播放的音效
	 */
	UPROPERTY(EditAnywhere)
	class USoundCue* ShellSound;
};
