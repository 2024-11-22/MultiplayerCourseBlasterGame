// Fill out your copyright notice in the Description page of Project Settings.


#include "JumpPickup.h"
#include "Blaster/Character/BlasterCharacter.h"
#include "Blaster/BlasterComponents/BuffComponent.h"

/**
 * 当玩家接触到跳跃增益拾取物时触发的重叠事件处理函数
 * 处理跳跃增益逻辑，通过Buff组件为玩家提供跳跃能力增强
 * @param OverlappedComponent 重叠的组件（通常是碰撞球体）
 * @param OtherActor 与拾取物重叠的其他Actor（通常是玩家角色）
 * @param OtherComp 其他Actor上的重叠组件
 * @param OtherBodyIndex 重叠组件的BodyIndex
 * @param bFromSweep 是否通过扫描触发
 * @param SweepResult 扫描结果信息
 */
void AJumpPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 调用父类的重叠事件处理（处理特效、音效等通用逻辑）
	Super::OnSphereOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	// 尝试将重叠的Actor转换为玩家角色
	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(OtherActor);
	if (BlasterCharacter)
	{
		// 获取玩家角色的Buff组件
		UBuffComponent* Buff = BlasterCharacter->GetBuff();
		if (Buff)
		{
			// 通过Buff组件为玩家提供跳跃能力增强
			Buff->BuffJump(JumpZVelocityBuff, JumpBuffTime);
		}
	}

	// 拾取完成后销毁跳跃增益拾取物
	Destroy();
}