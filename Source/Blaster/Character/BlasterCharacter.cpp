// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "Blaster/Weapon/Weapon.h"
#include "Blaster/BlasterComponents/CombatComponent.h"
#include "Blaster/BlasterComponents/BuffComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "BlasterAnimInstance.h"
#include "Blaster/Blaster.h"
#include "Blaster/PlayerController/BlasterPlayerController.h"
#include "Blaster/GameMode/BlasterGameMode.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "Blaster/PlayerState/BlasterPlayerState.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Components/BoxComponent.h"
#include "Blaster/BlasterComponents/LagCompensationComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Blaster/GameState/BlasterGameState.h"
#include "Blaster/PlayerStart/TeamPlayerStart.h"

/**
 * 构造函数：初始化角色组件和属性
 */
ABlasterCharacter::ABlasterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// 创建和配置相机臂组件（弹簧臂）
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	CameraBoom->TargetArmLength = 600.f; // 相机臂长度
	CameraBoom->bUsePawnControlRotation = true; // 控制旋转

	// 创建和配置跟随相机
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false; // 不使用控制器旋转

	// 设置角色旋转行为
	bUseControllerRotationYaw = false; // 不使用控制器旋转偏航
	GetCharacterMovement()->bOrientRotationToMovement = true; // 朝向移动方向旋转

	// 创建和配置头顶Widget组件
	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	// 创建和配置战斗组件
	Combat = CreateDefaultSubobject<UCombatComponent>(TEXT("Combat"));
	Combat->SetIsReplicated(true); // 设置为可复制

	// 创建和配置增益组件
	Buff = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	Buff->SetIsReplicated(true); // 设置为可复制

	// 创建和配置延迟补偿组件（用于网络同步）
	LagCompensation = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensation"));

	// 设置角色移动属性
	GetCharacterMovement()->NavAgentProps.bCanCrouch = true; // 允许蹲伏
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); // 忽略相机碰撞
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh); // 设置网格碰撞类型
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore); // 忽略相机碰撞
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block); // 阻挡可见性
	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 850.f); // 旋转速率

	// 初始化状态变量
	TurningInPlace = ETurningInPlace::ETIP_NotTurning; // 初始为不转向状态
	NetUpdateFrequency = 66.f; // 网络更新频率
	MinNetUpdateFrequency = 33.f; // 最小网络更新频率

	// 创建溶解时间线组件（用于角色死亡效果）
	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));

	// 创建和配置附加的手榴弹组件
	AttachedGrenade = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Attached Grenade"));
	AttachedGrenade->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 禁用碰撞

	/**
	* 为服务器端重绕功能创建碰撞盒（用于精确的命中检测）
	*/

	// 创建头部碰撞盒
	head = CreateDefaultSubobject<UBoxComponent>(TEXT("head"));
	head->SetupAttachment(GetMesh(), FName("head"));
	HitCollisionBoxes.Add(FName("head"), head);

	// 创建盆骨碰撞盒
	pelvis = CreateDefaultSubobject<UBoxComponent>(TEXT("pelvis"));
	pelvis->SetupAttachment(GetMesh(), FName("pelvis"));
	HitCollisionBoxes.Add(FName("pelvis"), pelvis);

	// 创建脊柱碰撞盒
	spine_02 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_02"));
	spine_02->SetupAttachment(GetMesh(), FName("spine_02"));
	HitCollisionBoxes.Add(FName("spine_02"), spine_02);

	spine_03 = CreateDefaultSubobject<UBoxComponent>(TEXT("spine_03"));
	spine_03->SetupAttachment(GetMesh(), FName("spine_03"));
	HitCollisionBoxes.Add(FName("spine_03"), spine_03);

	// 创建左臂碰撞盒
	upperarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_l"));
	upperarm_l->SetupAttachment(GetMesh(), FName("upperarm_l"));
	HitCollisionBoxes.Add(FName("upperarm_l"), upperarm_l);

	lowerarm_l = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_l"));
	lowerarm_l->SetupAttachment(GetMesh(), FName("lowerarm_l"));
	HitCollisionBoxes.Add(FName("lowerarm_l"), lowerarm_l);

	hand_l = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_l"));
	hand_l->SetupAttachment(GetMesh(), FName("hand_l"));
	HitCollisionBoxes.Add(FName("hand_l"), hand_l);

	// 创建右臂碰撞盒
	upperarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("upperarm_r"));
	upperarm_r->SetupAttachment(GetMesh(), FName("upperarm_r"));
	HitCollisionBoxes.Add(FName("upperarm_r"), upperarm_r);

	lowerarm_r = CreateDefaultSubobject<UBoxComponent>(TEXT("lowerarm_r"));
	lowerarm_r->SetupAttachment(GetMesh(), FName("lowerarm_r"));
	HitCollisionBoxes.Add(FName("lowerarm_r"), lowerarm_r);

	hand_r = CreateDefaultSubobject<UBoxComponent>(TEXT("hand_r"));
	hand_r->SetupAttachment(GetMesh(), FName("hand_r"));
	HitCollisionBoxes.Add(FName("hand_r"), hand_r);

	// 创建背部碰撞盒
	blanket = CreateDefaultSubobject<UBoxComponent>(TEXT("blanket"));
	blanket->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("blanket"), blanket);

	backpack = CreateDefaultSubobject<UBoxComponent>(TEXT("backpack"));
	backpack->SetupAttachment(GetMesh(), FName("backpack"));
	HitCollisionBoxes.Add(FName("backpack"), backpack);

	// 创建左腿碰撞盒
	thigh_l = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_l"));
	thigh_l->SetupAttachment(GetMesh(), FName("thigh_l"));
	HitCollisionBoxes.Add(FName("thigh_l"), thigh_l);

	calf_l = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_l"));
	calf_l->SetupAttachment(GetMesh(), FName("calf_l"));
	HitCollisionBoxes.Add(FName("calf_l"), calf_l);

	foot_l = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_l"));
	foot_l->SetupAttachment(GetMesh(), FName("foot_l"));
	HitCollisionBoxes.Add(FName("foot_l"), foot_l);

	// 创建右腿碰撞盒
	thigh_r = CreateDefaultSubobject<UBoxComponent>(TEXT("thigh_r"));
	thigh_r->SetupAttachment(GetMesh(), FName("thigh_r"));
	HitCollisionBoxes.Add(FName("thigh_r"), thigh_r);

	calf_r = CreateDefaultSubobject<UBoxComponent>(TEXT("calf_r"));
	calf_r->SetupAttachment(GetMesh(), FName("calf_r"));
	HitCollisionBoxes.Add(FName("calf_r"), calf_r);

	foot_r = CreateDefaultSubobject<UBoxComponent>(TEXT("foot_r"));
	foot_r->SetupAttachment(GetMesh(), FName("foot_r"));
	HitCollisionBoxes.Add(FName("foot_r"), foot_r);

	// 设置所有碰撞盒的碰撞属性
	for (auto Box : HitCollisionBoxes)
	{
		if (Box.Value)
		{
			Box.Value->SetCollisionObjectType(ECC_HitBox); // 设置碰撞类型为HitBox
			Box.Value->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); // 默认忽略所有通道
			Box.Value->SetCollisionResponseToChannel(ECC_HitBox, ECollisionResponse::ECR_Block); // 仅阻挡HitBox通道
			Box.Value->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 初始禁用碰撞
		}
	}
}

