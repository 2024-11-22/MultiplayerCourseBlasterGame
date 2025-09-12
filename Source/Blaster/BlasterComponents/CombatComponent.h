// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Blaster/HUD/BlasterHUD.h"
#include "Blaster/Weapon/WeaponTypes.h"
#include "Blaster/BlasterTypes/CombatState.h"
#include "CombatComponent.generated.h"

/**
 * 战斗组件 - 处理角色的所有战斗相关功能，如武器装备、射击、换弹、手榴弹投掷等
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BLASTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	/** 构造函数 */
	UCombatComponent();
	friend class ABlasterCharacter;
	/** 组件每帧更新函数 */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	/** 复制属性设置函数 */
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** 装备武器 */
	void EquipWeapon(class AWeapon* WeaponToEquip);
	/** 交换主副武器 */
	void SwapWeapons();
	/** 开始重新装弹 */
	void Reload();
	/** 完成重新装弹（蓝图可调用） */
	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	/** 完成武器交换（蓝图可调用） */
	UFUNCTION(BlueprintCallable)
	void FinishSwap();

	/** 完成武器交换并附加武器（蓝图可调用） */
	UFUNCTION(BlueprintCallable)
	void FinishSwapAttachWeapons();

	/** 处理开火按钮按下状态 */
	void FireButtonPressed(bool bPressed);

	/** 霰弹枪单发射壳装弹（蓝图可调用） */
	UFUNCTION(BlueprintCallable)
	void ShotgunShellReload();

	/** 跳转到霰弹枪装弹动画结尾 */
	void JumpToShotgunEnd();

	/** 完成手榴弹投掷（蓝图可调用） */
	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	/** 发射手榴弹（蓝图可调用） */
	UFUNCTION(BlueprintCallable)
	void LaunchGrenade();

	/** 服务器端发射手榴弹（可靠RPC） */
	UFUNCTION(Server, Reliable)
	void ServerLaunchGrenade(const FVector_NetQuantize& Target);

	/** 拾取弹药 */
	void PickupAmmo(EWeaponType WeaponType, int32 AmmoAmount);
	/** 本地是否正在装弹 */
	bool bLocallyReloading = false;
protected:
	/** 组件初始化函数 */
	virtual void BeginPlay() override;
	/** 设置瞄准状态 */
	void SetAiming(bool bIsAiming);

	/** 服务器端设置瞄准状态（可靠RPC） */
	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	/** 当装备武器复制到客户端时调用 */
	UFUNCTION()
	void OnRep_EquippedWeapon();

	/** 当副武器复制到客户端时调用 */
	UFUNCTION()
	void OnRep_SecondaryWeapon();

	/** 开火主函数 */
	void Fire();
	/** 发射投射物武器 */
	void FireProjectileWeapon();
	/** 发射 hitscan 类型武器 */
	void FireHitScanWeapon();
	/** 发射霰弹枪 */
	void FireShotgun();
	/** 在本地播放开火效果 */
	void LocalFire(const FVector_NetQuantize& TraceHitTarget);
	/** 在本地播放霰弹枪开火效果 */
	void ShotgunLocalFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	/** 服务器端处理开火（可靠RPC，带验证） */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerFire(const FVector_NetQuantize& TraceHitTarget, float FireDelay);

	/** 多播处理开火效果（可靠RPC） */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	/** 服务器端处理霰弹枪开火（可靠RPC，带验证） */
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets, float FireDelay);

	/** 多播处理霰弹枪开火效果（可靠RPC） */
	UFUNCTION(NetMulticast, Reliable)
	void MulticastShotgunFire(const TArray<FVector_NetQuantize>& TraceHitTargets);

	/** 在准星下进行射线检测 */
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	/** 设置HUD准星 */
	void SetHUDCrosshairs(float DeltaTime);

	/** 服务器端处理重新装弹（可靠RPC） */
	UFUNCTION(Server, Reliable)
	void ServerReload();

	/** 处理重新装弹逻辑 */
	void HandleReload();
	/** 计算需要装弹的数量 */
	int32 AmountToReload();

	/** 投掷手榴弹 */
	void ThrowGrenade();

	/** 服务器端处理投掷手榴弹（可靠RPC） */
	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	/** 手榴弹类 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;

	/** 丢弃当前装备的武器 */
	void DropEquippedWeapon();
	/** 将Actor附加到右手 */
	void AttachActorToRightHand(AActor* ActorToAttach);
	/** 将Actor附加到左手 */
	void AttachActorToLeftHand(AActor* ActorToAttach);
	/** 将旗帜附加到左手 */
	void AttachFlagToLeftHand(AWeapon* Flag);
	/** 将Actor附加到背包 */
	void AttachActorToBackpack(AActor* ActorToAttach);
	/** 更新携带的弹药数量 */
	void UpdateCarriedAmmo();
	/** 播放装备武器的声音 */
	void PlayEquipWeaponSound(AWeapon* WeaponToEquip);
	/** 为空武器装弹 */
	void ReloadEmptyWeapon();
	/** 显示或隐藏附加的手榴弹 */
	void ShowAttachedGrenade(bool bShowGrenade);
	/** 装备主武器 */
	void EquipPrimaryWeapon(AWeapon* WeaponToEquip);
	/** 装备副武器 */
	void EquipSecondaryWeapon(AWeapon* WeaponToEquip);
