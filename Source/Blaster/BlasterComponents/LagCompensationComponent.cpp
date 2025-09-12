// Fill out your copyright notice in the Description page of Project Settings.


#include "LagCompensationComponent.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/Weapon/Weapon.h"
#include "Components/BoxComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blaster/Blaster.h"
#include "DrawDebugHelpers.h"

/**
 * 延迟补偿组件的构造函数
 * 初始化组件并启用其Tick功能
 */
ULagCompensationComponent::ULagCompensationComponent()
{
	// 允许组件进行Tick更新，用于保存帧数据
	PrimaryComponentTick.bCanEverTick = true;
}

/**
 * 延迟补偿组件的BeginPlay函数
 * 组件初始化时调用，执行基类的BeginPlay逻辑
 */
void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();
}

/**
 * 在两个帧数据包之间进行插值，生成一个特定时间点的帧数据包
 * @param OlderFrame 较早的帧数据包
 * @param YoungerFrame 较新的帧数据包
 * @param HitTime 需要插值到的时间点
 * @return 插值后的帧数据包
 */
FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime)
{
	// 计算两个帧之间的时间差
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	// 计算插值比例，并确保其在0到1之间
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);

	// 创建插值后的帧数据包
	FFramePackage InterpFramePackage;
	// 设置插值帧的时间为命中时间
	InterpFramePackage.Time = HitTime;

	// 对每个碰撞盒信息进行插值计算
	for (auto& YoungerPair : YoungerFrame.HitBoxInfo)
	{
		const FName& BoxInfoName = YoungerPair.Key;

		// 获取两个帧中相同名称的碰撞盒信息
		const FBoxInformation& OlderBox = OlderFrame.HitBoxInfo[BoxInfoName];		const FBoxInformation& YoungerBox = YoungerFrame.HitBoxInfo[BoxInfoName];

		// 创建插值后的碰撞盒信息
		FBoxInformation InterpBoxInfo;

		// 对位置和旋转进行线性插值
		InterpBoxInfo.Location = FMath::VInterpTo(OlderBox.Location, YoungerBox.Location, 1.f, InterpFraction);
		InterpBoxInfo.Rotation = FMath::RInterpTo(OlderBox.Rotation, YoungerBox.Rotation, 1.f, InterpFraction);
		// 使用较新帧的盒体大小
		InterpBoxInfo.BoxExtent = YoungerBox.BoxExtent;

		// 将插值后的碰撞盒信息添加到帧数据包中
		InterpFramePackage.HitBoxInfo.Add(BoxInfoName, InterpBoxInfo);
	}

	// 返回插值后的帧数据包
	return InterpFramePackage;
}

/**
 * 确认是否命中目标角色，用于延迟补偿系统中的命中验证
 * @param Package 要检查的帧数据包
 * @param HitCharacter 被击中的角色
 * @param TraceStart 射线起始点
 * @param HitLocation 命中位置
 * @return 延迟补偿结果，包含是否命中和是否是头部命中
 */
FServerSideRewindResult ULagCompensationComponent::ConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation)
{
	// 如果目标角色为空，直接返回失败结果
	if (HitCharacter == nullptr) return FServerSideRewindResult();

	// 缓存当前帧的碰撞盒位置，用于之后恢复
	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	// 移动碰撞盒到指定帧的位置
	MoveBoxes(HitCharacter, Package);
	// 禁用角色网格的碰撞，避免干扰碰撞盒检测
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	// 首先启用头部碰撞盒的碰撞
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

	// 设置射线检测结果变量和射线终点（稍微延伸以确保能命中）
	FHitResult ConfirmHitResult;
	const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
	UWorld* World = GetWorld();
	if (World)
	{
		// 进行射线检测，检查是否命中头部
		World->LineTraceSingleByChannel(
			ConfirmHitResult,
			TraceStart,
			TraceEnd,
			ECC_HitBox
		);
		if (ConfirmHitResult.bBlockingHit) // 命中头部，提前返回
		{
			// 恢复碰撞盒位置和网格碰撞
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			// 返回成功结果，标记为头部命中
			return FServerSideRewindResult{ true, true };
		}
		else // 未命中头部，检查其他碰撞盒
		{
			// 启用所有碰撞盒的碰撞
			for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
			{
				if (HitBoxPair.Value != nullptr)
				{
					HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
					HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
				}
			}
			// 再次进行射线检测
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			if (ConfirmHitResult.bBlockingHit)
			{
				// 恢复碰撞盒位置和网格碰撞
				ResetHitBoxes(HitCharacter, CurrentFrame);
				EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
				// 返回成功结果，标记为非头部命中
				return FServerSideRewindResult{ true, false };
			}
		}
	}

	// 未命中任何碰撞盒，恢复碰撞盒位置和网格碰撞
	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	// 返回失败结果
	return FServerSideRewindResult{ false, false };
}

