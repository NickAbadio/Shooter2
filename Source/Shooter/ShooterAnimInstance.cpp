// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UShooterAnimInstance::UShooterAnimInstance() :
    Speed(0.f),
    bIsAccelerating(false),
    bIsInAir(false),
    bIsAiming(false),
    CharacterYawLastFrame(0.f),
    MovementOffset(0.f),
    LastMovementOffsetYaw(0.f),
    CharacterYaw(0.f),
    RootYawOffset(0.f)
{

}
void UShooterAnimInstance::UpdateAnimationProperties(float DeltaTime)
{
    if(ShooterCharacter == nullptr)
    {
    ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
    }
    if(ShooterCharacter)
    {
        // Get lateral speed of the character from velocity
        FVector Velocity{ShooterCharacter->GetVelocity()};
        Velocity.Z = 0;
        Speed = Velocity.Size();

        //Is the character in the air?
        bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();

        //Is the character accelerating?
        if(ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f)
        {
            bIsAccelerating = true;

        }
        else
        {
            bIsAccelerating = false;
        }

        FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
        FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
        MovementOffset = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation).Yaw;
        //FString RotationMessage = FString::Printf(TEXT("Base Aim Rotation: %f"), AimRotation.Yaw);
        //FString MovementMessage = FString::Printf(TEXT("Movement Rotation: %f"), MovementRotation.Yaw);
        FString MovementMessage = FString::Printf(TEXT("Movement Offset: %f"), MovementOffset);

        if(ShooterCharacter->GetVelocity().Size() > 0.f)
        {
        LastMovementOffsetYaw = MovementOffset;
        }

        if(GEngine)
        {
            //GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, RotationMessage);
            GEngine->AddOnScreenDebugMessage(1, 0.f, FColor::White, MovementMessage);
        }

        bIsAiming = ShooterCharacter->GetAiming();
        
    }
    TurnInPlace();
    
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
    ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::TurnInPlace()
{
    if (ShooterCharacter == nullptr) return;
    Pitch = ShooterCharacter->GetBaseAimRotation().Pitch;
    if (Speed > 0)
    {
        RootYawOffset = 0.f;
        CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
        CharacterYawLastFrame = CharacterYaw;
        RotationCurveLastFrame = 0.f;
        RotationCurve = 0.f;

    }
    else
    {
        CharacterYawLastFrame = CharacterYaw;
        CharacterYaw = ShooterCharacter->GetActorRotation().Yaw;
        const float YawDelta { CharacterYaw - CharacterYawLastFrame};

        // Root Yaw Offset, Updated and clamped to [-180, 180]
        RootYawOffset = UKismetMathLibrary::NormalizeAxis(RootYawOffset - YawDelta);

        const float Turning{ GetCurveValue(TEXT("Turning")) };
        if (Turning > 0)
        {
            RotationCurveLastFrame = RotationCurve;
            RotationCurve = GetCurveValue(TEXT("Rotation"));
            const float DeltaRotation{ RotationCurve - RotationCurveLastFrame};

            if(RootYawOffset>0) //Turning left
            {
                RootYawOffset -= DeltaRotation;

            }
            else //Turning Right
            {
                RootYawOffset += DeltaRotation;
            }

            const float ABSRootYawOffset{FMath::Abs(RootYawOffset)};
            if (ABSRootYawOffset > 90.f)
            {
                const float YawExcess { ABSRootYawOffset - 90.f};
                RootYawOffset > 0 ? RootYawOffset -= YawExcess : RootYawOffset += YawExcess;
            }


        }

       
       
    }
}