/**
 * 设置网络复制属性
 * \param OutLifetimeProps 输出参数：网络复制属性列表
 */
void ABlasterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 设置重叠武器只对拥有者复制
	DOREPLIFETIME_CONDITION(ABlasterCharacter, OverlappingWeapon, COND_OwnerOnly);
	// 设置健康值、护盾值和游戏性禁用标志为始终复制
	DOREPLIFETIME(ABlasterCharacter, Health);
	DOREPLIFETIME(ABlasterCharacter, Shield);
	DOREPLIFETIME(ABlasterCharacter, bDisableGameplay);
}

/**
 * 网络复制移动数据后的回调函数
 * 处理模拟代理（非本地控制）角色的转向判断和移动同步
 */
void ABlasterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement(); // 调用父类实现
	SimProxiesTurn(); // 处理模拟代理的转向判断
	TimeSinceLastMovementReplication = 0.f; // 重置最后一次移动复制时间
}

/**
 * 处理角色被消灭的逻辑
 * \param bPlayerLeftGame 标记玩家是否主动离开游戏
 */
void ABlasterCharacter::Elim(bool bPlayerLeftGame)
{
	DropOrDestroyWeapons(); // 丢弃或销毁所有武器
	MulticastElim(bPlayerLeftGame); // 在所有客户端上执行消灭效果
}

/**
 * 在所有客户端上执行角色消灭效果
 * \param bPlayerLeftGame 标记玩家是否主动离开游戏
 */
void ABlasterCharacter::MulticastElim_Implementation(bool bPlayerLeftGame)
{
	bLeftGame = bPlayerLeftGame;
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDWeaponAmmo(0); // 清空HUD上的武器弹药显示
	}
	bElimmed = true; // 标记角色已被消灭
	PlayElimMontage(); // 播放消灭动画蒙太奇
	// 开始溶解效果
	if (DissolveMaterialInstance)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), 0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}
	StartDissolve();

	// 禁用角色移动
	bDisableGameplay = true;
	GetCharacterMovement()->DisableMovement();
	if (Combat)
	{
		Combat->FireButtonPressed(false); // 确保停止射击
	}
	// 禁用碰撞
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AttachedGrenade->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 生成消灭机器人效果
	if (ElimBotEffect)
	{
		FVector ElimBotSpawnPoint(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + 200.f);
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotEffect,
			ElimBotSpawnPoint,
			GetActorRotation()
		);
	}
	if (ElimBotSound)
	{
		UGameplayStatics::SpawnSoundAtLocation(
			this,
			ElimBotSound,
			GetActorLocation()
		);
	}
	// 如果玩家正在使用狙击枪瞄准，则隐藏狙击镜
	bool bHideSniperScope = IsLocallyControlled() &&
		Combat &&
		Combat->bAiming &&
		Combat->EquippedWeapon &&
		Combat->EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle;
	if (bHideSniperScope)
	{
		ShowSniperScopeWidget(false);
	}
	// 移除皇冠效果（如果有）
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
	// 设置消灭延迟计时器
	GetWorldTimerManager().SetTimer(
		ElimTimer,
		this,
		&ABlasterCharacter::ElimTimerFinished,
		ElimDelay
	);
}

/**
 * 消灭计时器完成后的回调函数
 * 处理玩家重生或离开游戏的逻辑
 */
void ABlasterCharacter::ElimTimerFinished()
{
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	// 如果玩家没有离开游戏，则请求重生
	if (BlasterGameMode && !bLeftGame)
	{
		BlasterGameMode->RequestRespawn(this, Controller);
	}
	// 如果玩家主动离开游戏且是本地控制的，则广播离开游戏事件
	if (bLeftGame && IsLocallyControlled())
	{
		OnLeftGame.Broadcast();
	}
}

/**
 * 服务器端处理玩家离开游戏的实现函数
 * 通知游戏模式玩家已离开游戏
 */
void ABlasterCharacter::ServerLeaveGame_Implementation()
{
	// 获取游戏模式引用（如果尚未获取）
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	// 获取玩家状态引用（如果尚未获取）
	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	// 通知游戏模式玩家已离开游戏
	if (BlasterGameMode && BlasterPlayerState)
	{
		BlasterGameMode->PlayerLeftGame(BlasterPlayerState);
	}
}

