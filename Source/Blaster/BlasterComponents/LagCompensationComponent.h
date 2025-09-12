// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

// 盒子碰撞信息结构体
// 存储碰撞盒的位置、旋转和大小信息
USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	// 碰撞盒的世界空间位置
	UPROPERTY()
	FVector Location;

	// 碰撞盒的世界空间旋转
	UPROPERTY()
	FRotator Rotation;

	// 碰撞盒的大小范围
	UPROPERTY()
	FVector BoxExtent;
};

// 帧数据包结构体
// 存储特定时间点角色所有碰撞盒的信息
USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	// 记录的时间戳
	UPROPERTY()
	float Time;

	// 碰撞盒名称到碰撞盒信息的映射
	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	// 关联的角色指针
	UPROPERTY()
	ABlasterCharacter* Character;
};

// 服务器回退结果结构体
// 用于返回命中确认结果
USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	// 是否确认命中
	UPROPERTY()
	bool bHitConfirmed;

	// 是否命中头部
	UPROPERTY()
	bool bHeadShot;
};

// 霰弹枪服务器回退结果结构体
// 用于返回霰弹枪的命中确认结果
USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	// 头部命中次数映射表
	UPROPERTY()
	TMap<ABlasterCharacter*, uint32> HeadShots;

	// 身体命中次数映射表
	UPROPERTY()
	TMap<ABlasterCharacter*, uint32> BodyShots;

};

// 延迟补偿组件类
// 用于处理多人游戏中的网络延迟补偿，实现服务器端回退功能
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// 构造函数
	ULagCompensationComponent();
	
	// 友元类声明，允许BlasterCharacter访问私有成员
	friend class ABlasterCharacter;
	
	// 组件的Tick函数，每帧执行
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	// 在调试中显示帧数据包信息
	// @param Package 要显示的帧数据包
	// @param Color 显示的颜色
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

	/** 
	* 射线武器服务器端回退
	* @param HitCharacter 被击中的角色
	* @param TraceStart 射线起始位置
	* @param HitLocation 命中位置
	* @param HitTime 命中时间
	* @return 回退结果，包含是否命中和是否爆头
	*/
	FServerSideRewindResult ServerSideRewind(
		class ABlasterCharacter* HitCharacter, 
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize& HitLocation, 
		float HitTime);

	/** 
	* 投射物武器服务器端回退
	* @param HitCharacter 被击中的角色
	* @param TraceStart 投射物起始位置
	* @param InitialVelocity 投射物初始速度
	* @param HitTime 命中时间
	* @return 回退结果，包含是否命中和是否爆头
	*/
	FServerSideRewindResult ProjectileServerSideRewind(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	/** 
	* 霰弹枪服务器端回退
	* @param HitCharacters 被击中的角色数组
	* @param TraceStart 射击起始位置
	* @param HitLocations 命中位置数组
	* @param HitTime 命中时间
	* @return 霰弹枪回退结果，包含每个角色的头部和身体命中次数
	*/
	FShotgunServerSideRewindResult ShotgunServerSideRewind(
		const TArray<ABlasterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime);

	// 服务器端分数请求RPC函数（射线武器）
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime
	);

	// 服务器端分数请求RPC函数（投射物武器）
	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	// 服务器端分数请求RPC函数（霰弹枪）
	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(
		const TArray<ABlasterCharacter*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime
	);

protected:
	// 组件开始播放时调用
	virtual void BeginPlay() override;
	
	// 保存帧数据包（带参数版本）
	// @param Package 要保存的帧数据包引用
	void SaveFramePackage(FFramePackage& Package);
	
	// 在两个帧数据包之间进行插值
	// @param OlderFrame 较早的帧数据包
	// @param YoungerFrame 较新的帧数据包
	// @param HitTime 目标时间点
	// @return 插值后的帧数据包
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	
	// 缓存角色碰撞盒位置信息
	// @param HitCharacter 要缓存的角色
	// @param OutFramePackage 输出的帧数据包
	void CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage);
	
	// 移动角色的碰撞盒到指定帧数据包的位置
	// @param HitCharacter 要移动碰撞盒的角色
	// @param Package 指定的帧数据包
	void MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
	
	// 重置角色的碰撞盒到原始位置
	// @param HitCharacter 要重置碰撞盒的角色
	// @param Package 包含原始位置的帧数据包
	void ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package);
	
	// 启用或禁用角色网格体碰撞
	// @param HitCharacter 要修改的角色
	// @param CollisionEnabled 碰撞类型
	void EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	
	// 保存当前帧数据包（无参数版本）
	void SaveFramePackage();
	
	// 获取用于检查的帧数据包
	// @param HitCharacter 目标角色
	// @param HitTime 命中时间
	// @return 适当的帧数据包（可能是插值结果）
	FFramePackage GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime);

	/** 
	* 确认射线武器命中
	* @param Package 要检查的帧数据包
	* @param HitCharacter 被击中的角色
	* @param TraceStart 射线起始位置
	* @param HitLocation 命中位置
	* @return 命中确认结果
	*/
	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package,
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation);

	/** 
	* 确认投射物武器命中
	* @param Package 要检查的帧数据包
	* @param HitCharacter 被击中的角色
	* @param TraceStart 投射物起始位置
	* @param InitialVelocity 投射物初始速度
	* @param HitTime 命中时间
	* @return 命中确认结果
	*/
	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package,
		ABlasterCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	/** 
	* 确认霰弹枪命中
	* @param FramePackages 要检查的帧数据包数组
	* @param TraceStart 射击起始位置
	* @param HitLocations 命中位置数组
	* @return 霰弹枪命中确认结果
	*/

	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations
	);

private:

	// 组件所属的角色指针
	UPROPERTY()
	ABlasterCharacter* Character;

	// 控制器指针
	UPROPERTY()
	class ABlasterPlayerController* Controller;

	// 帧历史记录（双向链表）
	TDoubleLinkedList<FFramePackage> FrameHistory;

	// 最大记录时间（秒）
	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;

public:	
	
};
