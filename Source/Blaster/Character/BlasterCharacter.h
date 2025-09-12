// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Blaster/BlasterTypes/TurningInPlace.h"
#include "Blaster/Interfaces/InteractWithCrosshairsInterface.h"
#include "Components/TimelineComponent.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "Blaster/BlasterTypes/Team.h"
#include "BlasterCharacter.generated.h"

/**
 * 玩家角色委托：在玩家离开游戏时广播
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

/**
 * 游戏角色类：管理玩家角色的基本行为、战斗能力和状态
 */
UCLASS()
class BLASTER_API ABlasterCharacter : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	/**
	* 构造函数：初始化角色组件和属性
	*/
	ABlasterCharacter();
	
	/**
	* 每帧更新：处理角色状态和行为更新
	* @param DeltaTime 帧间隔时间
	*/
	virtual void Tick(float DeltaTime) override;
	
	/**
	* 设置玩家输入组件：绑定输入事件到对应函数
	* @param PlayerInputComponent 玩家输入组件
	*/
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	
	/**
	* 获取生命周期复制属性：定义需要在网络上复制的属性
	* @param OutLifetimeProps 输出的生命周期属性列表
	*/
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	/**
	* 初始化组件后：设置组件之间的引用关系
	*/
	virtual void PostInitializeComponents() override;

	/**
	* 播放动画蒙太奇
	*/
	/**
	* 播放射击动画蒙太奇
	* @param bAiming 是否在瞄准状态
	*/
	void PlayFireMontage(bool bAiming);
	/**
	* 播放换弹动画蒙太奇
	*/
	void PlayReloadMontage();
	/**
	* 播放淘汰动画蒙太奇
	*/
	void PlayElimMontage();
	/**
	* 播放投掷手榴弹动画蒙太奇
	*/
	void PlayThrowGrenadeMontage();
	/**
	* 播放武器切换动画蒙太奇
	*/
	void PlaySwapMontage();

	/**
	* 复制移动时调用：处理模拟代理的旋转
	*/
	virtual void OnRep_ReplicatedMovement() override;
	/**
	* 角色被淘汰：处理角色淘汰逻辑
	* @param bPlayerLeftGame 玩家是否主动离开游戏
	*/
	void Elim(bool bPlayerLeftGame);
	/**
	* 多播角色被淘汰：在所有客户端上执行淘汰效果
	* @param bPlayerLeftGame 玩家是否主动离开游戏
	*/
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);
	/**
	* 角色被销毁时调用：清理资源
	*/
	virtual void Destroyed() override;

	/**
	* 是否禁用游戏玩法：用于淘汰状态或其他需要禁用角色控制的情况
	*/
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	/**
	* 显示狙击镜UI：蓝图实现事件，用于显示或隐藏狙击镜界面
	* @param bShowScope 是否显示狙击镜
	*/
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);

	/**
	* 更新HUD生命值：更新玩家界面上的生命值显示
	*/
	void UpdateHUDHealth();
	/**
	* 更新HUD护盾值：更新玩家界面上的护盾值显示
	*/
	void UpdateHUDShield();
	/**
	* 更新HUD弹药数：更新玩家界面上的弹药数量显示
	*/
	void UpdateHUDAmmo();

	/**
	* 生成默认武器：生成玩家的初始武器
	*/
	void SpawDefaultWeapon();

	/**
	* 击中碰撞盒映射：用于服务器端回放的碰撞检测
	*/
	UPROPERTY()
	TMap<FName, class UBoxComponent*> HitCollisionBoxes;

	/**
	* 是否完成武器切换：用于武器切换动画的同步
	*/
	bool bFinishedSwapping = false;

	/**
	* 服务器离开游戏：通知服务器玩家离开游戏
	*/
	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	/**
	* 离开游戏委托：当玩家离开游戏时触发
	*/
	FOnLeftGame OnLeftGame;

	/**
	* 多播获得领先：通知所有客户端该玩家获得了领先
	*/
	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();

	/**
	* 多播失去领先：通知所有客户端该玩家失去了领先
	*/
	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();

	/**
	* 设置队伍颜色：根据队伍设置角色的材质颜色
	* @param Team 队伍枚举值
	*/
	void SetTeamColor(ETeam Team);

