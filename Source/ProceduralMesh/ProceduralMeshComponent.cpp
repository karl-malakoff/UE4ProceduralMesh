// UE4 Procedural Mesh Generation from the Epic Wiki (https://wiki.unrealengine.com/Procedural_Mesh_Generation)
//
// forked from "Engine/Plugins/Runtime/CustomMeshComponent/Source/CustomMeshComponent/Private/CustomMeshComponent.cpp"

#include "ProceduralMesh.h"
#include "DynamicMeshBuilder.h"
#include "ProceduralMeshComponent.h"
#include "Runtime/Launch/Resources/Version.h"

void FProceduralMeshData::ResetTriangles()
{
	Triangles.Reset();
}

void FProceduralMeshData::ResetVertices()
{
	VertexPositions.Reset();
	VertexColors.Reset();
}

int32 FProceduralMeshData::TrianglesNum() const
{
	return Triangles.Num();
}

int32 FProceduralMeshData::VerteciesNum() const
{
	return VertexPositions.Num();
}


/** Vertex Buffer */
class FProceduralMeshVertexBuffer : public FVertexBuffer
{
public:
	TArray<FDynamicMeshVertex> Vertices;

	virtual void InitRHI() override
	{
		FRHIResourceCreateInfo CreateInfo;
		VertexBufferRHI = RHICreateVertexBuffer(Vertices.Num() * sizeof(FDynamicMeshVertex), BUF_Static, CreateInfo);
		// Copy the vertex data into the vertex buffer.
		void* VertexBufferData = RHILockVertexBuffer(VertexBufferRHI, 0, Vertices.Num() * sizeof(FDynamicMeshVertex), RLM_WriteOnly);
		FMemory::Memcpy(VertexBufferData, Vertices.GetData(), Vertices.Num() * sizeof(FDynamicMeshVertex));
		RHIUnlockVertexBuffer(VertexBufferRHI);
	}
};

/** Index Buffer */
class FProceduralMeshIndexBuffer : public FIndexBuffer
{
public:
	TArray<int32> Indices;

	virtual void InitRHI() override
	{
		FRHIResourceCreateInfo CreateInfo;
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(int32), Indices.Num() * sizeof(int32), BUF_Static, CreateInfo);
		// Write the indices to the index buffer.
		void* Buffer = RHILockIndexBuffer(IndexBufferRHI, 0, Indices.Num() * sizeof(int32), RLM_WriteOnly);
		FMemory::Memcpy(Buffer, Indices.GetData(), Indices.Num() * sizeof(int32));
		RHIUnlockIndexBuffer(IndexBufferRHI);
	}
};

/** Vertex Factory */
class FProceduralMeshVertexFactory : public FLocalVertexFactory
{
public:
	FProceduralMeshVertexFactory()
	{
	}

	/** Initialization */
	void Init(const FProceduralMeshVertexBuffer* VertexBuffer)
	{
		// Commented out to enable building light of a level (but no backing is done for the procedural mesh itself)
		//check(!IsInRenderingThread());

		ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
			InitProceduralMeshVertexFactory,
			FProceduralMeshVertexFactory*, VertexFactory, this,
			const FProceduralMeshVertexBuffer*, VertexBuffer, VertexBuffer,
		{
			// Initialize the vertex factory's stream components.
			DataType NewData;
			NewData.PositionComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer,FDynamicMeshVertex,Position,VET_Float3);
			NewData.TextureCoordinates.Add(
				FVertexStreamComponent(VertexBuffer,STRUCT_OFFSET(FDynamicMeshVertex,TextureCoordinate),sizeof(FDynamicMeshVertex),VET_Float2)
				);
			NewData.TangentBasisComponents[0] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer,FDynamicMeshVertex,TangentX,VET_PackedNormal);
			NewData.TangentBasisComponents[1] = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer,FDynamicMeshVertex,TangentZ,VET_PackedNormal);
			NewData.ColorComponent = STRUCTMEMBER_VERTEXSTREAMCOMPONENT(VertexBuffer, FDynamicMeshVertex, Color, VET_Color);
			VertexFactory->SetData(NewData);
		});
	}
};



