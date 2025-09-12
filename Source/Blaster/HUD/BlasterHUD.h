// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "BlasterHUD.generated.h"

/**
 * HUD包结构体：包含准星相关的纹理和参数配置
 */
USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()
public:
	/** 中心准星纹理 */
	class UTexture2D* CrosshairsCenter;
	/** 左侧准星纹理 */
	UTexture2D* CrosshairsLeft;
	/** 右侧准星纹理 */
	UTexture2D* CrosshairsRight;
	/** 上方准星纹理 */
	UTexture2D* CrosshairsTop;
	/** 下方准星纹理 */
	UTexture2D* CrosshairsBottom;
	/** 准星扩散程度 */
	float CrosshairSpread;
	/** 准星颜色 */
	FLinearColor CrosshairsColor;
};

/**
 * 游戏HUD类：负责绘制游戏界面元素，如准星、角色状态、公告信息等
 */
UCLASS()
class BLASTER_API ABlasterHUD : public AHUD
{
	GENERATED_BODY()
public:
	/** 重写父类的DrawHUD函数，用于绘制游戏界面元素 */
	virtual void DrawHUD() override;

	/** 角色状态叠加层类，用于显示玩家的生命值、弹药等信息 */
	UPROPERTY(EditAnywhere, Category = "Player Stats")
	TSubclassOf<class UUserWidget> CharacterOverlayClass;
	/** 添加角色状态叠加层到游戏界面 */
	void AddCharacterOverlay();

	/** 角色状态叠加层实例 */
	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	/** 公告UI类，用于显示游戏公告信息 */
	UPROPERTY(EditAnywhere, Category = "Announcements")
	TSubclassOf<UUserWidget> AnnouncementClass;

	/** 公告UI实例 */
	UPROPERTY()
	class UAnnouncement* Announcement;

	/** 添加公告UI到游戏界面 */
	void AddAnnouncement();
	/** 添加击杀公告信息 */
	void AddElimAnnouncement(FString Attacker, FString Victim);
protected:
	/** 重写父类的BeginPlay函数，在游戏开始时执行初始化操作 */
	virtual void BeginPlay() override;

private:

	/** 拥有该HUD的玩家控制器 */
	UPROPERTY()
	class APlayerController* OwningPlayer;

	/** HUD包数据，包含准星相关信息 */
	FHUDPackage HUDPackage;

	/** 绘制单个准星元素 */
	void DrawCrosshair(UTexture2D* Texture, FVector2D ViewportCenter, FVector2D Spread, FLinearColor CrosshairColor);

	/** 准星最大扩散距离 */
	UPROPERTY(EditAnywhere)
	float CrosshairSpreadMax = 16.f;

	/** 击杀公告UI类 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UElimAnnouncement> ElimAnnouncementClass;

	/** 击杀公告显示时间（秒） */
	UPROPERTY(EditAnywhere)
	float ElimAnnouncementTime = 2.5f;

	/** 击杀公告计时器完成回调函数 */
	UFUNCTION()
	void ElimAnnouncementTimerFinished(UElimAnnouncement* MsgToRemove);

	/** 存储当前显示的所有击杀公告 */
	UPROPERTY()
	TArray<UElimAnnouncement*> ElimMessages;
public:
	/** 设置HUD包数据 */
	FORCEINLINE void SetHUDPackage(const FHUDPackage& Package) { HUDPackage = Package; }
};