protected:
	/**
	* 开始游戏时调用：初始化角色状态和绑定事件
	*/
	virtual void BeginPlay() override;

	/**
	* 向前移动：处理角色向前移动输入
	* @param Value 移动输入值
	*/
	void MoveForward(float Value);
	/**
	* 向右移动：处理角色向右移动输入
	* @param Value 移动输入值
	*/
	void MoveRight(float Value);
	/**
	* 转向：处理角色水平旋转输入
	* @param Value 旋转输入值
	*/
	void Turn(float Value);
	/**
	* 向上看：处理角色垂直旋转输入
	* @param Value 旋转输入值
	*/
	void LookUp(float Value);
	/**
	* 装备按钮按下：处理装备武器或切换武器的输入
	*/
	void EquipButtonPressed();
	/**
	* 蹲下按钮按下：处理蹲下/站立切换的输入
	*/
	void CrouchButtonPressed();
	/**
	* 换弹按钮按下：处理武器换弹的输入
	*/
	void ReloadButtonPressed();
	/**
	* 瞄准按钮按下：处理开始瞄准的输入
	*/
	void AimButtonPressed();
	/**
	* 瞄准按钮释放：处理结束瞄准的输入
	*/
	void AimButtonReleased();
	/**
	* 瞄准偏移：计算并更新角色的瞄准偏移值
	* @param DeltaTime 帧间隔时间
	*/
	void AimOffset(float DeltaTime);
	/**
	* 计算AO俯仰角：计算角色的俯仰角偏移
	*/
	void CalculateAO_Pitch();
	/**
	* 模拟代理转向：处理网络模拟代理的转向逻辑
	*/
	void SimProxiesTurn();
	/**
	* 跳跃：处理角色跳跃输入
	*/
	virtual void Jump() override;
	/**
	* 开火按钮按下：处理武器开火的输入
	*/
	void FireButtonPressed();
	/**
	* 开火按钮释放：处理停止开火的输入
	*/
	void FireButtonReleased();
	/**
	* 播放受击反应蒙太奇：播放角色被击中时的动画
	*/
	void PlayHitReactMontage();
	/**
	* 手榴弹按钮按下：处理投掷手榴弹的输入
	*/
	void GrenadeButtonPressed();
	/**
	* 丢弃或销毁武器：处理单个武器的丢弃或销毁
	* @param Weapon 要处理的武器
	*/
	void DropOrDestroyWeapon(AWeapon* Weapon);
	/**
	* 丢弃或销毁所有武器：处理角色携带的所有武器的丢弃或销毁
	*/
	void DropOrDestroyWeapons();
	/**
	* 设置出生点：根据队伍设置角色的出生位置
	*/
	void SetSpawnPoint();
	/**
	* 玩家状态初始化：当玩家状态初始化时调用
	*/
	void OnPlayerStateInitialized();

	/**
	* 接收伤害：处理角色受到伤害的逻辑
	* @param DamagedActor 受伤的Actor
	* @param Damage 伤害值
	* @param DamageType 伤害类型
	* @param InstigatorController 伤害发起者的控制器
	* @param DamageCauser 造成伤害的Actor
	*/
	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	/**
	* 轮询初始化：检查并初始化相关类和HUD
	*/
	void PollInit();
	/**
	* 原地旋转：处理角色原地旋转的逻辑
	* @param DeltaTime 帧间隔时间
	*/
	void RotateInPlace(float DeltaTime);

	/**
	* 用于服务器端回放的碰撞盒
	*/

	UPROPERTY(EditAnywhere)
	class UBoxComponent* head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* backpack;

	UPROPERTY(EditAnywhere)
	UBoxComponent* blanket;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;