/**
 * 确认投射物是否命中目标角色，用于延迟补偿系统中的投射物命中验证
 * @param Package 要检查的帧数据包
 * @param HitCharacter 被击中的角色
 * @param TraceStart 投射物起始位置
 * @param InitialVelocity 投射物初速度
 * @param HitTime 命中时间
 * @return 延迟补偿结果，包含是否命中和是否是头部命中
 */
FServerSideRewindResult ULagCompensationComponent::ProjectileConfirmHit(const FFramePackage& Package, ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	// 缓存当前帧的碰撞盒位置，用于之后恢复
	FFramePackage CurrentFrame;
	CacheBoxPositions(HitCharacter, CurrentFrame);
	// 移动碰撞盒到指定帧的位置
	MoveBoxes(HitCharacter, Package);
	// 禁用角色网格的碰撞，避免干扰碰撞盒检测
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::NoCollision);

	// 首先启用头部碰撞盒的碰撞
	UBoxComponent* HeadBox = HitCharacter->HitCollisionBoxes[FName("head")];
	HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);

	// 设置投射物路径预测参数
	FPredictProjectilePathParams PathParams;
	PathParams.bTraceWithCollision = true; // 启用碰撞追踪
	PathParams.MaxSimTime = MaxRecordTime; // 最大模拟时间
	PathParams.LaunchVelocity = InitialVelocity; // 发射速度
	PathParams.StartLocation = TraceStart; // 起始位置
	PathParams.SimFrequency = 15.f; // 模拟频率
	PathParams.ProjectileRadius = 5.f; // 投射物半径
	PathParams.TraceChannel = ECC_HitBox; // 追踪通道
	PathParams.ActorsToIgnore.Add(GetOwner()); // 忽略自身

	// 预测投射物路径
	FPredictProjectilePathResult PathResult;
	UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);

	if (PathResult.HitResult.bBlockingHit) // 命中头部，提前返回
	{
		// 恢复碰撞盒位置和网格碰撞
		ResetHitBoxes(HitCharacter, CurrentFrame);
		EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
		// 返回成功结果，标记为头部命中
		return FServerSideRewindResult{ true, true };
	}
	else // 未命中头部，检查其他碰撞盒
	{
		// 启用所有碰撞盒的碰撞
		for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}

		// 再次预测投射物路径
		UGameplayStatics::PredictProjectilePath(this, PathParams, PathResult);
		if (PathResult.HitResult.bBlockingHit)
		{
			// 恢复碰撞盒位置和网格碰撞
			ResetHitBoxes(HitCharacter, CurrentFrame);
			EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
			// 返回成功结果，标记为非头部命中
			return FServerSideRewindResult{ true, false };
		}
	}

	// 未命中任何碰撞盒，恢复碰撞盒位置和网格碰撞
	ResetHitBoxes(HitCharacter, CurrentFrame);
	EnableCharacterMeshCollision(HitCharacter, ECollisionEnabled::QueryAndPhysics);
	// 返回失败结果
	return FServerSideRewindResult{ false, false };
}

/**
 * 确认霰弹枪是否命中目标角色，用于延迟补偿系统中的霰弹枪命中验证
 * @param FramePackages 要检查的帧数据包数组
 * @param TraceStart 射击起始点
 * @param HitLocations 命中位置数组
 * @return 霰弹枪延迟补偿结果，包含头部命中和身体命中的信息
 */
FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunConfirmHit(const TArray<FFramePackage>& FramePackages, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations)
{
	// 检查所有帧中的角色是否有效
	for (auto& Frame : FramePackages)
	{
		if (Frame.Character == nullptr) return FShotgunServerSideRewindResult();
	}
	// 创建霰弹枪命中结果
	FShotgunServerSideRewindResult ShotgunResult;
	// 用于存储当前帧的碰撞盒信息，便于后续恢复
	TArray<FFramePackage> CurrentFrames;
	// 准备所有角色的碰撞盒
	for (auto& Frame : FramePackages)
	{
		FFramePackage CurrentFrame;
		CurrentFrame.Character = Frame.Character;
		// 缓存当前碰撞盒位置
		CacheBoxPositions(Frame.Character, CurrentFrame);
		// 移动碰撞盒到指定帧的位置
		MoveBoxes(Frame.Character, Frame);
		// 禁用角色网格碰撞
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::NoCollision);
		// 保存当前帧信息
		CurrentFrames.Add(CurrentFrame);
	}

	// 首先启用所有角色的头部碰撞盒
	for (auto& Frame : FramePackages)
	{
		// 首先启用头部碰撞盒
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		HeadBox->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
	}

	// 获取世界指针
	UWorld* World = GetWorld();
	// 检查头部命中
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if (World)
		{
			// 进行射线检测
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			// 检查命中的是否是角色
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if (BlasterCharacter)
			{
				// 记录头部命中次数
				if (ShotgunResult.HeadShots.Contains(BlasterCharacter))
				{
					ShotgunResult.HeadShots[BlasterCharacter]++;
				}
				else
				{
					ShotgunResult.HeadShots.Emplace(BlasterCharacter, 1);
				}
			}
		}
	}

	// 启用所有碰撞盒，然后禁用头部碰撞盒
	for (auto& Frame : FramePackages)
	{
		for (auto& HitBoxPair : Frame.Character->HitCollisionBoxes)
		{
			if (HitBoxPair.Value != nullptr)
			{
				HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
				HitBoxPair.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block);
			}
		}
		// 禁用头部碰撞盒，只检测身体命中
		UBoxComponent* HeadBox = Frame.Character->HitCollisionBoxes[FName("head")];
		HeadBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 检查身体命中
	for (auto& HitLocation : HitLocations)
	{
		FHitResult ConfirmHitResult;
		const FVector TraceEnd = TraceStart + (HitLocation - TraceStart) * 1.25f;
		if (World)
		{
			// 进行射线检测
			World->LineTraceSingleByChannel(
				ConfirmHitResult,
				TraceStart,
				TraceEnd,
				ECC_HitBox
			);
			// 检查命中的是否是角色
			ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(ConfirmHitResult.GetActor());
			if (BlasterCharacter)
			{
				// 记录身体命中次数
				if (ShotgunResult.BodyShots.Contains(BlasterCharacter))
				{
					ShotgunResult.BodyShots[BlasterCharacter]++;
				}
				else
				{
					ShotgunResult.BodyShots.Emplace(BlasterCharacter, 1);
				}
			}
		}
	}

	// 恢复所有角色的碰撞盒位置和网格碰撞
	for (auto& Frame : CurrentFrames)
	{
		ResetHitBoxes(Frame.Character, Frame);
		EnableCharacterMeshCollision(Frame.Character, ECollisionEnabled::QueryAndPhysics);
	}

	// 返回霰弹枪命中结果
	return ShotgunResult;
}

/**
 * 缓存角色所有碰撞盒的位置、旋转和大小信息
 * @param HitCharacter 要缓存碰撞盒信息的角色
 * @param OutFramePackage 输出的帧数据包，用于存储碰撞盒信息
 */
void ULagCompensationComponent::CacheBoxPositions(ABlasterCharacter* HitCharacter, FFramePackage& OutFramePackage)
{
	// 检查角色是否有效
	if (HitCharacter == nullptr) return;
	// 遍历角色的所有碰撞盒
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		// 检查碰撞盒是否有效
		if (HitBoxPair.Value != nullptr)
		{
			// 创建碰撞盒信息结构体并填充数据
			FBoxInformation BoxInfo;
			BoxInfo.Location = HitBoxPair.Value->GetComponentLocation();
			BoxInfo.Rotation = HitBoxPair.Value->GetComponentRotation();
			BoxInfo.BoxExtent = HitBoxPair.Value->GetScaledBoxExtent();
			// 将碰撞盒信息添加到帧数据包中
			OutFramePackage.HitBoxInfo.Add(HitBoxPair.Key, BoxInfo);
		}
	}
}

/**
 * 移动角色的碰撞盒到指定帧数据包中的位置和旋转
 * @param HitCharacter 要移动碰撞盒的角色
 * @param Package 包含目标位置和旋转信息的帧数据包
 */
void ULagCompensationComponent::MoveBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	// 检查角色是否有效
	if (HitCharacter == nullptr) return;
	// 遍历角色的所有碰撞盒
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		// 检查碰撞盒是否有效
		if (HitBoxPair.Value != nullptr)
		{
			// 设置碰撞盒的位置、旋转和大小
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
		}
	}
}