/**
 * 丢弃或销毁指定的武器
 * 根据武器属性决定是直接销毁还是让它掉落在地面上
 * \param Weapon 要处理的武器对象指针
 */
void ABlasterCharacter::DropOrDestroyWeapon(AWeapon* Weapon)
{
	if (Weapon == nullptr) return; // 如果武器为空则直接返回
	// 根据武器的销毁标志决定处理方式
	if (Weapon->bDestroyWeapon)
	{
		Weapon->Destroy(); // 直接销毁武器
	}
	else
	{
		Weapon->Dropped(); // 让武器掉落在地面上
	}
}

/**
 * 丢弃或销毁玩家当前持有的所有武器和旗帜
 */
void ABlasterCharacter::DropOrDestroyWeapons()
{
	if (Combat)
	{
		// 处理主武器
		if (Combat->EquippedWeapon)
		{
			DropOrDestroyWeapon(Combat->EquippedWeapon);
		}
		// 处理副武器
		if (Combat->SecondaryWeapon)
		{
			DropOrDestroyWeapon(Combat->SecondaryWeapon);
		}
		// 处理旗帜（如果持有）
		if (Combat->TheFlag)
		{
			Combat->TheFlag->Dropped();
		}
	}
}

/**
 * 玩家状态初始化时的回调函数
 */
void ABlasterCharacter::OnPlayerStateInitialized()
{
	BlasterPlayerState->AddToScore(0.f); // 初始化分数为0
	BlasterPlayerState->AddToDefeats(0); // 初始化击败数为0
	SetTeamColor(BlasterPlayerState->GetTeam()); // 设置角色的团队颜色
	SetSpawnPoint(); // 设置角色的出生点
}

/**
 * 设置玩家的出生点位置
 * 仅在服务器端且玩家有队伍归属时执行
 * 从玩家所属队伍的可用出生点中随机选择一个
 */
void ABlasterCharacter::SetSpawnPoint()
{
	// 仅在服务器端且玩家有队伍归属时执行
	if (HasAuthority() && BlasterPlayerState->GetTeam() != ETeam::ET_NoTeam)
	{
		TArray<AActor*> PlayerStarts;
		// 获取所有TeamPlayerStart类型的出生点
		UGameplayStatics::GetAllActorsOfClass(this, ATeamPlayerStart::StaticClass(), PlayerStarts);
		TArray<ATeamPlayerStart*> TeamPlayerStarts;
		// 筛选出与玩家同队伍的出生点
		for (auto Start : PlayerStarts)
		{
			ATeamPlayerStart* TeamStart = Cast<ATeamPlayerStart>(Start);
			if (TeamStart && TeamStart->Team == BlasterPlayerState->GetTeam())
			{
				TeamPlayerStarts.Add(TeamStart);
			}
		}
		// 如果有可用的队伍出生点，则随机选择一个
		if (TeamPlayerStarts.Num() > 0)
		{
			ATeamPlayerStart* ChosenPlayerStart = TeamPlayerStarts[FMath::RandRange(0, TeamPlayerStarts.Num() - 1)];
			SetActorLocationAndRotation(
				ChosenPlayerStart->GetActorLocation(),
				ChosenPlayerStart->GetActorRotation()
			);
		}
	}
}

/**
 * 角色被销毁时的回调函数
 * 清理角色相关的资源和组件
 */
void ABlasterCharacter::Destroyed()
{
	Super::Destroyed(); // 调用父类实现

	// 清理消灭机器人效果组件
	if (ElimBotComponent)
	{
		ElimBotComponent->DestroyComponent();
	}

	// 获取游戏模式引用
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	// 检查比赛是否未进行中
	bool bMatchNotInProgress = BlasterGameMode && BlasterGameMode->GetMatchState() != MatchState::InProgress;
	// 如果比赛未进行中且角色有装备武器，则销毁该武器
	if (Combat && Combat->EquippedWeapon && bMatchNotInProgress)
	{
		Combat->EquippedWeapon->Destroy();
	}
}

/**
 * 在所有客户端上执行获得领先效果的实现函数
 * 为领先玩家显示皇冠效果
 */
void ABlasterCharacter::MulticastGainedTheLead_Implementation()
{
	if (CrownSystem == nullptr) return; // 如果皇冠特效系统为空则直接返回
	// 如果皇冠组件不存在，则创建新的皇冠组件
	if (CrownComponent == nullptr)
	{
		CrownComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
			CrownSystem, // 皇冠特效系统
			GetMesh(), // 附加到角色网格
			FName(), // 附加套接字名称（使用默认）
			GetActorLocation() + FVector(0.f, 0.f, 110.f), // 皇冠位置（角色头部上方）
			GetActorRotation(), // 皇冠旋转
			EAttachLocation::KeepWorldPosition, // 保持世界位置
			false // 不随父组件缩放
		);
	}
	// 如果皇冠组件存在，则激活它
	if (CrownComponent)
	{
		CrownComponent->Activate();
	}
}

/**
 * 在所有客户端上执行失去领先效果的实现函数
 * 移除领先玩家的皇冠效果
 */
void ABlasterCharacter::MulticastLostTheLead_Implementation()
{
	// 如果皇冠组件存在，则销毁它
	if (CrownComponent)
	{
		CrownComponent->DestroyComponent();
	}
}

/**
 * 设置角色的团队颜色
 * @param Team 团队枚举值（无团队、蓝队或红队）
 */