private:
	/**
	* 相机臂组件：控制相机跟随角色的方式
	*/
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class USpringArmComponent* CameraBoom;

	/**
	* 跟随相机组件：玩家视角的相机
	*/
	UPROPERTY(VisibleAnywhere, Category = Camera)
	class UCameraComponent* FollowCamera;

	/**
	* 头顶部件组件：显示玩家头顶信息的UI部件
	*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

	/**
	* 重叠武器：角色当前可以拾取的武器
	*/
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	/**
	* 重叠武器复制时调用：更新武器的拾取UI显示
	* @param LastWeapon 上一个重叠的武器
	*/
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	/**
	* 角色组件
	*/

	/**
	* 战斗组件：处理武器使用、射击等战斗相关逻辑
	*/
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent* Combat;

	/**
	* 增益组件：处理角色的增益效果
	*/
	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;

	/**
	* 延迟补偿组件：处理网络延迟的补偿逻辑
	*/
	UPROPERTY(VisibleAnywhere)
	class ULagCompensationComponent* LagCompensation;

	/**
	* 服务器装备按钮按下：在服务器上处理装备按钮的输入
	*/
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();

	/**
	* 瞄准偏移相关变量
	*/
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	/**
	* 原地旋转状态：角色当前是否在原地旋转
	*/
	ETurningInPlace TurningInPlace;
	/**
	* 原地旋转处理：计算并更新角色的原地旋转状态
	* @param DeltaTime 帧间隔时间
	*/
	void TurnInPlace(float DeltaTime);

	/**
	* 动画蒙太奇
	*/

	/**
	* 射击动画蒙太奇：武器射击时播放的动画
	*/
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	/**
	* 换弹动画蒙太奇：武器换弹时播放的动画
	*/
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;

	/**
	* 受击反应动画蒙太奇：角色被击中时播放的动画
	*/
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	/**
	* 淘汰动画蒙太奇：角色被淘汰时播放的动画
	*/
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;

	/**
	* 投掷手榴弹动画蒙太奇：投掷手榴弹时播放的动画
	*/
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowGrenadeMontage;

	/**
	* 武器切换动画蒙太奇：切换武器时播放的动画
	*/
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* SwapMontage;

	/**
	* 相机太近时隐藏角色：当相机距离角色太近时隐藏角色模型
	*/
	void HideCameraIfCharacterClose();

	/**
	* 相机阈值：决定何时隐藏角色模型的距离阈值
	*/
	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;

	/**
	* 旋转根骨相关变量
	*/
	bool bRotateRootBone;
	float TurnThreshold = 0.5f;
	FRotator ProxyRotationLastFrame;
	FRotator ProxyRotation;
	float ProxyYaw;
	float TimeSinceLastMovementReplication;
	/**
	* 计算移动速度：计算角色的水平移动速度
	* @return 水平移动速度
	*/
	float CalculateSpeed();

	/**
	* 玩家生命值
	*/

	/**
	* 最大生命值：角色的最大生命值
	*/
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	/**
	* 当前生命值：角色的当前生命值
	*/
	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	/**
	* 生命值复制时调用：更新HUD显示和播放受击动画
	* @param LastHealth 上一个生命值
	*/
	UFUNCTION()
	void OnRep_Health(float LastHealth);

	/**
	* 玩家护盾
	*/

	/**
	* 最大护盾值：角色的最大护盾值
	*/
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;

	/**
	* 当前护盾值：角色的当前护盾值
	*/
	UPROPERTY(ReplicatedUsing = OnRep_Shield, EditAnywhere, Category = "Player Stats")
	float Shield = 0.f;

	/**
	* 护盾值复制时调用：更新HUD显示和播放受击动画
	* @param LastShield 上一个护盾值
	*/
	UFUNCTION()
	void OnRep_Shield(float LastShield);

	/**
	* 角色控制器引用
	*/
	UPROPERTY()
	class ABlasterPlayerController* BlasterPlayerController;

	/**
	* 是否被淘汰：角色当前是否处于淘汰状态
	*/
	bool bElimmed = false;

	/**
	* 淘汰计时器：控制淘汰后重生的时间
	*/
	FTimerHandle ElimTimer;

	/**
	* 淘汰延迟：被淘汰后到重生之间的延迟时间
	*/
	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	/**
	* 淘汰计时器结束：处理淘汰延迟结束后的逻辑
	*/
	void ElimTimerFinished();

	/**
	* 是否离开游戏：玩家是否主动离开游戏
	*/
	bool bLeftGame = false;

	/**
	* 溶解效果
	*/

	/**
	* 溶解时间线组件：控制溶解效果的动画进度
	*/
	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;

	/**
	* 溶解曲线：定义溶解效果的动画曲线
	*/
	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	/**
	* 更新溶解材质：更新溶解效果的材质参数
	* @param DissolveValue 溶解进度值
	*/
	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	/**
	* 开始溶解效果：启动角色的溶解动画
	*/
	void StartDissolve();

	/**
	* 动态溶解材质实例：可以在运行时修改的溶解材质实例
	*/
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	/**
	* 溶解材质实例：蓝图中设置的溶解材质实例，用于创建动态材质实例
	*/
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	/** 
	* 队伍颜色材质
	*/

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* RedDissolveMatInst;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* RedMaterial;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* BlueDissolveMatInst;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* BlueMaterial;

	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* OriginalMaterial;

	/**
	* 淘汰效果
	*/

	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* ElimBotSound;

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* CrownSystem;

	UPROPERTY()
	class UNiagaraComponent* CrownComponent;

	/**
	* 手榴弹相关组件
	*/

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

	/**
	* 默认武器
	*/

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWeapon> DefaultWeaponClass;

	UPROPERTY()
	class ABlasterGameMode* BlasterGameMode;

