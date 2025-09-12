// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeaponTypes.h"
#include "Blaster/BlasterTypes/Team.h"
#include "Weapon.generated.h"

/**
 * 武器状态枚举
 * 定义武器可能处于的不同状态
 */
UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	//初始状态
	EWS_Initial UMETA(DisplayName = "Initial State"),
	//已装备
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	//已装备为次要武器
	EWS_EquippedSecondary UMETA(DisplayName = "Equipped Secondary"),
	//已丢弃
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	//默认最大值
	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

/**
 * 武器发射类型枚举
 * 定义武器的不同发射机制
 */
UENUM(BlueprintType)
enum class EFireType : uint8
{
	//射线武器
	EFT_HitScan UMETA(DisplayName = "Hit Scan Weapon"),
	//投射物武器
	EFT_Projectile UMETA(DisplayName = "Projectile Weapon"),
	//霰弹枪
	EFT_Shotgun UMETA(DisplayName = "Shotgun Weapon"),
	//默认最大值
	EFT_MAX UMETA(DisplayName = "DefaultMAX")
};

/**
 * 武器基类
 * 所有游戏内武器的基类，提供武器的通用功能和属性
 */
UCLASS()
class BLASTER_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:
	/**
	 * 构造函数
	 * 初始化武器的基本属性和组件
	 */
	AWeapon();

	/**
	 * 每帧更新函数
	 * @param DeltaTime - 帧间隔时间
	 */
	virtual void Tick(float DeltaTime) override;

	/**
	 * 网络复制属性设置
	 * 定义哪些属性需要在网络上同步
	 * @param OutLifetimeProps - 输出参数，用于存储需要复制的属性
	 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * 所有者变更时的网络回调
	 * 当武器的所有者通过网络变更时调用
	 */
	virtual void OnRep_Owner() override;

	/**
	 * 设置HUD上的弹药显示
	 * 更新玩家HUD上的弹药数量显示
	 */
	void SetHUDAmmo();

	/**
	 * 显示或隐藏拾取提示界面
	 * @param bShowWidget - 是否显示拾取界面
	 */
	void ShowPickupWidget(bool bShowWidget);

	/**
	 * 武器发射函数
	 * 基类实现，派生类可以重写以实现不同的发射逻辑
	 * @param HitTarget - 命中目标的位置
	 */
	virtual void Fire(const FVector& HitTarget);

	/**
	 * 丢弃武器函数
	 * 将武器从角色手中丢弃到地面
	 */
	virtual void Dropped();

	/**
	 * 添加弹药
	 * @param AmmoToAdd - 要添加的弹药数量
	 */
	void AddAmmo(int32 AmmoToAdd);

	/**
	 * 计算带散射的射线终点
	 * 用于模拟武器子弹的散布效果
	 * @param HitTarget - 原始命中目标位置
	 * @return 散射后的射线终点位置
	 */
	FVector TraceEndWithScatter(const FVector& HitTarget);
	
	/**
	* 武器准星纹理
	*/

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsLeft;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsRight;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsTop;

	UPROPERTY(EditAnywhere, Category = Crosshairs)
	UTexture2D* CrosshairsBottom;

	/** 
	* 瞄准状态下的视场角
	*/

	UPROPERTY(EditAnywhere)
	float ZoomedFOV = 30.f;

	/** 
	* 瞄准状态下的视场角插值速度
	*/
	UPROPERTY(EditAnywhere)
	float ZoomInterpSpeed = 20.f;

	/** 
	* 自动发射设置
	*/
	UPROPERTY(EditAnywhere, Category = Combat)
	float FireDelay = .15f;

	UPROPERTY(EditAnywhere, Category = Combat)
	bool bAutomatic = true;

	/** 
	* 装备武器时播放的音效
	*/
	UPROPERTY(EditAnywhere)
	class USoundCue* EquipSound;

	/** 
	* 启用或禁用自定义深度渲染
	* @param bEnable - 是否启用自定义深度
	*/
	void EnableCustomDepth(bool bEnable);

	/** 
	* 是否销毁武器的标志
	*/
	bool bDestroyWeapon = false;

	/** 
	* 武器发射类型
	*/
	UPROPERTY(EditAnywhere)
	EFireType FireType;

	/** 
	* 是否使用子弹散射效果
	*/
	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	bool bUseScatter = false;

