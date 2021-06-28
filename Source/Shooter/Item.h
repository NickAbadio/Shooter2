// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UENUM(BlueprintType)
enum class EItemRarity : uint8
{
	EIR_Damaged UMETA(DisplayName = "Damaged"),
	EIR_Common UMETA(DisplayName = "Common"),
	EIR_Uncommon UMETA(DisplayName = "Uncommon"),
	EIR_Rare UMETA(DisplayName = "Rare"),
	EIR_Legendary UMETA(DisplayName = "Legendary"),

	EIR_MAX UMETA(DisplayName = "DefaultMAX"),
};

UCLASS()
class SHOOTER_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnSphereOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex, 
	bool bFromSweep, 
	const FHitResult& SweepResult);


	UFUNCTION()
	void OnSphereEndOverlap(
	UPrimitiveComponent* OverlappedComponent, 
	AActor* OtherActor, 
	UPrimitiveComponent* OtherComp, 
	int32 OtherBodyIndex);

	void SetActiveStars();
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	//basic mesh for items.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* ItemMesh;

	//Basic colldier component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	class UBoxComponent* CollisionBox;

	//enables item tracing when overlap
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	class USphereComponent* AreaSphere;

	//Pop Up widget for when player looks at item
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	FString ItemName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	int32 ItemCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	EItemRarity ItemRarity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Item Property", meta = (AllowPrivateAccess = "true"))
	TArray<bool> ActiveStars;
	



public:
	FORCEINLINE UWidgetComponent* GetPickupWidget() const { return PickupWidget; }
};