void ABlasterCharacter::SetTeamColor(ETeam Team)
{
	if (GetMesh() == nullptr || OriginalMaterial == nullptr) return; // 检查网格体和原始材质是否有效
	switch (Team)
	{
	case ETeam::ET_NoTeam: // 无团队时使用原始材质
		GetMesh()->SetMaterial(0, OriginalMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst; // 设置溶解效果材质
		break;
	case ETeam::ET_BlueTeam: // 蓝队时使用蓝色材质
		GetMesh()->SetMaterial(0, BlueMaterial);
		DissolveMaterialInstance = BlueDissolveMatInst;
		break;
	case ETeam::ET_RedTeam: // 红队时使用红色材质
		GetMesh()->SetMaterial(0, RedMaterial);
		DissolveMaterialInstance = RedDissolveMatInst;
		break;
	}
}

/**
 * 角色开始播放时的初始化函数
 */
void ABlasterCharacter::BeginPlay()
{
	Super::BeginPlay();

	// 仅在服务器端添加伤害接收回调
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &ABlasterCharacter::ReceiveDamage);
	}
	// 初始隐藏附加的手榴弹模型
	if (AttachedGrenade)
	{
		AttachedGrenade->SetVisibility(false);
	}
}

/**
 * 角色每帧更新函数
 * @param DeltaTime 帧间隔时间
 */
void ABlasterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime); // 更新原地转向状态
	HideCameraIfCharacterClose(); // 当相机靠近角色时隐藏角色模型
	PollInit(); // 轮询初始化玩家状态和控制器
}

/**
 * 处理角色的原地旋转逻辑
 * 根据角色状态和网络角色类型调整旋转行为
 * \param DeltaTime 帧间隔时间
 */
void ABlasterCharacter::RotateInPlace(float DeltaTime)
{
	// 如果角色持有旗帜，则使用不同的旋转设置
	if (Combat && Combat->bHoldingTheFlag)
	{
		bUseControllerRotationYaw = false; // 不使用控制器旋转Yaw
		GetCharacterMovement()->bOrientRotationToMovement = true; // 朝向移动方向旋转
		TurningInPlace = ETurningInPlace::ETIP_NotTurning; // 不在原地转向状态
		return;
	}
	// 如果装备了武器，则调整旋转设置
	if (Combat && Combat->EquippedWeapon) GetCharacterMovement()->bOrientRotationToMovement = false;
	if (Combat && Combat->EquippedWeapon) bUseControllerRotationYaw = true;
	// 如果游戏玩法被禁用，则重置旋转设置
	if (bDisableGameplay)
	{
		bUseControllerRotationYaw = false;
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}
	// 对于本地控制的角色（客户端或服务器），更新瞄准偏移
	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		AimOffset(DeltaTime);
	}
	// 对于模拟代理（非本地控制角色），定期更新动画参数
	else
	{
		TimeSinceLastMovementReplication += DeltaTime;
		// 如果超过一定时间未收到移动复制数据，则强制更新
		if (TimeSinceLastMovementReplication > 0.25f)
		{
			OnRep_ReplicatedMovement();
		}
		CalculateAO_Pitch(); // 计算俯仰角瞄准偏移
	}
}

/**
 * 设置玩家输入组件
 * 绑定各种输入事件到相应的处理函数
 * \param PlayerInputComponent 玩家输入组件指针
 */
void ABlasterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent); // 调用父类实现

	// 绑定动作输入
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ABlasterCharacter::Jump); // 跳跃输入

	// 绑定轴输入
	PlayerInputComponent->BindAxis("MoveForward", this, &ABlasterCharacter::MoveForward); // 前进/后退移动
	PlayerInputComponent->BindAxis("MoveRight", this, &ABlasterCharacter::MoveRight); // 左右移动
	PlayerInputComponent->BindAxis("Turn", this, &ABlasterCharacter::Turn); // 水平旋转（左右看）
	PlayerInputComponent->BindAxis("LookUp", this, &ABlasterCharacter::LookUp); // 垂直旋转（上下看）

	// 绑定武器和战斗相关输入
	PlayerInputComponent->BindAction("Equip", IE_Pressed, this, &ABlasterCharacter::EquipButtonPressed); // 装备武器
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ABlasterCharacter::CrouchButtonPressed); // 蹲下
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ABlasterCharacter::AimButtonPressed); // 开始瞄准
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ABlasterCharacter::AimButtonReleased); // 结束瞄准
	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ABlasterCharacter::FireButtonPressed); // 开始射击
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ABlasterCharacter::FireButtonReleased); // 结束射击
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &ABlasterCharacter::ReloadButtonPressed); // 换弹
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ABlasterCharacter::GrenadeButtonPressed); // 扔手雷
}

/**
 * 组件初始化完成后的回调函数
 * 在所有组件都创建并初始化后调用，用于进一步配置组件关系
 */
void ABlasterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents(); // 调用父类实现
	// 设置战斗组件的角色引用，建立组件间的联系
	if (Combat)
	{
		Combat->Character = this;
	}
	// 设置增益组件的角色引用和初始速度参数
	if (Buff)
	{
		Buff->Character = this; // 设置增益组件的角色引用
		// 设置初始移动速度基准值，用于后续的速度增益计算
		Buff->SetInitialSpeeds(
			GetCharacterMovement()->MaxWalkSpeed, // 初始行走速度
			GetCharacterMovement()->MaxWalkSpeedCrouched // 初始蹲伏速度
		);
		// 设置初始跳跃速度基准值，用于后续的跳跃增益计算
		Buff->SetInitialJumpVelocity(GetCharacterMovement()->JumpZVelocity); // 初始跳跃速度
	}
	// 设置延迟补偿组件的角色和控制器引用
	if (LagCompensation)
	{
		LagCompensation->Character = this;
		if (Controller)
		{
			LagCompensation->Controller = Cast<ABlasterPlayerController>(Controller);
		}
	}
}

/**
 * 播放射击动画蒙太奇
 * \param bAiming 是否处于瞄准状态
 */