/** Scene proxy */
class FProceduralMeshSceneProxy : public FPrimitiveSceneProxy
{
public:

	FProceduralMeshSceneProxy(UProceduralMeshComponent* Component)
		: FPrimitiveSceneProxy(Component)
		, MaterialRelevance(Component->GetMaterialRelevance(GetScene().GetFeatureLevel()))
	{

		// Add each triangle to the vertex/index buffer
		for (int TriIdx = 0; TriIdx<Component->MeshData.TrianglesNum(); TriIdx++)
		{
			FProceduralMeshTriangle& Tri = Component->MeshData.Triangles[TriIdx];
			TArray<FVector>& VertexPositions = Component->MeshData.VertexPositions;
			TArray<FColor>& VertexColors = Component->MeshData.VertexColors;


			const FVector Edge01 = (VertexPositions[Tri.Vertex1] - VertexPositions[Tri.Vertex0]);
			const FVector Edge02 = (VertexPositions[Tri.Vertex2] - VertexPositions[Tri.Vertex0]);

			const FVector TangentX = Edge01.GetSafeNormal();
			const FVector TangentZ = (Edge02 ^ Edge01).GetSafeNormal();
			const FVector TangentY = (TangentX ^ TangentZ).GetSafeNormal();

			FDynamicMeshVertex Vert0;
			Vert0.Position = VertexPositions[Tri.Vertex0];
			Vert0.Color = VertexColors[Tri.Vertex0];
			Vert0.SetTangents(TangentX, TangentY, TangentZ);
			Vert0.TextureCoordinate.Set(Tri.UV0.U, Tri.UV0.V);
			int32 VIndex = VertexBuffer.Vertices.Add(Vert0);
			IndexBuffer.Indices.Add(VIndex);

			FDynamicMeshVertex Vert1;
			Vert1.Position = VertexPositions[Tri.Vertex1];
			Vert1.Color = VertexColors[Tri.Vertex1];
			Vert1.SetTangents(TangentX, TangentY, TangentZ);
			Vert1.TextureCoordinate.Set(Tri.UV1.U, Tri.UV1.V);
			VIndex = VertexBuffer.Vertices.Add(Vert1);
			IndexBuffer.Indices.Add(VIndex);

			FDynamicMeshVertex Vert2;
			Vert2.Position = VertexPositions[Tri.Vertex2];
			Vert2.Color = VertexColors[Tri.Vertex2];
			Vert2.SetTangents(TangentX, TangentY, TangentZ);
			Vert2.TextureCoordinate.Set(Tri.UV2.U, Tri.UV2.V);
			VIndex = VertexBuffer.Vertices.Add(Vert2);
			IndexBuffer.Indices.Add(VIndex);
		}

		// Init vertex factory
		VertexFactory.Init(&VertexBuffer);

		// Enqueue initialization of render resource
		BeginInitResource(&VertexBuffer);
		BeginInitResource(&IndexBuffer);
		BeginInitResource(&VertexFactory);

		// Grab material
		Material = Component->GetMaterial(0);
		if(Material == NULL)
		{
			Material = UMaterial::GetDefaultMaterial(MD_Surface);
		}
	}

	virtual ~FProceduralMeshSceneProxy()
	{
		VertexBuffer.ReleaseResource();
		IndexBuffer.ReleaseResource();
		VertexFactory.ReleaseResource();
	}

    virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
	{
		QUICK_SCOPE_CYCLE_COUNTER( STAT_ProceduralMeshSceneProxy_GetDynamicMeshElements );

		const bool bWireframe = AllowDebugViewmodes() && ViewFamily.EngineShowFlags.Wireframe;

		auto WireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy(IsSelected()) : NULL,
			FLinearColor(0, 0.5f, 1.f)
			);

		Collector.RegisterOneFrameMaterialProxy(WireframeMaterialInstance);

		FMaterialRenderProxy* MaterialProxy = NULL;
		if(bWireframe)
		{
			MaterialProxy = WireframeMaterialInstance;
		}
		else
		{
			MaterialProxy = Material->GetRenderProxy(IsSelected());
		}

