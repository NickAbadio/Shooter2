// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Engine/SkeletalMeshSocket.h"
#include "DrawDebugHelpers.h"
#include "Particles/ParticleSystemComponent.h"

// Sets default values
AShooterCharacter::AShooterCharacter() :
	BaseTurnRate(45.f),
	BaseLookUpRate(45.f),
	bIsAiming(false),
	CameraDefaultFOV(0.f),
	CameraZoomedFOV(40.f),
	CameraCurrentFOV(0.f),
	ZoomInterpSpeed(1.f),
	HipTurnRate(90.f),
	HipLookUpRate(90.f),
	AimingTurnRate(20.f),
	AimingLookUpRate(20.f),
	MouseHipTurnRate(1.f),
	MouseHipLookUpRate(1.f),
	MouseAimingTurnRate(.3f),
	MouseAimingLookUpRate(.3f),
	CrosshairSpreadMultiplyer(0.f),
	CrosshairVelocityFactor(0.f),
	CrosshairShootingFactor(0.f),
	CrosshairInAirFactor(0.f),
	CrosshairAimFactor(0.f),
	ShootTimeDuration(0.05f),
	bFiringBullet(false),
	AutomaticFireRate(0.1f),
	bShouldFire(true),
	bFireButtonPressed(false)
{
	
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//Create a Camera boom (Pulls in towards character if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 180.f; //Camera follows at this distance
	CameraBoom->bUsePawnControlRotation = true; //Rotate the arm based on controller
	CameraBoom->SocketOffset = FVector(0.f, 70.f,70.f);

	//Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); //Attach camera to end of boom
	FollowCamera->bUsePawnControlRotation = false; //Does not rotate relative to arm
	//Controler should only affect the camera 
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;
	

	//Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = false; //Character moves in the direction of the input
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // At this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 10.f;
	
}

// Called when the game starts or when spawned
void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if(FollowCamera)
	{
		CameraDefaultFOV = GetFollowCamera()->FieldOfView;
		CameraCurrentFOV = CameraDefaultFOV;
	}



}

// Called every frame
void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	ChangeCameraFOV(DeltaTime);
	ChangeTurnRate();
	CalculatedCrosshairSpread(DeltaTime);

}

void AShooterCharacter::MoveForward(float Value)
{
	if((Controller != nullptr) && (Value != 0.0f))
	{
		//Find out which way is forward
		const FRotator Rotation{ Controller->GetControlRotation()};
		const FRotator YawRotation{ 0, Rotation.Yaw, 0};
		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::X) };
		AddMovementInput(Direction, Value);
	}
}
void AShooterCharacter::MoveRight(float Value)
{
	if((Controller != nullptr) && (Value != 0.0f))
	{	
		// Find out which way is right
		const FRotator Rotation{ Controller->GetControlRotation()};
		const FRotator YawRotation{ 0, Rotation.Yaw, 0};
		const FVector Direction{ FRotationMatrix{YawRotation}.GetUnitAxis(EAxis::Y) };
		AddMovementInput(Direction, Value);
	}
	
}

void AShooterCharacter::FireButtonPressed()
{
 bFireButtonPressed = true;
 StartFireTimer();
 
}
void AShooterCharacter::FireButtonReleased()
{
 bFireButtonPressed = false;
 
}

void AShooterCharacter::StartFireTimer()
{
	if(bShouldFire)
	{
		FireWeapon();
		bShouldFire = false;
		GetWorldTimerManager().SetTimer(AutoFireTimer, this, &AShooterCharacter::AutoFireReset, AutomaticFireRate);
	}

}


void AShooterCharacter::AutoFireReset()
{
bShouldFire = true;
if(bFireButtonPressed)
{
	StartFireTimer();
}
}

void AShooterCharacter::FireWeapon()
{
	if(FireSound)
	{
		UGameplayStatics::PlaySound2D(this, FireSound);
	}
	const USkeletalMeshSocket* BarrelSocket = GetMesh()->GetSocketByName("BarrelSocket");
	if (BarrelSocket)
	{
		const FTransform SocketTransform = BarrelSocket->GetSocketTransform(GetMesh());
		if (MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
		}

		FVector BeamEnd;
		bool bBeamEnd = GetBeamEndLocation(SocketTransform.GetLocation(), BeamEnd);
		if(bBeamEnd)
		{
			if(ImpactParticle)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticle, BeamEnd);
			}

			UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticle, SocketTransform);

			if(Beam)
			{
				Beam->SetVectorParameter(FName("Target"), BeamEnd);
			}
		}

	
}	
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if(AnimInstance && HipFireMontage)
	{
		AnimInstance->Montage_Play(HipFireMontage);
		AnimInstance->Montage_JumpToSection(FName("StartFire"));
	}

	StartCrosshairBulletFire();

}

