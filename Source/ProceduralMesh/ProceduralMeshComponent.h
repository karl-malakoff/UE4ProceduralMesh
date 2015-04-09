// UE4 Procedural Mesh Generation from the Epic Wiki (https://wiki.unrealengine.com/Procedural_Mesh_Generation)
//
// forked from "Engine/Plugins/Runtime/CustomMeshComponent/Source/CustomMeshComponent/Classes/CustomMeshComponent.h"

#pragma once

#include "ProceduralMeshComponent.generated.h"

//Positions and colors should be seperate
//UV's should be per face not per vertex?
USTRUCT(BlueprintType)
struct FProceduralMeshVertexUV
{
	GENERATED_USTRUCT_BODY()

	FProceduralMeshVertexUV()
		: U(0.f)
		, V(0.f){}

	FProceduralMeshVertexUV(float InU, float InV)
		: U(InU)
		, V(InV){}

	UPROPERTY(EditAnywhere, Category=Triangle)
	float U;

	UPROPERTY(EditAnywhere, Category=Triangle)
	float V;
};

//This replicates a lot of data, also makes it hard to look things up
//should use int
USTRUCT(BlueprintType)
struct FProceduralMeshTriangle
{
	GENERATED_USTRUCT_BODY()

	FProceduralMeshTriangle()
		: Vertex0(0)
		, Vertex1(0)
		, Vertex2(0){}

	FProceduralMeshTriangle(int32 V0, int32 V1, int32 V2)
		: Vertex0(V0)
		, Vertex1(V1)
		, Vertex2(V2) {}

	UPROPERTY(EditAnywhere, Category=Triangle)
	int32 Vertex0;

	UPROPERTY(EditAnywhere, Category=Triangle)
	int32 Vertex1;

	UPROPERTY(EditAnywhere, Category=Triangle)
	int32 Vertex2;

	UPROPERTY(EditAnywhere, Category = Triangle)
	FProceduralMeshVertexUV UV0;

	UPROPERTY(EditAnywhere, Category = Triangle)
	FProceduralMeshVertexUV UV1;

	UPROPERTY(EditAnywhere, Category = Triangle)
	FProceduralMeshVertexUV UV2;
};

USTRUCT(BlueprintType)
struct FProceduralMeshData
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Procedual Mesh Data")
	TArray<FVector> VertexPositions;

	UPROPERTY(EditAnywhere, Category = "Procedual Mesh Data")
	TArray<FColor> VertexColors;

	UPROPERTY(EditAnywhere, Category = "Procedual Mesh Data")
	TArray<FProceduralMeshTriangle> Triangles;

	int32 TrianglesNum() const;
	int32 VerteciesNum() const;

	void ResetTriangles();
	void ResetVertices();
};

/** Component that allows you to specify custom triangle mesh geometry */
UCLASS(editinlinenew, meta = (BlueprintSpawnableComponent), ClassGroup=Rendering)
class UProceduralMeshComponent : public UMeshComponent, public IInterface_CollisionDataProvider
{
	GENERATED_UCLASS_BODY()

public:
	/** Set the geometry to use on this triangle mesh */
	//UFUNCTION(BlueprintCallable, Category="Components|ProceduralMesh")
	//bool SetProceduralMeshTriangles(const TArray<FProceduralMeshTriangle>& Triangles);


	/** Add to the geometry to use on this triangle mesh, should be called after adding vertices as it will check that indecies are valid.  This may cause an allocation.  Use SetCustomMeshTriangles() instead when possible to reduce allocations. */
	//UFUNCTION(BlueprintCallable, Category="Components|ProceduralMesh")
	//void AddProceduralMeshTriangles(const TArray<FProceduralMeshTriangle>& Triangles);

	/**Set the mesh data*/
	UFUNCTION(BlueprintCallable, Category = "Components|ProceduralMesh")
		bool SetMeshData(const FProceduralMeshData& Data);

	/** Removes all geometry from this triangle mesh including vercixes and colors.  Does not deallocate memory, allowing new geometry to reuse the existing allocation. */
	UFUNCTION(BlueprintCallable, Category="Components|ProceduralMesh")
	void ClearProceduralMeshTriangles();

	/**Get a refference to the data, adding or removing from sub arrays could cause errors, only modify data */
	UFUNCTION(BLueprintCallable, Category = "Components|ProceduralMesh")
		FProceduralMeshData& GetMeshData();

	//Want a way to ensure that vertex colors and vertices stay the same...
	//While ensuring that colors (and vertices) can still be changed

	//transform and set functions for all 3 arrays, triangles can't change for now
	//just get a reference to the arrays, let aactor modify it directly
	/** Transform vertices from index a to index b by vector */
	//UFUNCTION(BlueprintCallable, Category = "Components|ProceduralMesh")
	//	TArray<	 TransformVertices(int32 VertexStart)

	/** Description of collision */
	UPROPERTY(BlueprintReadOnly, Category="Collision")
	class UBodySetup* ModelBodySetup;

	// Begin Interface_CollisionDataProvider Interface
	virtual bool GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData) override;
	virtual bool ContainsPhysicsTriMeshData(bool InUseAllTriData) const override;
	virtual bool WantsNegXTriMesh() override{ return false; }
	// End Interface_CollisionDataProvider Interface

	// Begin UPrimitiveComponent interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	virtual class UBodySetup* GetBodySetup() override;
	// End UPrimitiveComponent interface.

	// Begin UMeshComponent interface.
	virtual int32 GetNumMaterials() const override;
	// End UMeshComponent interface.

	void UpdateBodySetup();
	void UpdateCollision();

private:
	// Begin USceneComponent interface.
	virtual FBoxSphereBounds CalcBounds(const FTransform & LocalToWorld) const override;
	// Begin USceneComponent interface.

	/** The mesh data */
	UPROPERTY()
	FProceduralMeshData MeshData;

	friend class FProceduralMeshSceneProxy;
};