/**
 * 重置角色的碰撞盒到指定帧数据包中的位置和旋转（用于恢复碰撞盒状态）
 * @param HitCharacter 要重置碰撞盒的角色
 * @param Package 包含原始位置和旋转信息的帧数据包
 */
void ULagCompensationComponent::ResetHitBoxes(ABlasterCharacter* HitCharacter, const FFramePackage& Package)
{
	// 检查角色是否有效
	if (HitCharacter == nullptr) return;
	// 遍历角色的所有碰撞盒
	for (auto& HitBoxPair : HitCharacter->HitCollisionBoxes)
	{
		// 检查碰撞盒是否有效
		if (HitBoxPair.Value != nullptr)
		{
			// 重置碰撞盒的位置
			HitBoxPair.Value->SetWorldLocation(Package.HitBoxInfo[HitBoxPair.Key].Location);
			// 重置碰撞盒的旋转
			HitBoxPair.Value->SetWorldRotation(Package.HitBoxInfo[HitBoxPair.Key].Rotation);
			// 重置碰撞盒的大小
			HitBoxPair.Value->SetBoxExtent(Package.HitBoxInfo[HitBoxPair.Key].BoxExtent);
			// 禁用碰撞盒的碰撞
			HitBoxPair.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
	}
}

/**
 * 启用或禁用角色网格的碰撞
 * @param HitCharacter 要设置碰撞状态的角色
 * @param CollisionEnabled 碰撞启用类型（NoCollision表示禁用，QueryAndPhysics表示启用）
 */
void ULagCompensationComponent::EnableCharacterMeshCollision(ABlasterCharacter* HitCharacter, ECollisionEnabled::Type CollisionEnabled)
{
	// 检查角色及其网格是否有效
	if (HitCharacter && HitCharacter->GetMesh())
	{
		// 设置角色网格的碰撞状态
		HitCharacter->GetMesh()->SetCollisionEnabled(CollisionEnabled);
	}
}

/**
 * 显示帧数据包中的碰撞盒信息（用于调试）
 * @param Package 要显示的帧数据包
 * @param Color 显示的颜色
 */
void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package, const FColor& Color)
{
	// 遍历帧数据包中的所有碰撞盒信息
	for (auto& BoxInfo : Package.HitBoxInfo)
	{
		// 绘制调试盒体
		DrawDebugBox(
			GetWorld(),
			BoxInfo.Value.Location,
			BoxInfo.Value.BoxExtent,
			FQuat(BoxInfo.Value.Rotation),
			Color,
			false,
			4.f
		);
	}
}

/**
 * 执行服务器端回退，确认击中目标角色
 * @param HitCharacter 被击中的角色
 * @param TraceStart 射线起始点
 * @param HitLocation 命中位置
 * @param HitTime 命中时间
 * @return 服务器端回退结果，包含是否命中和是否是头部命中
 */
FServerSideRewindResult ULagCompensationComponent::ServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	// 获取要检查的帧数据包
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	// 确认是否命中目标角色
	return ConfirmHit(FrameToCheck, HitCharacter, TraceStart, HitLocation);
}

/**
 * 执行投射物服务器端回退，确认投射物击中目标角色
 * @param HitCharacter 被击中的角色
 * @param TraceStart 投射物起始位置
 * @param InitialVelocity 投射物初始速度
 * @param HitTime 命中时间
 * @return 服务器端回退结果，包含是否命中和是否是头部命中
 */
FServerSideRewindResult ULagCompensationComponent::ProjectileServerSideRewind(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	// 获取要检查的帧数据包
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	// 确认投射物是否命中目标角色
	return ProjectileConfirmHit(FrameToCheck, HitCharacter, TraceStart, InitialVelocity, HitTime);
}

/**
 * 执行霰弹枪服务器端回退，确认霰弹枪击中目标角色
 * @param HitCharacters 被击中的角色数组
 * @param TraceStart 射击起始点
 * @param HitLocations 命中位置数组
 * @param HitTime 命中时间
 * @return 霰弹枪服务器端回退结果，包含头部命中和身体命中的信息
 */
FShotgunServerSideRewindResult ULagCompensationComponent::ShotgunServerSideRewind(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	// 为每个被击中的角色获取要检查的帧数据包
	TArray<FFramePackage> FramesToCheck;
	for (ABlasterCharacter* HitCharacter : HitCharacters)
	{
		FramesToCheck.Add(GetFrameToCheck(HitCharacter, HitTime));
	}

	// 确认霰弹枪是否命中目标角色
	return ShotgunConfirmHit(FramesToCheck, TraceStart, HitLocations);
}

