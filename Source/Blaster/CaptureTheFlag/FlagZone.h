// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Blaster/BlasterTypes/Team.h"
#include "FlagZone.generated.h"

/**
 * 夺旗模式中的旗帜放置区域
 * 当敌方旗帜进入己方区域时，触发旗帜捕获逻辑
 */
UCLASS()
class BLASTER_API AFlagZone : public AActor
{
	GENERATED_BODY()
	
public:	
	/** 默认构造函数 */
	AFlagZone();

	/** 区域所属的队伍，用于判断是否为敌方旗帜进入 */
	UPROPERTY(EditAnywhere, Category = "Flag Zone", BlueprintReadOnly, meta = (ExposeOnSpawn = "true"))
	ETeam Team;
protected:
	/** 初始化组件和设置重叠事件 */
	virtual void BeginPlay() override;

	/**
	 * 处理与球体组件的重叠事件
	 * @param OverlappedComponent 与其他对象重叠的组件
	 * @param OtherActor 重叠的其他对象
	 * @param OtherComp 重叠的其他组件
	 * @param OtherBodyIndex 重叠的物理主体索引
	 * @param bFromSweep 是否由扫描触发
	 * @param SweepResult 扫描结果
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

private:

	/** 用于检测旗帜进入的球体碰撞组件 */
	UPROPERTY(EditAnywhere, Category = "Flag Zone", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* ZoneSphere;

public:    

};