protected:
	/**
	 * 开始游戏时的初始化
	 * 武器生成时调用一次
	 */
	virtual void BeginPlay() override;

	/**
	 * 武器状态变更时调用
	 * 根据新的武器状态执行相应的逻辑
	 */
	virtual void OnWeaponStateSet();

	/**
	 * 武器被装备时调用
	 * 设置武器为装备状态的逻辑
	 */
	virtual void OnEquipped();

	/**
	 * 武器被丢弃时调用
	 * 设置武器为丢弃状态的逻辑
	 */
	virtual void OnDropped();

	/**
	 * 武器被装备为次要武器时调用
	 * 设置武器为次要武器状态的逻辑
	 */
	virtual void OnEquippedSecondary();

	/**
	 * 区域球重叠开始时的回调
	 * 当有Actor进入武器的可拾取范围时触发
	 * @param OverlappedComponent - 重叠的组件
	 * @param OtherActor - 重叠的其他Actor
	 * @param OtherComp - 重叠的其他组件
	 * @param OtherBodyIndex - 重叠的身体索引
	 * @param bFromSweep - 是否来自扫描
	 * @param SweepResult - 扫描结果
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

	/**
	 * 区域球重叠结束时的回调
	 * 当Actor离开武器的可拾取范围时触发
	 * @param OverlappedComponent - 重叠的组件
	 * @param OtherActor - 重叠的其他Actor
	 * @param OtherComp - 重叠的其他组件
	 * @param OtherBodyIndex - 重叠的身体索引
	 */
	UFUNCTION()
	void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex
	);

	/**
	* 散射相关属性
	*/

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float DistanceToSphere = 800.f;

	UPROPERTY(EditAnywhere, Category = "Weapon Scatter")
	float SphereRadius = 75.f;

	/**
	* 武器伤害属性
	*/
	UPROPERTY(EditAnywhere)
	float Damage = 20.f;

	UPROPERTY(EditAnywhere)
	float HeadShotDamage = 40.f;

	/**
	* 是否使用服务器端倒带（网络延迟补偿）
	*/
	UPROPERTY(Replicated, EditAnywhere)
	bool bUseServerSideRewind = false;

	/**
	* 武器所有者相关引用
	*/
	UPROPERTY()
	class ABlasterCharacter* BlasterOwnerCharacter;
	UPROPERTY()
	class ABlasterPlayerController* BlasterOwnerController;

	/**
	 * 当网络延迟过高时的回调
	 * @param bPingTooHigh - 网络延迟是否过高
	 */
	UFUNCTION()
	void OnPingTooHigh(bool bPingTooHigh);
private:
	/**
	* 武器网格组件
	*/
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	USkeletalMeshComponent* WeaponMesh;

	/**
	* 区域球组件，用于检测可拾取范围
	*/
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent* AreaSphere;

	/**
	* 武器当前状态，通过网络复制并在变更时触发回调
	*/
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	/**
	 * 武器状态通过网络复制后的回调
	 * 当客户端收到武器状态变更时触发
	 */
	UFUNCTION()
	void OnRep_WeaponState();

	/**
	* 拾取提示界面组件
	*/
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class UWidgetComponent* PickupWidget;

	/**
	* 射击动画资产
	*/
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset* FireAnimation;

	/**
	* 弹壳类
	*/
	UPROPERTY(EditAnywhere)
	TSubclassOf<class ACasing> CasingClass;

	/**
	* 当前弹药数量
	*/
	UPROPERTY(EditAnywhere)
	int32 Ammo;

	/**
	 * 客户端更新弹药数量
	 * 从服务器向客户端同步弹药数量
	 * @param ServerAmmo - 服务器上的弹药数量
	 */
	UFUNCTION(Client, Reliable)
	void ClientUpdateAmmo(int32 ServerAmmo);

	/**
	 * 客户端添加弹药
	 * 从服务器向客户端同步添加弹药的操作
	 * @param AmmoToAdd - 要添加的弹药数量
	 */
	UFUNCTION(Client, Reliable)
	void ClientAddAmmo(int32 AmmoToAdd);

	/**
	 * 消耗一发弹药
	 * 射击时减少弹药数量并更新UI
	 */
	void SpendRound();

	/**
	* 弹夹容量
	*/
	UPROPERTY(EditAnywhere)
	int32 MagCapacity;

	/**
	* 未处理的服务器弹药请求数量
	* 在SpendRound中递增，在ClientUpdateAmmo中递减
	*/
	int32 Sequence = 0;

	/**
	* 武器类型
	*/
	UPROPERTY(EditAnywhere)
	EWeaponType WeaponType;

	/**
	* 武器所属队伍
	*/
	UPROPERTY(EditAnywhere)
	ETeam Team;

public:
	/**
	 * 设置武器状态
	 * @param State - 新的武器状态
	 */
	void SetWeaponState(EWeaponState State);

	// 快捷访问函数
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
	FORCEINLINE float GetZoomedFOV() const { return ZoomedFOV; }
	FORCEINLINE float GetZoomInterpSpeed() const { return ZoomInterpSpeed; }

	/**
	 * 检查武器是否为空（无弹药）
	 * @return 武器是否为空
	 */
	bool IsEmpty();

	/**
	 * 检查武器弹药是否已满
	 * @return 弹药是否已满
	 */
	bool IsFull();

	// 快捷访问函数
	FORCEINLINE EWeaponType GetWeaponType() const { return WeaponType; }
	FORCEINLINE int32 GetAmmo() const { return Ammo; }
	FORCEINLINE int32 GetMagCapacity() const { return MagCapacity; }
	FORCEINLINE float GetDamage() const { return Damage; }
	FORCEINLINE float GetHeadShotDamage() const { return HeadShotDamage; }
	FORCEINLINE ETeam GetTeam() const { return Team; }
};