/**
 * 获取用于检查命中的帧数据包
 * @param HitCharacter 被击中的角色
 * @param HitTime 命中时间
 * @return 用于检查的帧数据包
 */
FFramePackage ULagCompensationComponent::GetFrameToCheck(ABlasterCharacter* HitCharacter, float HitTime)
{
	// 检查角色及其延迟补偿组件是否有效
	bool bReturn = 
		HitCharacter == nullptr ||
		HitCharacter->GetLagCompensation() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensation()->FrameHistory.GetTail() == nullptr;
	if (bReturn) return FFramePackage();
	// 用于检查命中的帧数据包
	FFramePackage FrameToCheck;
	bool bShouldInterpolate = true;
	// 获取角色的帧历史记录
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensation()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;
	// 如果命中时间早于最旧的历史记录时间，则无法进行回退
	if (OldestHistoryTime > HitTime)
	{
		// 时间太早，无法进行服务器端回退
		return FFramePackage();
	}
	// 如果命中时间恰好等于最旧的历史记录时间，则使用该帧
	if (OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}
	// 如果命中时间晚于或等于最新的历史记录时间，则使用最新的帧
	if (NewestHistoryTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	// 寻找命中时间所在的两个相邻帧
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = Younger;
	while (Older->GetValue().Time > HitTime) // Older是否仍然比HitTime年轻？
	{
		// 向后遍历直到：OlderTime < HitTime < YoungerTime
		if (Older->GetNextNode() == nullptr) break;
		Older = Older->GetNextNode();
		if (Older->GetValue().Time > HitTime)
		{
			Younger = Older;
		}
	}
	// 如果找到恰好等于命中时间的帧，则直接使用该帧
	if (Older->GetValue().Time == HitTime) // 可能性极低，但如果找到则使用该帧
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}
	// 如果需要插值，则在找到的两个帧之间进行插值
	if (bShouldInterpolate)
	{
		// 在Younger和Older帧之间进行插值
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}
	// 设置帧数据包关联的角色
	FrameToCheck.Character = HitCharacter;
	return FrameToCheck;
}

/**
 * 服务器端分数请求实现函数（用于确认击中并应用伤害）
 * @param HitCharacter 被击中的角色
 * @param TraceStart 射线起始点
 * @param HitLocation 命中位置
 * @param HitTime 命中时间
 */