bool AShooterCharacter::GetBeamEndLocation(const FVector& MuzzleSocketLocation, FVector& OutBeamLocation)
{
	FVector2D ViewportSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	FVector2D CrosshairLocation(ViewportSize.X/2.f, ViewportSize.Y/2.f);
	//CrosshairLocation.Y -= 50.f;
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;

	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
			UGameplayStatics::GetPlayerController(this,0), 
			CrosshairLocation,
			CrosshairWorldPosition,
			CrosshairWorldDirection);

	if(bScreenToWorld)
		{
			FHitResult ScreenTraceHit;
			const FVector Start{CrosshairWorldPosition};
			const FVector End{CrosshairWorldPosition + CrosshairWorldDirection * 50'000.f};

			OutBeamLocation = End;
			GetWorld()->LineTraceSingleByChannel(ScreenTraceHit, Start, End,
				ECollisionChannel::ECC_Visibility);

			if(ScreenTraceHit.bBlockingHit)
			{
				OutBeamLocation = ScreenTraceHit.Location;
				
			}

			FHitResult WeaponTraceHit;
			const FVector WeaponTraceStart{MuzzleSocketLocation};
			const FVector WeaponTraceEnd{ OutBeamLocation};
			GetWorld()->LineTraceSingleByChannel(
				WeaponTraceHit,
				WeaponTraceStart,
				WeaponTraceEnd,
				ECollisionChannel::ECC_Visibility);
			
			if(WeaponTraceHit.bBlockingHit)
			{
				OutBeamLocation = WeaponTraceHit.Location;
			}
			return true;
		}
	return false;
}


void AShooterCharacter::ChangeCameraFOV(float DeltaTime)
{
	if(bIsAiming)
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraZoomedFOV, DeltaTime, ZoomInterpSpeed);

	}
	else
	{
		CameraCurrentFOV = FMath::FInterpTo(CameraCurrentFOV, CameraDefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	GetFollowCamera()->SetFieldOfView(CameraCurrentFOV);
}

void AShooterCharacter::AimingButtonPressed()
{
 bIsAiming = true;
 
 if(FollowCamera)
 {

 }
}
void AShooterCharacter::AimingButtonReleased()
{
 bIsAiming = false;
 
}
void AShooterCharacter::ChangeTurnRate()
{
 
if (bIsAiming)
	{
		BaseTurnRate = AimingTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}
	else
	{
		BaseTurnRate = HipTurnRate;
		BaseLookUpRate = AimingLookUpRate;
	}

}

void AShooterCharacter::TurnAtRate(float Rate)
{
	//Calculate Delta for base turn rate
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds()); 
}

void AShooterCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AShooterCharacter::Turn(float Value)
{
	float TurnScaleFactor{};
	if(bIsAiming)
	{
		TurnScaleFactor = MouseAimingTurnRate;
	}
	else
	{
		TurnScaleFactor = MouseHipTurnRate;
	}
	AddControllerYawInput(Value * TurnScaleFactor);
}	
void AShooterCharacter::LookUp(float Value)
{
float TurnScaleFactor{};
	if(bIsAiming)
	{
		TurnScaleFactor = MouseAimingLookUpRate;
	}
	else
	{
		TurnScaleFactor = MouseHipLookUpRate;
	}
	AddControllerPitchInput(Value * TurnScaleFactor);
}


void AShooterCharacter::StartCrosshairBulletFire()
{
	bFiringBullet = true;
	GetWorldTimerManager().SetTimer(CrosshairShootTimer, this, &AShooterCharacter::FinishCrosshairBulletFire, ShootTimeDuration);
}

void AShooterCharacter::FinishCrosshairBulletFire()
{
	bFiringBullet = false;
}

float AShooterCharacter::GetCrosshairSpreadMultplier() const
{
	return CrosshairSpreadMultiplyer;
}
void AShooterCharacter::CalculatedCrosshairSpread(float DeltaTime)
{
	FVector2D WalkSpeedRange{0.f, 600.f};
	FVector2D VelocityMultiplayerRange{0.f, 1.f};
	FVector Velocity{ GetVelocity() };
	Velocity.Z = 0.f;

	if(GetCharacterMovement()->IsFalling())
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25);
	}
	else
	{
		CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
	}

	if(bIsAiming)
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, .5f, DeltaTime, 15);
	}
	else
	{
		CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 1.f, DeltaTime, 15);
	}

	if(bFiringBullet)
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, .3f, DeltaTime, 60.f);
	}
	else
	{
		CrosshairShootingFactor = FMath::FInterpTo(CrosshairShootingFactor, 0.f, DeltaTime, 60.f);
	}

	CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplayerRange, Velocity.Size());
	CrosshairSpreadMultiplyer = CrosshairAimFactor*(0.5f + CrosshairVelocityFactor + CrosshairInAirFactor + CrosshairShootingFactor); 
}

// Called to bind functionality to input
void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis("TurnRate", this, &AShooterCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AShooterCharacter::LookUpAtRate);
	PlayerInputComponent->BindAxis("Turn", this, &AShooterCharacter::Turn);
	PlayerInputComponent->BindAxis("LookUp", this, &AShooterCharacter::LookUp);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("FireButton", IE_Pressed, this, &AShooterCharacter::FireButtonPressed);
	PlayerInputComponent->BindAction("FireButton", IE_Released, this, &AShooterCharacter::FireButtonReleased);
	PlayerInputComponent->BindAction("AimButton", IE_Pressed, this, &AShooterCharacter::AimingButtonPressed);
	PlayerInputComponent->BindAction("AimButton", IE_Released, this, &AShooterCharacter::AimingButtonReleased);
	
}