		for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
		{
			if (VisibilityMap & (1 << ViewIndex))
			{
				const FSceneView* View = Views[ViewIndex];
				// Draw the mesh.
				FMeshBatch& Mesh = Collector.AllocateMesh();
				FMeshBatchElement& BatchElement = Mesh.Elements[0];
				BatchElement.IndexBuffer = &IndexBuffer;
				Mesh.bWireframe = bWireframe;
				Mesh.VertexFactory = &VertexFactory;
				Mesh.MaterialRenderProxy = MaterialProxy;
				BatchElement.PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), GetBounds(), GetLocalBounds(), true, UseEditorDepthTest());
				BatchElement.FirstIndex = 0;
				BatchElement.NumPrimitives = IndexBuffer.Indices.Num() / 3;
				BatchElement.MinVertexIndex = 0;
				BatchElement.MaxVertexIndex = VertexBuffer.Vertices.Num() - 1;
				Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
				Mesh.Type = PT_TriangleList;
				Mesh.DepthPriorityGroup = SDPG_World;
				Mesh.bCanApplyViewModeOverrides = false;
				Collector.AddMesh(ViewIndex, Mesh);
			}
		}
	}

	virtual void DrawDynamicElements(FPrimitiveDrawInterface* PDI, const FSceneView* View)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_ProceduralMeshSceneProxy_DrawDynamicElements);

		const bool bWireframe = AllowDebugViewmodes() && View->Family->EngineShowFlags.Wireframe;

		FColoredMaterialRenderProxy WireframeMaterialInstance(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy(IsSelected()) : NULL,
			FLinearColor(0, 0.5f, 1.f)
			);

		FMaterialRenderProxy* MaterialProxy = NULL;
		if(bWireframe)
		{
			MaterialProxy = &WireframeMaterialInstance;
		}
		else
		{
			MaterialProxy = Material->GetRenderProxy(IsSelected());
		}

		// Draw the mesh.
		FMeshBatch Mesh;
		FMeshBatchElement& BatchElement = Mesh.Elements[0];
		BatchElement.IndexBuffer = &IndexBuffer;
		Mesh.bWireframe = bWireframe;
		Mesh.VertexFactory = &VertexFactory;
		Mesh.MaterialRenderProxy = MaterialProxy;
		BatchElement.PrimitiveUniformBuffer = CreatePrimitiveUniformBufferImmediate(GetLocalToWorld(), GetBounds(), GetLocalBounds(), true, UseEditorDepthTest());
		BatchElement.FirstIndex = 0;
		BatchElement.NumPrimitives = IndexBuffer.Indices.Num() / 3;
		BatchElement.MinVertexIndex = 0;
		BatchElement.MaxVertexIndex = VertexBuffer.Vertices.Num() - 1;
		Mesh.ReverseCulling = IsLocalToWorldDeterminantNegative();
		Mesh.Type = PT_TriangleList;
		Mesh.DepthPriorityGroup = SDPG_World;
		PDI->DrawMesh(Mesh);
	}

	virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View)
	{
		FPrimitiveViewRelevance Result;
		Result.bDrawRelevance = IsShown(View);
		Result.bShadowRelevance = IsShadowCast(View);
		Result.bDynamicRelevance = true;
		MaterialRelevance.SetPrimitiveViewRelevance(Result);
		return Result;
	}

	virtual bool CanBeOccluded() const override
	{
		return !MaterialRelevance.bDisableDepthTest;
	}

		virtual uint32 GetMemoryFootprint(void) const
	{
		return(sizeof(*this) + GetAllocatedSize());
	}

	uint32 GetAllocatedSize(void) const
	{
		return(FPrimitiveSceneProxy::GetAllocatedSize());
	}

private:

	UMaterialInterface* Material;
	FProceduralMeshVertexBuffer VertexBuffer;
	FProceduralMeshIndexBuffer IndexBuffer;
	FProceduralMeshVertexFactory VertexFactory;

	FMaterialRelevance MaterialRelevance;
};


//////////////////////////////////////////////////////////////////////////