void ULagCompensationComponent::ServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& HitLocation, float HitTime)
{
	// 执行服务器端回退，确认击中
	FServerSideRewindResult Confirm = ServerSideRewind(HitCharacter, TraceStart, HitLocation, HitTime);

	// 检查角色、武器是否有效，以及是否确认命中
	if (Character && HitCharacter && Character->GetEquippedWeapon() && Confirm.bHitConfirmed)
	{
		// 根据是否是头部命中确定伤害值
		const float Damage = Confirm.bHeadShot ? Character->GetEquippedWeapon()->GetHeadShotDamage() : Character->GetEquippedWeapon()->GetDamage();

		// 应用伤害
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

/**
 * 投射物服务器端分数请求实现函数（用于确认投射物击中并应用伤害）
 * @param HitCharacter 被击中的角色
 * @param TraceStart 投射物起始位置
 * @param InitialVelocity 投射物初始速度
 * @param HitTime 命中时间
 */
void ULagCompensationComponent::ProjectileServerScoreRequest_Implementation(ABlasterCharacter* HitCharacter, const FVector_NetQuantize& TraceStart, const FVector_NetQuantize100& InitialVelocity, float HitTime)
{
	// 执行投射物服务器端回退，确认击中
	FServerSideRewindResult Confirm = ProjectileServerSideRewind(HitCharacter, TraceStart, InitialVelocity, HitTime);

	// 检查角色、武器是否有效，以及是否确认命中
	if (Character && HitCharacter && Confirm.bHitConfirmed && Character->GetEquippedWeapon())
	{
		// 根据是否是头部命中确定伤害值
		const float Damage = Confirm.bHeadShot ? Character->GetEquippedWeapon()->GetHeadShotDamage() : Character->GetEquippedWeapon()->GetDamage();

		// 应用伤害
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			Damage,
			Character->Controller,
			Character->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

/**
 * 霰弹枪服务器端分数请求实现函数（用于确认霰弹枪击中并应用伤害）
 * @param HitCharacters 被击中的角色数组
 * @param TraceStart 射击起始点
 * @param HitLocations 命中位置数组
 * @param HitTime 命中时间
 */
void ULagCompensationComponent::ShotgunServerScoreRequest_Implementation(const TArray<ABlasterCharacter*>& HitCharacters, const FVector_NetQuantize& TraceStart, const TArray<FVector_NetQuantize>& HitLocations, float HitTime)
{
	// 执行霰弹枪服务器端回退，确认击中
	FShotgunServerSideRewindResult Confirm = ShotgunServerSideRewind(HitCharacters, TraceStart, HitLocations, HitTime);

	// 遍历所有被击中的角色
	for (auto& HitCharacter : HitCharacters)
	{
		// 检查角色是否有效
		if (HitCharacter == nullptr || HitCharacter->GetEquippedWeapon() == nullptr || Character == nullptr) continue;
		float TotalDamage = 0.f;
		if (Confirm.HeadShots.Contains(HitCharacter))
		{
			float HeadShotDamage = Confirm.HeadShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetHeadShotDamage();
			TotalDamage += HeadShotDamage;
		}
		if (Confirm.BodyShots.Contains(HitCharacter))
		{
			float BodyShotDamage = Confirm.BodyShots[HitCharacter] * HitCharacter->GetEquippedWeapon()->GetDamage();
			TotalDamage += BodyShotDamage;
		}
		UGameplayStatics::ApplyDamage(
			HitCharacter,
			TotalDamage,
			Character->Controller,
			HitCharacter->GetEquippedWeapon(),
			UDamageType::StaticClass()
		);
	}
}

/**
 * 组件的Tick函数，每帧调用一次
 * 主要用于保存当前帧的角色状态数据
 * @param DeltaTime 帧时间间隔
 * @param TickType 帧类型
 * @param ThisTickFunction 帧函数指针
 */
void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	// 执行基类的Tick逻辑
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// 保存当前帧的数据
	SaveFramePackage();
}

/**
 * 保存当前角色的帧数据包到历史记录中
 * 管理帧历史的长度，确保不超过最大记录时间
 */
void ULagCompensationComponent::SaveFramePackage()
{
	// 检查角色是否有效且具有权限
	if (Character == nullptr || !Character->HasAuthority()) return;
	// 如果历史记录为空或只有一个帧数据，直接添加新帧
	if (FrameHistory.Num() <= 1)
	{
		FFramePackage ThisFrame;
		// 保存当前帧数据
		SaveFramePackage(ThisFrame);
		// 添加到历史记录头部
		FrameHistory.AddHead(ThisFrame);
	}
	else
	{
		// 计算当前历史记录的时间长度
		float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		// 移除过旧的帧数据，确保历史记录不超过最大记录时间
		while (HistoryLength > MaxRecordTime)
		{
			FrameHistory.RemoveNode(FrameHistory.GetTail());
			HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
		}
		FFramePackage ThisFrame;
		// 保存当前帧数据
		SaveFramePackage(ThisFrame);
		// 添加到历史记录头部
		FrameHistory.AddHead(ThisFrame);

		//ShowFramePackage(ThisFrame, FColor::Red);
	}
}

/**
 * 填充帧数据包的详细信息
 * @param Package 要填充的帧数据包引用
 */
void ULagCompensationComponent::SaveFramePackage(FFramePackage& Package)
{
	// 获取或确认角色引用
	Character = Character == nullptr ? Cast<ABlasterCharacter>(GetOwner()) : Character;
	if (Character)
	{
		// 设置帧数据包的时间戳
		Package.Time = GetWorld()->GetTimeSeconds();
		// 设置帧数据包关联的角色
		Package.Character = Character;
		// 遍历并保存所有碰撞盒的信息
		for (auto& BoxPair : Character->HitCollisionBoxes)
		{
			FBoxInformation BoxInformation;
			// 保存碰撞盒的位置、旋转和大小
			BoxInformation.Location = BoxPair.Value->GetComponentLocation();
			BoxInformation.Rotation = BoxPair.Value->GetComponentRotation();
			BoxInformation.BoxExtent = BoxPair.Value->GetScaledBoxExtent();
			// 添加到帧数据包的碰撞盒信息映射中
			Package.HitBoxInfo.Add(BoxPair.Key, BoxInformation);
		}
	}
}
