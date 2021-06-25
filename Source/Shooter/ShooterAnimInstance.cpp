// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

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
}

void UShooterAnimInstance::NativeInitializeAnimation()
{
    ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}