UProceduralMeshComponent::UProceduralMeshComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;

	SetCollisionProfileName(UCollisionProfile::BlockAllDynamic_ProfileName);
}

bool UProceduralMeshComponent::SetMeshData(const FProceduralMeshData& Data)
{
	//ensure that an equal number of positions, colors are present
	if (Data.VertexPositions.Num() == Data.VertexColors.Num())
	{
		//check that all indecies in the triangle array are valid
		for (auto Triangle : Data.Triangles)
		{
			if (!(Data.VertexColors.IsValidIndex(Triangle.Vertex0) &&
				Data.VertexColors.IsValidIndex(Triangle.Vertex1) &&
				Data.VertexColors.IsValidIndex(Triangle.Vertex2)))
			{
				return false;
			}
		}

		MeshData = Data;

		UpdateCollision();

		MarkRenderStateDirty();

		return true;
	}

	return false;
}

FProceduralMeshData& UProceduralMeshComponent::GetMeshData()
{
	return MeshData;
}

//bool UProceduralMeshComponent::SetProceduralMeshTriangles(const TArray<FProceduralMeshTriangle>& Triangles)
//{
//	ProceduralMeshTris = Triangles;
//
//	UpdateCollision();
//
//	// Need to recreate scene proxy to send it over
//	MarkRenderStateDirty();
//
//	return true;
//}

//void UProceduralMeshComponent::AddProceduralMeshTriangles(const TArray<FProceduralMeshTriangle>& Triangles)
//{
//	ProceduralMeshTris.Append(Triangles);
//
//	// Need to recreate scene proxy to send it over
//	MarkRenderStateDirty();
//}

void  UProceduralMeshComponent::ClearProceduralMeshTriangles()
{
	MeshData.ResetTriangles();

	// Need to recreate scene proxy to send it over
	MarkRenderStateDirty();
}


FPrimitiveSceneProxy* UProceduralMeshComponent::CreateSceneProxy()
{
	FPrimitiveSceneProxy* Proxy = NULL;
	// Only if have enough triangles
	if(MeshData.TrianglesNum() > 0)
	{
		Proxy = new FProceduralMeshSceneProxy(this);
	}
	return Proxy;
}

int32 UProceduralMeshComponent::GetNumMaterials() const
{
	return 1;
}