void ABlasterCharacter::PlayFireMontage(bool bAiming)
{
	// 检查战斗组件和装备武器是否有效
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	// 获取动画实例并播放相应的动画蒙太奇
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireWeaponMontage)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName SectionName;
		// 根据是否瞄准选择不同的动画片段
		SectionName = bAiming ? FName("RifleAim") : FName("RifleHip");
		// 跳转到相应的动画片段
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayReloadMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ReloadMontage)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName SectionName;

		switch (Combat->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			SectionName = FName("Rifle");
			break;
		case EWeaponType::EWT_RocketLauncher:
			SectionName = FName("RocketLauncher");
			break;
		case EWeaponType::EWT_Pistol:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_SubmachineGun:
			SectionName = FName("Pistol");
			break;
		case EWeaponType::EWT_Shotgun:
			SectionName = FName("Shotgun");
			break;
		case EWeaponType::EWT_SniperRifle:
			SectionName = FName("SniperRifle");
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			SectionName = FName("GrenadeLauncher");
			break;
		}

		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::PlayElimMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
	{
		AnimInstance->Montage_Play(ElimMontage);
	}
}

void ABlasterCharacter::PlayThrowGrenadeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ThrowGrenadeMontage)
	{
		AnimInstance->Montage_Play(ThrowGrenadeMontage);
	}
}

void ABlasterCharacter::PlaySwapMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && SwapMontage)
	{
		AnimInstance->Montage_Play(SwapMontage);
	}
}

void ABlasterCharacter::PlayHitReactMontage()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return;

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName SectionName("FromFront");
		AnimInstance->Montage_JumpToSection(SectionName);
	}
}

void ABlasterCharacter::GrenadeButtonPressed()
{
	if (Combat)
	{
		if (Combat->bHoldingTheFlag) return;
		Combat->ThrowGrenade();
	}
}

/**
 * 处理角色受到伤害的回调函数
 * 计算并应用伤害到角色的护盾和生命值
 * \param DamagedActor 受到伤害的Actor
 * \param Damage 原始伤害值
 * \param DamageType 伤害类型
 * \param InstigatorController 造成伤害的控制器
 * \param DamageCauser 造成伤害的物体
 */
void ABlasterCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	// 获取游戏模式引用（如果尚未获取）
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	// 如果角色已经被消灭或者游戏模式为空，则直接返回
	if (bElimmed || BlasterGameMode == nullptr) return;
	// 通过游戏模式计算最终伤害（可能包含伤害减免逻辑）
	Damage = BlasterGameMode->CalculateDamage(InstigatorController, Controller, Damage);

	// 初始化对生命值的伤害为原始伤害
	float DamageToHealth = Damage;
	// 如果角色有护盾，则优先消耗护盾
	if (Shield > 0.f)
	{
		// 如果护盾值大于等于伤害，则完全由护盾吸收伤害
		if (Shield >= Damage)
		{
			Shield = FMath::Clamp(Shield - Damage, 0.f, MaxShield);
			DamageToHealth = 0.f;
		}
		// 如果护盾值小于伤害，则护盾先吸收全部伤害，剩余部分由生命值承担
		else
		{
			DamageToHealth = FMath::Clamp(DamageToHealth - Shield, 0.f, Damage);
			Shield = 0.f;
		}
	}

	// 应用伤害到生命值，并确保不低于0或超过最大值
	Health = FMath::Clamp(Health - DamageToHealth, 0.f, MaxHealth);

	// 更新HUD显示的健康值和护盾值
	UpdateHUDHealth();
	UpdateHUDShield();
	// 播放受击反应动画
	PlayHitReactMontage();

	// 如果生命值归零，则处理角色消灭
	if (Health == 0.f)
	{
		if (BlasterGameMode)
		{
			// 获取玩家控制器引用（如果尚未获取）
			BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
			// 获取攻击者的控制器引用
			ABlasterPlayerController* AttackerController = Cast<ABlasterPlayerController>(InstigatorController);
			// 通知游戏模式处理玩家消灭
			BlasterGameMode->PlayerEliminated(this, BlasterPlayerController, AttackerController);
		}
	}
}

/**
 * 处理前后移动输入
 * \param Value 移动输入值（-1.0到1.0）
 */
void ABlasterCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return; // 如果禁用游戏玩法则直接返回
	if (Controller != nullptr && Value != 0.f)
	{
		// 获取仅包含Yaw旋转的旋转器
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		// 计算前向方向向量
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X));
		// 应用移动输入
		AddMovementInput(Direction, Value);
	}
}

/**
 * 处理左右移动输入
 * \param Value 移动输入值（-1.0到1.0）
 */
void ABlasterCharacter::MoveRight(float Value)
{
	if (bDisableGameplay) return; // 如果禁用游戏玩法则直接返回
	if (Controller != nullptr && Value != 0.f)
	{
		// 获取仅包含Yaw旋转的旋转器
		const FRotator YawRotation(0.f, Controller->GetControlRotation().Yaw, 0.f);
		// 计算右侧方向向量
		const FVector Direction(FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y));
		// 应用移动输入
		AddMovementInput(Direction, Value);
	}
}

/**
 * 处理水平旋转输入
 * \param Value 旋转输入值
 */
void ABlasterCharacter::Turn(float Value)
{
	AddControllerYawInput(Value); // 应用控制器Yaw旋转输入
}

/**
 * 处理垂直旋转输入
 * \param Value 旋转输入值
 */
void ABlasterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value); // 应用控制器Pitch旋转输入
}

/**
 * 处理装备按钮按下事件
 */