public:
	/**
	* 设置重叠武器：设置角色当前可以拾取的武器
	* @param Weapon 重叠的武器
	*/
	void SetOverlappingWeapon(AWeapon* Weapon);
	/**
	* 是否装备武器：检查角色是否装备了武器
	* @return 是否装备了武器
	*/
	bool IsWeaponEquipped();
	/**
	* 是否在瞄准：检查角色是否处于瞄准状态
	* @return 是否在瞄准
	*/
	bool IsAiming();
	/**
	* 获取AO偏航角：获取角色的水平瞄准偏移角
	* @return AO偏航角
	*/
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	/**
	* 获取AO俯仰角：获取角色的垂直瞄准偏移角
	* @return AO俯仰角
	*/
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	/**
	* 获取装备的武器：获取角色当前装备的武器
	* @return 装备的武器
	*/
	AWeapon* GetEquippedWeapon();
	/**
	* 获取原地旋转状态：获取角色当前的原地旋转状态
	* @return 原地旋转状态枚举
	*/
	FORCEINLINE ETurningInPlace GetTurningInPlace() const { return TurningInPlace; }
	/**
	* 获取命中目标：获取角色的瞄准命中点
	* @return 命中目标的位置
	*/
	FVector GetHitTarget() const;
	/**
	* 获取跟随相机：获取角色的跟随相机组件
	* @return 跟随相机组件
	*/
	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	/**
	* 是否应该旋转根骨：获取是否应该旋转角色的根骨骼
	* @return 是否应该旋转根骨
	*/
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }
	/**
	* 是否已被淘汰：获取角色是否已被淘汰
	* @return 是否已被淘汰
	*/
	FORCEINLINE bool IsElimmed() const { return bElimmed; }
	/**
	* 获取当前生命值：获取角色的当前生命值
	* @return 当前生命值
	*/
	FORCEINLINE float GetHealth() const { return Health; }
	/**
	* 设置当前生命值：设置角色的当前生命值
	* @param Amount 生命值数值
	*/
	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }
	/**
	* 获取最大生命值：获取角色的最大生命值
	* @return 最大生命值
	*/
	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	/**
	* 获取当前护盾值：获取角色的当前护盾值
	* @return 当前护盾值
	*/
	FORCEINLINE float GetShield() const { return Shield; }
	/**
	* 设置当前护盾值：设置角色的当前护盾值
	* @param Amount 护盾值数值
	*/
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	/**
	* 获取最大护盾值：获取角色的最大护盾值
	* @return 最大护盾值
	*/
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	/**
	* 获取战斗状态：获取角色当前的战斗状态
	* @return 战斗状态枚举
	*/
	ECombatState GetCombatState() const;
	/**
	* 获取战斗组件：获取角色的战斗组件
	* @return 战斗组件
	*/
	FORCEINLINE UCombatComponent* GetCombat() const { return Combat; }
	/**
	* 获取是否禁用游戏玩法：获取角色是否处于禁用游戏玩法状态
	* @return 是否禁用游戏玩法
	*/
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }
	/**
	* 获取换弹动画蒙太奇：获取角色的换弹动画蒙太奇
	* @return 换弹动画蒙太奇
	*/
	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }
	/**
	* 获取附加的手榴弹：获取角色附加的手榴弹组件
	* @return 附加的手榴弹组件
	*/
	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }
	/**
	* 获取增益组件：获取角色的增益组件
	* @return 增益组件
	*/
	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }
	/**
	* 是否在本地换弹：检查角色是否在本地进行换弹操作
	* @return 是否在本地换弹
	*/
	bool IsLocallyReloading();
	/**
	* 获取延迟补偿组件：获取角色的延迟补偿组件
	* @return 延迟补偿组件
	*/
	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensation; }
	/**
	* 是否持有旗帜：检查角色是否持有旗帜
	* @return 是否持有旗帜
	*/
	FORCEINLINE bool IsHoldingTheFlag() const;
	/**
	* 获取队伍：获取角色所属的队伍
	* @return 队伍枚举值
	*/
	ETeam GetTeam();
	/**
	* 设置是否持有旗帜：设置角色是否持有旗帜
	* @param bHolding 是否持有旗帜
	*/
	void SetHoldingTheFlag(bool bHolding);
};
