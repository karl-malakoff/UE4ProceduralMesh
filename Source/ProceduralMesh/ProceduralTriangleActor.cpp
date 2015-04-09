// UE4 Procedural Mesh Generation from the Epic Wiki (https://wiki.unrealengine.com/Procedural_Mesh_Generation)
//

#include "ProceduralMesh.h"
#include "ProceduralTriangleActor.h"

AProceduralTriangleActor::AProceduralTriangleActor()
{
	mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("ProceduralTriangle"));

	// Apply a simple material directly using the VertexColor as its BaseColor input
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("Material'/Game/Materials/BaseColor.BaseColor'"));
	// TODO Apply a real material with textures, using UVs
//	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("Material'/Game/Materials/M_Concrete_Poured.M_Concrete_Poured'"));
	mesh->SetMaterial(0, Material.Object);

	// Generate a single triangle
	TArray<FProceduralMeshTriangle> triangles;
	FProceduralMeshData MeshData;
	GenerateTriangle(MeshData);
	mesh->SetMeshData(MeshData);

	RootComponent = mesh;
}

// Generate a single horizontal triangle counterclockwise to point up (one face, visible only from the top, not from the bottom)
void AProceduralTriangleActor::GenerateTriangle(FProceduralMeshData& OutData)
{
	OutData.VertexPositions.Add(FVector(  0.f, -80.f, 0.f));
	OutData.VertexPositions.Add(FVector  (0.f, 80.f, 0.f));
	OutData.VertexPositions.Add(FVector(100.f, 0.f, 0.f));

	static const FColor Blue(51, 51, 255);

	OutData.VertexColors.Append(&Blue, 3);

	FProceduralMeshTriangle triangle(0, 1, 2);

	FProceduralMeshVertexUV UV;

	triangle.UV0 = UV;
	UV.U = 1.f;
	UV.V = 0.f;
	triangle.UV1 = UV;
	UV.U = 0.5f;
	UV.V = 0.75f;
	triangle.UV2 = UV;

	OutData.Triangles.Add(triangle);
}