void ABlasterCharacter::EquipButtonPressed()
{
	// 如果禁用游戏玩法则直接返回
	if (bDisableGameplay)
	{
		return;
	}
	
	if (Combat)
	{
		// 如果持有旗帜则不能装备武器
		if (Combat->bHoldingTheFlag)
		{
			return;
		}
		
		// 如果战斗状态空闲，则请求服务器装备武器
		if (Combat->CombatState == ECombatState::ECS_Unoccupied) 
		{
			ServerEquipButtonPressed();
		}
		
		// 检查是否应该在客户端先进行武器切换动画
		bool bSwap = Combat->ShouldSwapWeapons() &&
			!HasAuthority() &&
			Combat->CombatState == ECombatState::ECS_Unoccupied &&
			OverlappingWeapon == nullptr;

		if (bSwap)
		{
			PlaySwapMontage(); // 播放武器切换动画
			Combat->CombatState = ECombatState::ECS_SwappingWeapons; // 更新战斗状态
			bFinishedSwapping = false;
		}
	}
}

/**
 * 服务器端处理装备按钮按下事件的实现
 * 由客户端通过RPC调用
 */
void ABlasterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (Combat)
	{
		// 如果有重叠的武器，则装备它
		if (OverlappingWeapon)
		{
			Combat->EquipWeapon(OverlappingWeapon);
		}
		
		// 否则，如果可以切换武器，则切换主副武器
		else if (Combat->ShouldSwapWeapons())
		{
			Combat->SwapWeapons();
		}
	}
}

/**
 * 处理蹲伏按钮按下事件
 */
void ABlasterCharacter::CrouchButtonPressed()
{
	if (Combat && Combat->bHoldingTheFlag) return; // 如果持有旗帜则不能蹲伏
	if (bDisableGameplay) return; // 如果禁用游戏玩法则直接返回
	// 切换蹲伏状态
	if (bIsCrouched)
	{
		UnCrouch(); // 取消蹲伏
	}
	else
	{
		Crouch(); // 蹲伏
	}
}

/**
 * 处理换弹按钮按下事件
 */
void ABlasterCharacter::ReloadButtonPressed()
{
	if (Combat && Combat->bHoldingTheFlag) return; // 如果持有旗帜则不能换弹
	if (bDisableGameplay) return; // 如果禁用游戏玩法则直接返回
	if (Combat)
	{
		Combat->Reload(); // 通知战斗组件进行武器换弹
	}
}

/**
 * 处理瞄准按钮按下事件
 */
void ABlasterCharacter::AimButtonPressed()
{
	if (Combat && Combat->bHoldingTheFlag) return; // 如果持有旗帜则不能瞄准
	if (bDisableGameplay) return; // 如果禁用游戏玩法则直接返回
	if (Combat)
	{
		Combat->SetAiming(true); // 通知战斗组件设置瞄准状态为true
	}
}

/**
 * 处理瞄准按钮释放事件
 */
void ABlasterCharacter::AimButtonReleased()
{
	if (Combat && Combat->bHoldingTheFlag) return; // 如果持有旗帜则不能取消瞄准
	if (bDisableGameplay) return; // 如果禁用游戏玩法则直接返回
	if (Combat)
	{
		Combat->SetAiming(false); // 通知战斗组件设置瞄准状态为false
	}
}

/**
 * 计算角色的水平移动速度
 * \return 水平移动速度值
 */
float ABlasterCharacter::CalculateSpeed()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f; // 忽略垂直速度
	return Velocity.Size(); // 返回水平速度的大小
}

/**
 * 更新角色的瞄准偏移动画参数
 * 根据角色移动状态和瞄准方向调整角色动画
 * \param DeltaTime 帧间隔时间
 */
void ABlasterCharacter::AimOffset(float DeltaTime)
{
	if (Combat && Combat->EquippedWeapon == nullptr) return; // 如果没有装备武器则直接返回
	float Speed = CalculateSpeed(); // 计算水平移动速度
	bool bIsInAir = GetCharacterMovement()->IsFalling(); // 检查是否在空中

	// 角色静止不动且不在空中时的处理
	if (Speed == 0.f && !bIsInAir) // 静止站立，没有跳跃
	{
		bRotateRootBone = true; // 允许旋转根骨骼
		// 获取当前瞄准旋转（仅Yaw分量）
		FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		// 计算与起始瞄准方向的旋转差异
		FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRotation, StartingAimRotation);
		AO_Yaw = DeltaAimRotation.Yaw; // 设置Yaw方向的瞄准偏移
		// 如果不在原地转向状态，则插值更新瞄准偏移
		if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
		{
			InterpAO_Yaw = AO_Yaw;
		}
		bUseControllerRotationYaw = true; // 使用控制器旋转Yaw
		TurnInPlace(DeltaTime); // 处理原地转向
	}
	// 角色移动或在空中时的处理
	if (Speed > 0.f || bIsInAir) // 奔跑或跳跃
	{
		bRotateRootBone = false; // 禁止旋转根骨骼
		// 更新起始瞄准旋转
		StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f; // 重置Yaw方向瞄准偏移
		bUseControllerRotationYaw = true; // 使用控制器旋转Yaw
		TurningInPlace = ETurningInPlace::ETIP_NotTurning; // 重置原地转向状态
	}

	CalculateAO_Pitch(); // 计算Pitch方向瞄准偏移
}

/**
 * 计算角色的俯仰角瞄准偏移
 * 处理网络复制时的角度映射问题
 */
void ABlasterCharacter::CalculateAO_Pitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch; // 获取基础瞄准旋转的Pitch分量
	// 对于非本地控制的角色（网络复制的角色），需要特殊处理角度范围
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// 将pitch从[270, 360)范围映射到[-90, 0)范围
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

/**
 * 处理模拟代理（网络复制的角色）的转向判断
 * 确定非本地控制角色是否正在原地转向
 */
