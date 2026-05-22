//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"

namespace
{
	float TerrainY(CHeightMapTerrain* pTerrain, float x, float z, float fOffset)
	{
		return((pTerrain) ? (pTerrain->GetHeight(x, z) + fOffset) : fOffset);
	}

	CGameObject* CreateHouseObject(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		const char* pstrMeshFile, float x, float z, float fScale, float fYaw, CHeightMapTerrain* pTerrain)
	{
		CGameObject* pHouseObject = new CGameObject();
		pHouseObject->SetMesh(new CBinaryMeshFromFile(pd3dDevice, pd3dCommandList, pstrMeshFile));
		pHouseObject->m_nMaterials = 1;
		pHouseObject->m_ppMaterials = new CMaterial * [1];
		pHouseObject->m_ppMaterials[0] = new CMaterial();
		pHouseObject->m_ppMaterials[0]->SetPseudoLightingShader();
		pHouseObject->SetScale(fScale, fScale, fScale);
		pHouseObject->Rotate(0.0f, fYaw, 0.0f);
		pHouseObject->SetPosition(x, TerrainY(pTerrain, x, z, 4.0f), z);
		return(pHouseObject);
	}
}

CScene::CScene()
{
}

CScene::~CScene()
{
}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	XMFLOAT3 xmf3TerrainScale(8.0f, 2.0f, 8.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, _T("Image/HeightMap.raw"), 257, 257, 257, 257, xmf3TerrainScale);

	m_nGameObjects = 7;
	m_ppGameObjects = new CGameObject * [m_nGameObjects];

	CApacheObject *pApacheObject = new CApacheObject();
	CGameObject *pGameObject = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Model/Apache.txt");
	pApacheObject->SetChild(pGameObject);
	pApacheObject->OnInitialize();
	pApacheObject->SetScale(1.5f, 1.5f, 1.5f);
	pApacheObject->Rotate(0.0f, 90.0f, 0.0f);
	pApacheObject->SetPosition(620.0f, TerrainY(m_pTerrain, 620.0f, 760.0f, 45.0f), 760.0f);
	m_ppGameObjects[0] = pApacheObject;

	CSuperCobraObject* pSuperCobraObject = new CSuperCobraObject();
	pGameObject = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Model/SuperCobra.txt");
	pSuperCobraObject->SetChild(pGameObject);
	pSuperCobraObject->OnInitialize();
	pSuperCobraObject->SetScale(8.0f, 8.0f, 8.0f);
	pSuperCobraObject->Rotate(0.0f, -90.0f, 0.0f);
	pSuperCobraObject->SetPosition(760.0f, TerrainY(m_pTerrain, 760.0f, 900.0f, 45.0f), 900.0f);
	m_ppGameObjects[1] = pSuperCobraObject;

	CHummerObject* pHummerObject = new CHummerObject();
	pGameObject = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Model/M26.txt");
	pHummerObject->SetChild(pGameObject);
	pHummerObject->OnInitialize();
	pHummerObject->SetScale(8.0f, 8.0f, 8.0f);
	pHummerObject->Rotate(0.0f, -90.0f, 0.0f);
	pHummerObject->SetPosition(900.0f, TerrainY(m_pTerrain, 900.0f, 720.0f, 14.0f), 720.0f);
	m_ppGameObjects[2] = pHummerObject;

	m_ppGameObjects[3] = CreateHouseObject(pd3dDevice, pd3dCommandList, "Models/Meshes/ams_house3.bin", 920.0f, 1040.0f, 12.0f, 20.0f, m_pTerrain);
	m_ppGameObjects[4] = CreateHouseObject(pd3dDevice, pd3dCommandList, "Models/Meshes/ams_house4.bin", 1040.0f, 920.0f, 12.0f, -35.0f, m_pTerrain);
	m_ppGameObjects[5] = CreateHouseObject(pd3dDevice, pd3dCommandList, "Models/Meshes/ams_house5.bin", 1120.0f, 1120.0f, 12.0f, 55.0f, m_pTerrain);
	m_ppGameObjects[6] = CreateHouseObject(pd3dDevice, pd3dCommandList, "Models/Meshes/ams_house6.bin", 780.0f, 1120.0f, 12.0f, -10.0f, m_pTerrain);
CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	if (m_pTerrain) delete m_pTerrain;
	m_pTerrain = NULL;

	if (m_ppGameObjects)
	{
		for (int i = 0; i < m_nGameObjects; i++) delete m_ppGameObjects[i];
		delete[] m_ppGameObjects;
	}

	ReleaseShaderVariables();
}

ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_ROOT_PARAMETER pd3dRootParameters[2];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 32;
	pd3dRootParameters[1].Constants.ShaderRegister = 2; //GameObject
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)& pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CScene::ReleaseShaderVariables()
{
}

void CScene::ReleaseUploadBuffers()
{
	if (m_pTerrain) m_pTerrain->ReleaseUploadBuffers();
	for (int i = 0; i < m_nGameObjects; i++) m_ppGameObjects[i]->ReleaseUploadBuffers();
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 'W': m_ppGameObjects[0]->MoveForward(+1.0f); break;
		case 'S': m_ppGameObjects[0]->MoveForward(-1.0f); break;
		case 'A': m_ppGameObjects[0]->MoveStrafe(-1.0f); break;
		case 'D': m_ppGameObjects[0]->MoveStrafe(+1.0f); break;
		case 'Q': m_ppGameObjects[0]->MoveUp(+1.0f); break;
		case 'R': m_ppGameObjects[0]->MoveUp(-1.0f); break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return(false);
}

bool CScene::ProcessInput(UCHAR* pKeysBuffer)
{
	return(false);
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	for (int i = 0; i < m_nGameObjects; i++) m_ppGameObjects[i]->Animate(fTimeElapsed, NULL);

	for (int i = 0; i < m_nGameObjects; i++) m_ppGameObjects[i]->UpdateTransform(NULL);
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nGameObjects; i++) m_ppGameObjects[i]->Render(pd3dCommandList, pCamera);
}