FBoxSphereBounds UProceduralMeshComponent::CalcBounds(const FTransform & LocalToWorld) const
{
	// Only if have enough triangles
	if (MeshData.TrianglesNum() > 0)
	{
		// Minimum Vector: It's set to the first vertex's position initially (NULL == FVector::ZeroVector might be required and a known vertex vector has intrinsically valid values)
		FVector vecMin = MeshData.VertexPositions[MeshData.Triangles[0].Vertex0];

		// Maximum Vector: It's set to the first vertex's position initially (NULL == FVector::ZeroVector might be required and a known vertex vector has intrinsically valid values)
		FVector vecMax = MeshData.VertexPositions[MeshData.Triangles[0].Vertex0];

		// Get maximum and minimum X, Y and Z positions of vectors
		for (int32 TriIdx = 0; TriIdx < MeshData.TrianglesNum(); TriIdx++)
		{
			const FVector &Vertex0 = MeshData.VertexPositions[MeshData.Triangles[TriIdx].Vertex0];
			const FVector &Vertex1 = MeshData.VertexPositions[MeshData.Triangles[TriIdx].Vertex1];
			const FVector &Vertex2 = MeshData.VertexPositions[MeshData.Triangles[TriIdx].Vertex2];

			vecMin.X = (vecMin.X > Vertex0.X) ? Vertex0.X : vecMin.X;
			vecMin.X = (vecMin.X > Vertex1.X) ? Vertex1.X : vecMin.X;
			vecMin.X = (vecMin.X > Vertex2.X) ? Vertex2.X : vecMin.X;

			vecMin.Y = (vecMin.Y > Vertex0.Y) ? Vertex0.Y : vecMin.Y;
			vecMin.Y = (vecMin.Y > Vertex1.Y) ? Vertex1.Y : vecMin.Y;
			vecMin.Y = (vecMin.Y > Vertex2.Y) ? Vertex2.Y : vecMin.Y;

			vecMin.Z = (vecMin.Z > Vertex0.Z) ? Vertex0.Z : vecMin.Z;
			vecMin.Z = (vecMin.Z > Vertex1.Z) ? Vertex1.Z : vecMin.Z;
			vecMin.Z = (vecMin.Z > Vertex2.Z) ? Vertex2.Z : vecMin.Z;

			vecMax.X = (vecMax.X < Vertex0.X) ? Vertex0.X : vecMax.X;
			vecMax.X = (vecMax.X < Vertex1.X) ? Vertex1.X : vecMax.X;
			vecMax.X = (vecMax.X < Vertex2.X) ? Vertex2.X : vecMax.X;

			vecMax.Y = (vecMax.Y < Vertex0.Y) ? Vertex0.Y : vecMax.Y;
			vecMax.Y = (vecMax.Y < Vertex1.Y) ? Vertex1.Y : vecMax.Y;
			vecMax.Y = (vecMax.Y < Vertex2.Y) ? Vertex2.Y : vecMax.Y;

			vecMax.Z = (vecMax.Z < Vertex0.Z) ? Vertex0.Z : vecMax.Z;
			vecMax.Z = (vecMax.Z < Vertex1.Z) ? Vertex1.Z : vecMax.Z;
			vecMax.Z = (vecMax.Z < Vertex2.Z) ? Vertex2.Z : vecMax.Z;
		}

		FVector vecOrigin = ((vecMax - vecMin) / 2) + vecMin;	/* Origin = ((Max Vertex's Vector - Min Vertex's Vector) / 2 ) + Min Vertex's Vector */
		FVector BoxPoint = vecMax - vecMin;			/* The difference between the "Maximum Vertex" and the "Minimum Vertex" is our actual Bounds Box */
		return FBoxSphereBounds(vecOrigin, BoxPoint, BoxPoint.Size()).TransformBy(LocalToWorld);
	}
	else
	{
		return FBoxSphereBounds();
	}
}


bool UProceduralMeshComponent::GetPhysicsTriMeshData(struct FTriMeshCollisionData* CollisionData, bool InUseAllTriData)
{
	FTriIndices Triangle;

	for (int32 i = 0; i<MeshData.TrianglesNum(); i++)
	{
		const FProceduralMeshTriangle& tri = MeshData.Triangles[i];

		Triangle.v0 = CollisionData->Vertices.Add(MeshData.VertexPositions[tri.Vertex0]);
		Triangle.v1 = CollisionData->Vertices.Add(MeshData.VertexPositions[tri.Vertex1]);
		Triangle.v2 = CollisionData->Vertices.Add(MeshData.VertexPositions[tri.Vertex2]);

		CollisionData->Indices.Add(Triangle);
		CollisionData->MaterialIndices.Add(i);
	}

	CollisionData->bFlipNormals = true;

	return true;
}

bool UProceduralMeshComponent::ContainsPhysicsTriMeshData(bool InUseAllTriData) const
{
	return (MeshData.TrianglesNum() > 0);
}

void UProceduralMeshComponent::UpdateBodySetup()
{
	if(ModelBodySetup == NULL)
	{
		ModelBodySetup = ConstructObject<UBodySetup>(UBodySetup::StaticClass(), this);
		ModelBodySetup->CollisionTraceFlag = CTF_UseComplexAsSimple;
		ModelBodySetup->bMeshCollideAll = true;
	}
}

void UProceduralMeshComponent::UpdateCollision()
{
	if(bPhysicsStateCreated)
	{
		DestroyPhysicsState();
		UpdateBodySetup();
		CreatePhysicsState();

		// Works in Packaged build only since UE4.5:
		ModelBodySetup->InvalidatePhysicsData();
		ModelBodySetup->CreatePhysicsMeshes();
	}
}

UBodySetup* UProceduralMeshComponent::GetBodySetup()
{
	UpdateBodySetup();
	return ModelBodySetup;
}
