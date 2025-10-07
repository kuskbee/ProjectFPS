// ShieldBarrierActor.cpp

#include "ShieldBarrierActor.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Weapons/FPSProjectile.h"
#include "GameFramework/Character.h"

AShieldBarrierActor::AShieldBarrierActor()
{
	PrimaryActorTick.bCanEverTick = true;

	// Root Component (Collision Sphere)
	CollisionSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionSphere"));
	CollisionSphere->SetSphereRadius(BarrierRadius);
	CollisionSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap); // 발사체 감지
	CollisionSphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap); // 발사체 감지
	RootComponent = CollisionSphere;

	// Barrier Mesh (반투명 구체)
	BarrierMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BarrierMesh"));
	BarrierMesh->SetupAttachment(RootComponent);
	BarrierMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 메시는 충돌 없음
	BarrierMesh->SetCastShadow(false);

	// 구체 메시 기본 설정 (Blueprint에서 교체 가능)
	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMeshAsset(TEXT("/Engine/BasicShapes/Sphere"));
	if (SphereMeshAsset.Succeeded())
	{
		BarrierMesh->SetStaticMesh(SphereMeshAsset.Object);
		BarrierMesh->SetRelativeScale3D(FVector(3.0f, 3.0f, 3.0f)); // 반지름 150cm
	}

	// Replication 설정
	bReplicates = true;
	SetReplicateMovement(true);
}

void AShieldBarrierActor::BeginPlay()
{
	Super::BeginPlay();

	// HP 초기화
	CurrentHealth = MaxHealth;

	// 충돌 이벤트 바인딩
	CollisionSphere->OnComponentHit.AddDynamic(this, &AShieldBarrierActor::OnBarrierHit);
	CollisionSphere->OnComponentBeginOverlap.AddDynamic(this, &AShieldBarrierActor::OnProjectileBeginOverlap);

	UE_LOG(LogTemp, Log, TEXT("ShieldBarrierActor 생성 (HP: %.0f)"), MaxHealth);
}

void AShieldBarrierActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 필요하면 회전 애니메이션 등 추가 가능
}

void AShieldBarrierActor::SetBarrierHealth(float InHealth)
{
	MaxHealth = InHealth;
	CurrentHealth = MaxHealth;
}

void AShieldBarrierActor::OnBarrierHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// Hit 이벤트는 필요 시 사용 (현재는 Overlap으로 처리)
}

void AShieldBarrierActor::OnProjectileBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	// 발사체인지 체크
	AFPSProjectile* Projectile = Cast<AFPSProjectile>(OtherActor);
	if (!Projectile)
	{
		return;
	}

	// Owner가 발사한 발사체는 무시 (내부에서 발사)
	if (IsProjectileFromOwner(Projectile))
	{
		UE_LOG(LogTemp, Verbose, TEXT("방어막: Owner 발사체 통과"));
		return;
	}

	// 외부 발사체 → 방어막 대미지 처리
	float Damage = Projectile->GetDamage(); // AFPSProjectile에 GetDamage() 필요
	ProcessDamage(Projectile, Damage);

	// 발사체 파괴
	Projectile->Destroy();
	UE_LOG(LogTemp, Log, TEXT("방어막: 발사체 차단 (남은 HP: %.0f)"), CurrentHealth);
}

void AShieldBarrierActor::ProcessDamage(AActor* DamageCauser, float Damage)
{
	CurrentHealth -= Damage;

	if (CurrentHealth <= 0.0f)
	{
		CurrentHealth = 0.0f;
		UE_LOG(LogTemp, Warning, TEXT("방어막 HP 소진! 파괴됩니다."));

		// 방어막 파괴
		Destroy();
	}
}

bool AShieldBarrierActor::IsProjectileFromOwner(AActor* ProjectileActor) const
{
	if (!ProjectileActor || !GetOwner())
	{
		return false;
	}

	// 발사체의 Instigator가 방어막 Owner와 같으면 내부 발사
	APawn* ProjectileInstigator = ProjectileActor->GetInstigator();
	APawn* BarrierOwner = Cast<APawn>(GetOwner());

	return (ProjectileInstigator == BarrierOwner);
}