void ABlasterCharacter::SimProxiesTurn()
{
	if (Combat == nullptr || Combat->EquippedWeapon == nullptr) return; // 如果没有装备武器则直接返回
	bRotateRootBone = false; // 禁止旋转根骨骼
	float Speed = CalculateSpeed(); // 计算水平移动速度
	// 如果角色在移动，则不在原地转向状态
	if (Speed > 0.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		return;
	}

	// 保存上一帧的代理旋转并获取当前旋转
	ProxyRotationLastFrame = ProxyRotation;
	ProxyRotation = GetActorRotation();
	// 计算Yaw方向的旋转差异
	ProxyYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotation, ProxyRotationLastFrame).Yaw;

	// 根据旋转差异判断转向方向
	if (FMath::Abs(ProxyYaw) > TurnThreshold)
	{
		if (ProxyYaw > TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Right; // 向右转向
		}
		else if (ProxyYaw < -TurnThreshold)
		{
			TurningInPlace = ETurningInPlace::ETIP_Left; // 向左转向
		}
		else
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}
	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

}

void ABlasterCharacter::Jump()
{
	if (Combat && Combat->bHoldingTheFlag) return;
	if (bDisableGameplay) return;
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void ABlasterCharacter::FireButtonPressed()
{
	if (Combat && Combat->bHoldingTheFlag)
	{
		return;
	}
	
	if (bDisableGameplay)
	{
		return;
	}
	
	if (Combat)
	{
		Combat->FireButtonPressed(true);
	}
}

void ABlasterCharacter::FireButtonReleased()
{
	if (Combat && Combat->bHoldingTheFlag)
	{
		return;
	}
	
	if (bDisableGameplay)
	{
		return;
	}
	
	if (Combat)
	{
		Combat->FireButtonPressed(false);
	}
}

void ABlasterCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 4.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}

void ABlasterCharacter::HideCameraIfCharacterClose()
{
	if (!IsLocallyControlled()) return;
	if ((FollowCamera->GetComponentLocation() - GetActorLocation()).Size() < CameraThreshold)
	{
		GetMesh()->SetVisibility(false);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
		if (Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = true;
		}
	}
	else
	{
		GetMesh()->SetVisibility(true);
		if (Combat && Combat->EquippedWeapon && Combat->EquippedWeapon->GetWeaponMesh())
		{
			Combat->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
		if (Combat && Combat->SecondaryWeapon && Combat->SecondaryWeapon->GetWeaponMesh())
		{
			Combat->SecondaryWeapon->GetWeaponMesh()->bOwnerNoSee = false;
		}
	}
}

void ABlasterCharacter::OnRep_Health(float LastHealth)
{
	UpdateHUDHealth();
	if (Health < LastHealth)
	{
		PlayHitReactMontage();
	}
}

void ABlasterCharacter::OnRep_Shield(float LastShield)
{
	UpdateHUDShield();
	if (Shield < LastShield)
	{
		PlayHitReactMontage();
	}
}

/**
 * 更新HUD上显示的健康值
 * 通过玩家控制器设置当前健康值和最大健康值
 */
void ABlasterCharacter::UpdateHUDHealth()
{
	// 获取玩家控制器引用（如果尚未获取）
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	// 如果玩家控制器有效，则更新HUD上的健康值显示
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDHealth(Health, MaxHealth);
	}
}

/**
 * 更新HUD上显示的护盾值
 * 通过玩家控制器设置当前护盾值和最大护盾值
 */
void ABlasterCharacter::UpdateHUDShield()
{
	// 获取玩家控制器引用（如果尚未获取）
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	// 如果玩家控制器有效，则更新HUD上的护盾值显示
	if (BlasterPlayerController)
	{
		BlasterPlayerController->SetHUDShield(Shield, MaxShield);
	}
}

/**
 * 更新HUD上显示的弹药数量
 * 通过玩家控制器设置当前携带的总弹药数和装备武器的当前弹药数
 */
void ABlasterCharacter::UpdateHUDAmmo()
{
	// 获取玩家控制器引用（如果尚未获取）
	BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
	// 如果玩家控制器、战斗组件和装备武器都有效，则更新HUD上的弹药显示
	if (BlasterPlayerController && Combat && Combat->EquippedWeapon)
	{
		// 更新HUD上显示的携带总弹药数
		BlasterPlayerController->SetHUDCarriedAmmo(Combat->CarriedAmmo);
		// 更新HUD上显示的当前武器弹药数
		BlasterPlayerController->SetHUDWeaponAmmo(Combat->EquippedWeapon->GetAmmo());
	}
}

/**
 * 生成角色的默认武器
 * 在角色初始化时自动为其装备默认武器
 */
void ABlasterCharacter::SpawDefaultWeapon()
{
	// 获取游戏模式引用（如果尚未获取）
	BlasterGameMode = BlasterGameMode == nullptr ? GetWorld()->GetAuthGameMode<ABlasterGameMode>() : BlasterGameMode;
	UWorld* World = GetWorld();
	// 检查必要条件：游戏模式有效、世界存在、角色未被消灭、默认武器类存在
	if (BlasterGameMode && World && !bElimmed && DefaultWeaponClass)
	{
		// 生成默认武器实例
		AWeapon* StartingWeapon = World->SpawnActor<AWeapon>(DefaultWeaponClass);
		StartingWeapon->bDestroyWeapon = true; // 标记武器在角色消灭时销毁
		// 如果战斗组件有效，则装备生成的武器
		if (Combat)
		{
			Combat->EquipWeapon(StartingWeapon);
		}
	}
}

/**
 * 轮询初始化玩家状态和控制器
 * 确保玩家状态、控制器和相关组件都正确初始化
 */
