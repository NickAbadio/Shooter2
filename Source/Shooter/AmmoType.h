
#pragma once

UENUM(BlueprintType)
enum class EAmmoType : uint8
{
	EAT_9mm UMETA(DisplayName = "9mm"),
	EAT_556mm UMETA(DisplayName = "556mm"),
	EAT_MAX UMETA(DisplayName = "DefaultMax")
};