private:
	/** 拥有此组件的角色 */
	UPROPERTY()
	class ABlasterCharacter* Character;
	/** 角色的控制器 */
	UPROPERTY()
	class ABlasterPlayerController* Controller;
	/** HUD引用 */
	UPROPERTY()
	class ABlasterHUD* HUD;

	/** 当前装备的武器（带复制通知） */
	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	/** 副武器（带复制通知） */
	UPROPERTY(ReplicatedUsing = OnRep_SecondaryWeapon)
	AWeapon* SecondaryWeapon;

	/** 是否正在瞄准（带复制通知） */
	UPROPERTY(ReplicatedUsing = OnRep_Aiming)
	bool bAiming = false;

	/** 瞄准按钮是否被按下 */
	bool bAimButtonPressed = false;

	/** 当瞄准状态复制到客户端时调用 */
	UFUNCTION()
	void OnRep_Aiming();

	/** 基础移动速度 */
	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	/** 瞄准状态下的移动速度 */
	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	/** 开火按钮是否被按下 */
	bool bFireButtonPressed;

	/** 
	* HUD 和准星相关
	*/

	/** 准星随移动速度变化的因子 */
	float CrosshairVelocityFactor;
	/** 准星在空中的扩散因子 */
	float CrosshairInAirFactor;
	/** 准星在瞄准状态下的收缩因子 */
	float CrosshairAimFactor;
	/** 准星在射击时的扩散因子 */
	float CrosshairShootingFactor;

	/** 射线击中的目标位置 */
	FVector HitTarget;

	/** HUD显示包，包含准星等信息 */
	FHUDPackage HUDPackage;

	/** 
	* 瞄准和视野相关
	*/

	/** 未瞄准状态下的视野；在BeginPlay中设置为相机的基础FOV */
	float DefaultFOV;

	/** 瞄准状态下的视野（可编辑） */
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomedFOV = 30.f;

	/** 当前视野 */
	float CurrentFOV;

	/** 视野插值速度（可编辑） */
	UPROPERTY(EditAnywhere, Category = Combat)
	float ZoomInterpSpeed = 20.f;

	/** 视野插值函数 */
	void InterpFOV(float DeltaTime);

	/**
	* 自动开火相关
	*/

	/** 开火计时器句柄 */
	FTimerHandle FireTimer;
	/** 是否可以开火 */
	bool bCanFire = true;

	/** 开始开火计时器 */
	void StartFireTimer();
	/** 开火计时器结束回调 */
	void FireTimerFinished();

	/** 检查是否可以开火 */
	bool CanFire();

	/** 当前装备武器的携带弹药量（带复制通知） */
	UPROPERTY(ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	/** 当携带弹药量复制到客户端时调用 */
	UFUNCTION()
	void OnRep_CarriedAmmo();

	/** 按武器类型存储的携带弹药量映射 */
	TMap<EWeaponType, int32> CarriedAmmoMap;

	/** 最大可携带弹药量（可编辑） */
	UPROPERTY(EditAnywhere)
	int32 MaxCarriedAmmo = 500;

	/** 初始突击步枪弹药量（可编辑） */
	UPROPERTY(EditAnywhere)
	int32 StartingARAmmo = 30;

	/** 初始火箭弹药量（可编辑） */
	UPROPERTY(EditAnywhere)
	int32 StartingRocketAmmo = 0;

	/** 初始手枪弹药量（可编辑） */
	UPROPERTY(EditAnywhere)
	int32 StartingPistolAmmo = 0;

	/** 初始冲锋枪弹药量（可编辑） */
	UPROPERTY(EditAnywhere)
	int32 StartingSMGAmmo = 0;

	/** 初始霰弹枪弹药量（可编辑） */
	UPROPERTY(EditAnywhere)
	int32 StartingShotgunAmmo = 0;

	/** 初始狙击枪弹药量（可编辑） */
	UPROPERTY(EditAnywhere)
	int32 StartingSniperAmmo = 0;

	/** 初始榴弹发射器弹药量（可编辑） */
	UPROPERTY(EditAnywhere)
	int32 StartingGrenadeLauncherAmmo = 0;

	/** 初始化携带弹药量 */
	void InitializeCarriedAmmo();

	/** 战斗状态（带复制通知） */
	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	/** 当战斗状态复制到客户端时调用 */
	UFUNCTION()
	void OnRep_CombatState();

	/** 更新弹药显示值 */
	void UpdateAmmoValues();
	/** 更新霰弹枪弹药显示值 */
	void UpdateShotgunAmmoValues();

	/** 当前手榴弹数量（带复制通知） */
	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 4;

	/** 当手榴弹数量复制到客户端时调用 */
	UFUNCTION()
	void OnRep_Grenades();

	/** 最大手榴弹数量（可编辑） */
	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 4;

	/** 更新HUD上的手榴弹显示 */
	void UpdateHUDGrenades();

	/** 是否持有旗帜（带复制通知） */
	UPROPERTY(ReplicatedUsing = OnRep_HoldingTheFlag)
	bool bHoldingTheFlag = false;

	/** 当持有旗帜状态复制到客户端时调用 */
	UFUNCTION()
	void OnRep_HoldingTheFlag();

	/** 旗帜引用 */
	UPROPERTY()
	AWeapon* TheFlag;

public:	
	/** 获取当前手榴弹数量 */
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	/** 检查是否应该交换武器 */
	bool ShouldSwapWeapons();
};