void ABlasterCharacter::PollInit()
{
	// 初始化玩家状态
	if (BlasterPlayerState == nullptr)
	{
		BlasterPlayerState = GetPlayerState<ABlasterPlayerState>();
		if (BlasterPlayerState)
		{
			OnPlayerStateInitialized(); // 调用玩家状态初始化函数

			// 检查玩家是否处于领先位置，如果是则触发领先效果
			ABlasterGameState* BlasterGameState = Cast<ABlasterGameState>(UGameplayStatics::GetGameState(this));

			if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(BlasterPlayerState))
			{
				MulticastGainedTheLead();
			}
		}
	}
	// 初始化玩家控制器
	if (BlasterPlayerController == nullptr)
	{
		BlasterPlayerController = BlasterPlayerController == nullptr ? Cast<ABlasterPlayerController>(Controller) : BlasterPlayerController;
		if (BlasterPlayerController)
		{
			SpawDefaultWeapon(); // 生成默认武器
			UpdateHUDAmmo(); // 更新HUD弹药显示
			UpdateHUDHealth(); // 更新HUD健康值显示
			UpdateHUDShield(); // 更新HUD护盾值显示
		}
	}
}

/**
 * 更新溶解材质的参数值
 * 控制角色网格的溶解效果进度
 * \param DissolveValue 溶解效果参数值（0-1范围）
 */
void ABlasterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	// 检查动态溶解材质实例是否有效
	if (DynamicDissolveMaterialInstance)
	{
		// 设置溶解效果的参数值
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Dissolve"), DissolveValue);
	}
}

/**
 * 开始角色网格的溶解效果
 * 绑定溶解轨道函数并启动溶解时间线
 */
void ABlasterCharacter::StartDissolve()
{
	// 绑定溶解轨道函数到UpdateDissolveMaterial
	DissolveTrack.BindDynamic(this, &ABlasterCharacter::UpdateDissolveMaterial);
	// 检查溶解曲线和时间线是否有效
	if (DissolveCurve && DissolveTimeline)
	{
		// 添加溶解曲线到时间线并设置更新函数
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		// 开始播放溶解时间线
		DissolveTimeline->Play();
	}
}

/**
 * 设置角色当前重叠的武器
 * 处理武器拾取界面的显示/隐藏
 * \param Weapon 重叠的武器对象指针
 */
void ABlasterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	// 如果之前有重叠的武器，则隐藏其拾取界面
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	
	// 更新重叠武器引用
	OverlappingWeapon = Weapon;
	
	// 仅对本地控制角色处理拾取界面显示
	if (IsLocallyControlled())
	{
		// 如果有新的重叠武器，则显示其拾取界面
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

/**
 * 网络复制重叠武器后的回调函数
 * 处理武器拾取界面在客户端上的显示/隐藏更新
 * \param LastWeapon 之前重叠的武器对象指针
 */
void ABlasterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	// 如果当前有重叠的武器，则显示其拾取界面
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}
	
	// 如果之前有重叠的武器，则隐藏其拾取界面
	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

/**
 * 检查角色是否装备了武器
 * \return 如果装备了武器则返回true，否则返回false
 */
bool ABlasterCharacter::IsWeaponEquipped()
{
	// 检查战斗组件和装备武器是否存在
	return (Combat && Combat->EquippedWeapon);
}

/**
 * 检查角色是否正在瞄准
 * \return 如果正在瞄准则返回true，否则返回false
 */
bool ABlasterCharacter::IsAiming()
{
	// 检查战斗组件是否存在以及是否处于瞄准状态
	return (Combat && Combat->bAiming);
}

/**
 * 获取角色当前装备的武器
 * \return 当前装备的武器对象指针，如果没有则返回nullptr
 */
AWeapon* ABlasterCharacter::GetEquippedWeapon()
{
	// 检查战斗组件是否存在
	if (Combat == nullptr) return nullptr;
	// 返回装备的武器引用
	return Combat->EquippedWeapon;
}

/**
 * 获取角色当前的射击目标位置
 * \return 射击目标的世界坐标
 */
FVector ABlasterCharacter::GetHitTarget() const
{
	// 检查战斗组件是否存在
	if (Combat == nullptr) return FVector();
	// 返回射击目标位置
	return Combat->HitTarget;
}

/**
 * 获取角色当前的战斗状态
 * \return 当前的战斗状态枚举值
 */
ECombatState ABlasterCharacter::GetCombatState() const
{
	// 检查战斗组件是否存在
	if (Combat == nullptr) return ECombatState::ECS_MAX;
	// 返回当前战斗状态
	return Combat->CombatState;
}

/**
 * 检查角色是否正在本地重新加载武器
 * \return 如果正在本地重新加载则返回true，否则返回false
 */
bool ABlasterCharacter::IsLocallyReloading()
{
	// 检查战斗组件是否存在
	if (Combat == nullptr) return false;
	// 返回本地重新加载状态
	return Combat->bLocallyReloading;
}

/**
 * 检查角色是否持有旗帜（夺旗模式）
 * \return 如果持有旗帜则返回true，否则返回false
 */
bool ABlasterCharacter::IsHoldingTheFlag() const
{
	// 检查战斗组件是否存在
	if (Combat == nullptr) return false;
	// 返回持有旗帜状态
	return Combat->bHoldingTheFlag;
}

/**
 * 获取角色所属的队伍
 * \return 角色所属的队伍枚举值
 */
ETeam ABlasterCharacter::GetTeam()
{
	// 获取玩家状态引用（如果尚未获取）
	BlasterPlayerState = BlasterPlayerState == nullptr ? GetPlayerState<ABlasterPlayerState>() : BlasterPlayerState;
	// 检查玩家状态是否存在
	if (BlasterPlayerState == nullptr) return ETeam::ET_NoTeam;
	// 返回玩家所属队伍
	return BlasterPlayerState->GetTeam();
}

/**
 * 设置角色是否持有旗帜（夺旗模式）
 * \param bHolding 是否持有旗帜的布尔值
 */
void ABlasterCharacter::SetHoldingTheFlag(bool bHolding)
{
	// 检查战斗组件是否存在
	if (Combat == nullptr) return;
	// 设置持有旗帜状态
	Combat->bHoldingTheFlag = bHolding;
}