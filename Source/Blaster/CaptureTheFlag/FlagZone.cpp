// Fill out your copyright notice in the Description page of Project Settings.


#include "FlagZone.h"
#include "Components/SphereComponent.h"
#include "Blaster/Weapon/Flag.h"
#include "Blaster/GameMode/CaptureTheFlagGameMode.h"
#include "Blaster/Character/BlasterCharacter.h"

/**
 * 构造函数
 * 初始化FlagZone对象的基本属性和组件
 */
AFlagZone::AFlagZone()
{
	// 设置此Actor不需要每帧更新
	PrimaryActorTick.bCanEverTick = false;

	// 创建并初始化球体碰撞组件
	ZoneSphere = CreateDefaultSubobject<USphereComponent>(TEXT("ZoneSphere"));
	SetRootComponent(ZoneSphere);
}

/**
 * 游戏开始时调用
 * 设置重叠事件的回调函数
 */
void AFlagZone::BeginPlay()
{
	Super::BeginPlay();
	
	// 绑定重叠开始事件到OnSphereOverlap函数
	ZoneSphere->OnComponentBeginOverlap.AddDynamic(this, &AFlagZone::OnSphereOverlap);
}

/**
 * 处理与球体组件的重叠事件
 * 当敌方旗帜进入本区域时，触发旗帜捕获逻辑
 * 
 * @param OverlappedComponent 与其他对象重叠的组件
 * @param OtherActor 重叠的其他对象
 * @param OtherComp 重叠的其他组件
 * @param OtherBodyIndex 重叠的物理主体索引
 * @param bFromSweep 是否由扫描触发
 * @param SweepResult 扫描结果
 */
void AFlagZone::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 尝试将重叠的Actor转换为Flag类
	AFlag* OverlappingFlag = Cast<AFlag>(OtherActor);
	// 检查是否是敌方旗帜（旗帜队伍与区域队伍不同）
	if (OverlappingFlag && OverlappingFlag->GetTeam() != Team)
	{
		// 获取游戏模式并调用旗帜捕获函数
		ACaptureTheFlagGameMode* GameMode = GetWorld()->GetAuthGameMode<ACaptureTheFlagGameMode>();
		if (GameMode)
		{
			GameMode->FlagCaptured(OverlappingFlag, this);
		}
		// 重置旗帜到原始位置
		OverlappingFlag->ResetFlag();
	}
}
