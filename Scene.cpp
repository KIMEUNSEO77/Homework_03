// CScene.cpp

#include "stdafx.h"
#include "Scene.h"

namespace
{
	float TerrainY(CHeightMapTerrain* pTerrain, float x, float z, float fOffset)
	{
		return((pTerrain) ? (pTerrain->GetHeight(x, z) + fOffset) : fOffset);
	}
	float DistanceXZ(XMFLOAT3 a, XMFLOAT3 b)
	{
		float x = a.x - b.x;
		float z = a.z - b.z;
		return(sqrtf((x * x) + (z * z)));
	}

	float RandomRange(float fMin, float fMax)
	{
		return(fMin + ((fMax - fMin) * (rand() / float(RAND_MAX))));
	}
	void TurnObjectToTarget(CGameObject* pObject, XMFLOAT3 xmf3Target)
	{
		if (!pObject) return;

		XMFLOAT3 xmf3Position = pObject->GetPosition();
		XMFLOAT3 xmf3Direction = Vector3::Subtract(xmf3Target, xmf3Position);
		xmf3Direction.y = 0.0f;
		float fLength = Vector3::Length(xmf3Direction);
		if (fLength < 0.001f) return;
		xmf3Direction = Vector3::ScalarProduct(xmf3Direction, 1.0f / fLength, false);

		XMFLOAT3 xmf3Look = pObject->GetLook();
		xmf3Look.y = 0.0f;
		fLength = Vector3::Length(xmf3Look);
		if (fLength < 0.001f) return;
		xmf3Look = Vector3::ScalarProduct(xmf3Look, 1.0f / fLength, false);

		float fDot = max(-1.0f, min(1.0f, (xmf3Look.x * xmf3Direction.x) + (xmf3Look.z * xmf3Direction.z)));
		float fCross = (xmf3Look.z * xmf3Direction.x) - (xmf3Look.x * xmf3Direction.z);
		float fYaw = XMConvertToDegrees(atan2f(fCross, fDot));
		pObject->Rotate(0.0f, fYaw, 0.0f);
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

CGameObject* CScene::CreateColorCube(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4 xmf4Color, float fSize)
{
	CGameObject* pObject = new CGameObject();
	pObject->SetMesh(new CCubeMesh(pd3dDevice, pd3dCommandList, fSize, fSize, fSize));
	pObject->m_nMaterials = 1;
	pObject->m_ppMaterials = new CMaterial * [1];
	pObject->m_ppMaterials[0] = new CMaterial();
	pObject->m_ppMaterials[0]->SetPseudoLightingShader();
	pObject->SetColor(xmf4Color);
	pObject->SetPosition(0.0f, -10000.0f, 0.0f);
	return(pObject);
}
void CScene::ReleaseSceneObjects(CGameObject** ppObjects, int nObjects)
{
	if (!ppObjects) return;
	for (int i = 0; i < nObjects; i++) if (ppObjects[i]) delete ppObjects[i];
	delete[] ppObjects;
}

void CScene::RenderSceneObjects(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, CGameObject** ppObjects, int nObjects)
{
	if (!ppObjects) return;
	for (int i = 0; i < nObjects; i++) if (ppObjects[i]) ppObjects[i]->Render(pd3dCommandList, pCamera);
}

void CScene::BuildTitleObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	vector<CGameObject*> vObjects;
	CMesh* pMesh = new CCubeMesh(pd3dDevice, pd3dCommandList, 6.0f, 6.0f, 6.0f);

	auto AddBlock = [&](float x, float y, float z, XMFLOAT4 color)
	{
		CGameObject* pObject = new CGameObject();
		pObject->SetMesh(pMesh);
		pObject->m_nMaterials = 1;
		pObject->m_ppMaterials = new CMaterial * [1];
		pObject->m_ppMaterials[0] = new CMaterial();
		pObject->m_ppMaterials[0]->SetPseudoLightingShader();
		pObject->SetColor(color);
		pObject->SetPosition(x, y, z);
		vObjects.push_back(pObject);
	};

	for (int y = 0; y < 7; y++)
	{
		for (int x = 0; x < 48; x++)
		{
			bool bBorder = (y == 0) || (y == 6) || (x == 0) || (x == 47);
			if (bBorder) AddBlock(-144.0f + (x * 6.0f), 70.0f - (y * 6.0f), 0.0f, XMFLOAT4(0.1f, 0.55f, 1.0f, 1.0f));
		}
	}

	for (int i = 0; i < 22; i++) AddBlock(-66.0f + (i * 6.0f), -5.0f, 0.0f, XMFLOAT4(1.0f, 0.9f, 0.1f, 1.0f));
	for (int i = 0; i < 10; i++) AddBlock(-30.0f + (i * 6.0f), -35.0f, 0.0f, XMFLOAT4(1.0f, 0.25f, 0.15f, 1.0f));

	m_nTitleObjects = (int)vObjects.size();
	m_ppTitleObjects = new CGameObject * [m_nTitleObjects];
	for (int i = 0; i < m_nTitleObjects; i++) m_ppTitleObjects[i] = vObjects[i];
}

void CScene::BuildMenuObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	vector<CGameObject*> vObjects;
	const float fY[6] = { 90.0f, 55.0f, 20.0f, -15.0f, -50.0f, -85.0f };
	for (int i = 0; i < 6; i++)
	{
		int nWidth = (i == 4) ? 16 : 24;
		for (int x = 0; x < nWidth; x++)
		{
			CGameObject* pObject = CreateColorCube(pd3dDevice, pd3dCommandList, (i == 4) ? XMFLOAT4(0.15f, 1.0f, 0.2f, 1.0f) : XMFLOAT4(0.8f, 0.8f, 0.85f, 1.0f), 5.0f);
			pObject->SetPosition(-(nWidth * 2.5f) + (x * 5.0f), fY[i], 0.0f);
			vObjects.push_back(pObject);
		}
	}
	m_nMenuObjects = (int)vObjects.size();
	m_ppMenuObjects = new CGameObject * [m_nMenuObjects];
	for (int i = 0; i < m_nMenuObjects; i++) m_ppMenuObjects[i] = vObjects[i];
}
void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);
	BuildTitleObjects(pd3dDevice, pd3dCommandList);
	BuildMenuObjects(pd3dDevice, pd3dCommandList);

	XMFLOAT3 xmf3TerrainScale(8.0f, 2.0f, 8.0f);
	m_pTerrain = new CHeightMapTerrain(pd3dDevice, pd3dCommandList, _T("Image/HeightMap.raw"), 257, 257, 257, 257, xmf3TerrainScale);

	m_nGameObjects = 19;
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
	const char* ppstrHouseFiles[4] =
	{
		"Models/Meshes/ams_house3.bin",
		"Models/Meshes/ams_house4.bin",
		"Models/Meshes/ams_house5.bin",
		"Models/Meshes/ams_house6.bin"
	};
	for (int i = 7; i < m_nGameObjects; i++)
	{
		m_ppGameObjects[i] = CreateHouseObject(pd3dDevice, pd3dCommandList, ppstrHouseFiles[(i - 3) % 4], 0.0f, 0.0f, 12.0f, RandomRange(-60.0f, 60.0f), m_pTerrain);
		m_ppGameObjects[i]->SetPosition(0.0f, -10000.0f, 0.0f);
		m_bHouseActive[i - 3] = false;
	}
	m_pBomb = CreateColorCube(pd3dDevice, pd3dCommandList, XMFLOAT4(4.0f, 0.15f, 0.1f, 1.0f), 8.0f);
	for (int i = 0; i < 10; i++) m_ppCoinObjects[i] = CreateColorCube(pd3dDevice, pd3dCommandList, XMFLOAT4(4.0f, 3.0f, 0.1f, 1.0f), 1.4f);
	for (int i = 0; i < 16; i++)
	{
		m_ppExplosionObjects[i] = CreateColorCube(pd3dDevice, pd3dCommandList, XMFLOAT4(4.0f, 0.8f, 0.1f, 1.0f), 4.0f);
		m_pxmf3ExplosionVelocity[i] = XMFLOAT3(0.0f, 0.0f, 0.0f);
	}
CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CScene::FireBomb()
{
	if (!m_pPlayer || m_bBombActive || m_bGameOver || m_bGameClear) return;

	XMFLOAT3 xmf3Position = m_pPlayer->GetPosition();
	xmf3Position.y -= 20.0f;
	m_pBomb->SetPosition(xmf3Position);
	m_bBombActive = true;
}

void CScene::RespawnHouse(int nIndex)
{
	if ((nIndex < 0) || (nIndex >= 16) || !m_pTerrain) return;

	float fMargin = 180.0f;
	float x = RandomRange(fMargin, m_pTerrain->GetWidth() - fMargin);
	float z = RandomRange(fMargin, m_pTerrain->GetLength() - fMargin);
	float y = TerrainY(m_pTerrain, x, z, 4.0f);
	CGameObject* pHouse = m_ppGameObjects[3 + nIndex];
	pHouse->SetPosition(x, y, z);
	m_bHouseActive[nIndex] = true;
}

void CScene::MakeExplosion(XMFLOAT3 xmf3Position)
{
	for (int i = 0; i < 16; i++)
	{
		float fAngle = XM_2PI * (i / 16.0f);
		float fSpeed = RandomRange(45.0f, 95.0f);
		m_pxmf3ExplosionVelocity[i] = XMFLOAT3(cosf(fAngle) * fSpeed, RandomRange(35.0f, 90.0f), sinf(fAngle) * fSpeed);
		m_ppExplosionObjects[i]->SetPosition(xmf3Position);
		m_pfExplosionTime[i] = 0.7f;
	}
}

void CScene::UpdateCoinObjects(CCamera* pCamera)
{
	if (!pCamera) return;

	XMFLOAT3 xmf3Camera = pCamera->GetPosition();
	XMFLOAT3 xmf3Look = pCamera->GetLookVector();
	XMFLOAT3 xmf3Right = pCamera->GetRightVector();
	XMFLOAT3 xmf3Up = pCamera->GetUpVector();

	for (int i = 0; i < 10; i++)
	{
		XMFLOAT3 xmf3Position = Vector3::Add(xmf3Camera, xmf3Look, 35.0f);
		xmf3Position = Vector3::Add(xmf3Position, xmf3Right, -8.0f + (i * 1.8f));
		xmf3Position = Vector3::Add(xmf3Position, xmf3Up, 6.0f);
		if ((i >= m_nCoins) && !m_bGameOver && !m_bGameClear) xmf3Position.y -= 10000.0f;
		m_ppCoinObjects[i]->SetPosition(xmf3Position);
	}
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
	ReleaseSceneObjects(m_ppTitleObjects, m_nTitleObjects);
	m_ppTitleObjects = NULL;
	m_nTitleObjects = 0;
	ReleaseSceneObjects(m_ppMenuObjects, m_nMenuObjects);
	m_ppMenuObjects = NULL;
	m_nMenuObjects = 0;

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
	for (int i = 0; i < m_nTitleObjects; i++) if (m_ppTitleObjects[i]) m_ppTitleObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < m_nMenuObjects; i++) if (m_ppMenuObjects[i]) m_ppMenuObjects[i]->ReleaseUploadBuffers();
	if (m_pBomb) m_pBomb->ReleaseUploadBuffers();
	for (int i = 0; i < 10; i++) if (m_ppCoinObjects[i]) m_ppCoinObjects[i]->ReleaseUploadBuffers();
	for (int i = 0; i < 16; i++) if (m_ppExplosionObjects[i]) m_ppExplosionObjects[i]->ReleaseUploadBuffers();
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (nMessageID == WM_LBUTTONDOWN)
	{
		m_GameState.m_bMouseDown = true;
		if (m_GameState.m_nScene == GAME_SCENE_TITLE)
		{
			m_GameState.m_nScene = GAME_SCENE_MENU;
			return(true);
		}
		if (m_GameState.m_nScene == GAME_SCENE_MENU)
		{
			m_GameState.m_nScene = GAME_SCENE_LEVEL1;
			if (m_pPlayer && m_pTerrain)
			{
				float fPlayerX = m_pTerrain->GetWidth() * 0.5f;
				float fPlayerZ = m_pTerrain->GetLength() * 0.5f;
				XMFLOAT3 xmf3PlayerPosition(fPlayerX, m_pTerrain->GetHeight(fPlayerX, fPlayerZ) + 70.0f, fPlayerZ);
				m_pPlayer->SetPosition(xmf3PlayerPosition);
				CCamera* pCamera = m_pPlayer->GetCamera();
				if (pCamera)
				{
					pCamera->SetOffset(XMFLOAT3(0.0f, 150.0f, -170.0f));
					pCamera->SetTimeLag(0.25f);
					pCamera->SetPosition(Vector3::Add(xmf3PlayerPosition, pCamera->GetOffset()));
				}
				m_pPlayer->Update(0.0f);
			}
			return(true);
		}
	}
	if (nMessageID == WM_LBUTTONUP) m_GameState.m_bMouseDown = false;
	return(false);
}
bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}
bool CScene::ProcessInput(UCHAR* pKeysBuffer)
{
	if (m_GameState.m_nScene != GAME_SCENE_LEVEL1) return(true);

	bool bFireKeyDown = ((pKeysBuffer[VK_SPACE] & 0xF0) != 0);
	if (bFireKeyDown && !m_bFireKeyDown) FireBomb();
	m_bFireKeyDown = bFireKeyDown;
	return(false);
}
void CScene::AnimateObjects(float fTimeElapsed)
{
	if (m_GameState.m_nScene == GAME_SCENE_TITLE)
	{
		for (int i = 0; i < m_nTitleObjects; i++)
		{
			m_ppTitleObjects[i]->Rotate(0.0f, 20.0f * fTimeElapsed, 0.0f);
			m_ppTitleObjects[i]->UpdateTransform(NULL);
		}
		return;
	}
	if (m_GameState.m_nScene == GAME_SCENE_MENU)
	{
		for (int i = 0; i < m_nMenuObjects; i++) m_ppMenuObjects[i]->UpdateTransform(NULL);
		return;
	}	for (int i = 0; i < m_nGameObjects; i++) m_ppGameObjects[i]->Animate(fTimeElapsed, NULL);

	if (!m_bGameOver && !m_bGameClear && m_pPlayer)
	{
		XMFLOAT3 xmf3Player = m_pPlayer->GetPosition();
		for (int i = 0; i < 2; i++)
		{
			XMFLOAT3 xmf3Enemy = m_ppGameObjects[i]->GetPosition();
			XMFLOAT3 xmf3Direction = Vector3::Subtract(xmf3Player, xmf3Enemy);
			float fDistance = Vector3::Length(xmf3Direction);
			if (fDistance > 1.0f)
			{
				TurnObjectToTarget(m_ppGameObjects[i], xmf3Player);
				float fMove = min(42.0f * fTimeElapsed, fDistance);
				xmf3Enemy = Vector3::Add(xmf3Enemy, xmf3Direction, fMove / fDistance);
				m_ppGameObjects[i]->SetPosition(xmf3Enemy);
			}
			if (fDistance < 65.0f) m_bGameOver = true;
		}

		m_fHouseRespawnTimer += fTimeElapsed;
		if (m_fHouseRespawnTimer >= 5.0f)
		{
			m_fHouseRespawnTimer = 0.0f;
						for (int i = 4; i < 16; i++)
			{
				if (!m_bHouseActive[i])
				{
					RespawnHouse(i);
					break;
				}
			}
		}
	}

	if (m_bBombActive && m_pBomb)
	{
		XMFLOAT3 xmf3Bomb = m_pBomb->GetPosition();
		xmf3Bomb.y -= 330.0f * fTimeElapsed;
		m_pBomb->SetPosition(xmf3Bomb);

		float fGround = (m_pTerrain) ? m_pTerrain->GetHeight(xmf3Bomb.x, xmf3Bomb.z) : 0.0f;
		if (xmf3Bomb.y <= (fGround + 2.0f))
		{
			m_pBomb->SetPosition(0.0f, -10000.0f, 0.0f);
			m_bBombActive = false;
		}

		for (int i = 0; i < 16; i++)
		{
			if (!m_bHouseActive[i]) continue;
			CGameObject* pHouse = m_ppGameObjects[3 + i];
			XMFLOAT3 xmf3House = pHouse->GetPosition();
			if ((DistanceXZ(xmf3Bomb, xmf3House) < 55.0f) && (xmf3Bomb.y <= (xmf3House.y + 95.0f)))
			{
				MakeExplosion(xmf3House);
				pHouse->SetPosition(0.0f, -10000.0f, 0.0f);
				m_bHouseActive[i] = false;
				m_pBomb->SetPosition(0.0f, -10000.0f, 0.0f);
				m_bBombActive = false;
				if (m_nCoins < 10) m_nCoins++;
				if (m_nCoins >= 10) m_bGameClear = true;
				break;
			}
		}
	}

	for (int i = 0; i < 16; i++)
	{
		if (m_pfExplosionTime[i] > 0.0f)
		{
			m_pfExplosionTime[i] -= fTimeElapsed;
			XMFLOAT3 xmf3Position = m_ppExplosionObjects[i]->GetPosition();
			xmf3Position = Vector3::Add(xmf3Position, m_pxmf3ExplosionVelocity[i], fTimeElapsed);
			m_pxmf3ExplosionVelocity[i].y -= 160.0f * fTimeElapsed;
			m_ppExplosionObjects[i]->SetPosition(xmf3Position);
		}
		else
		{
			m_ppExplosionObjects[i]->SetPosition(0.0f, -10000.0f, 0.0f);
		}
	}

	if (m_bGameClear || m_bGameOver)
	{
		m_fGameEndBlink += fTimeElapsed;
		XMFLOAT4 xmf4Color = (m_bGameClear) ? XMFLOAT4(4.0f, 3.0f, 0.1f, 1.0f) : XMFLOAT4(4.0f, 0.0f, 0.0f, 1.0f);
		for (int i = 0; i < 10; i++) if (m_ppCoinObjects[i]) m_ppCoinObjects[i]->SetColor(xmf4Color);
	}

	for (int i = 0; i < m_nGameObjects; i++) m_ppGameObjects[i]->UpdateTransform(NULL);
	if (m_pBomb) m_pBomb->UpdateTransform(NULL);
	for (int i = 0; i < 10; i++) if (m_ppCoinObjects[i]) m_ppCoinObjects[i]->UpdateTransform(NULL);
	for (int i = 0; i < 16; i++) if (m_ppExplosionObjects[i]) m_ppExplosionObjects[i]->UpdateTransform(NULL);
}
void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	if (m_pTerrain) m_pTerrain->Render(pd3dCommandList, pCamera);

	for (int i = 0; i < m_nGameObjects; i++)
	{
		if ((i >= 3) && !m_bHouseActive[i - 3]) continue;
		m_ppGameObjects[i]->Render(pd3dCommandList, pCamera);
	}

	if (m_bBombActive && m_pBomb) m_pBomb->Render(pd3dCommandList, pCamera);
	for (int i = 0; i < 16; i++) if (m_pfExplosionTime[i] > 0.0f) m_ppExplosionObjects[i]->Render(pd3dCommandList, pCamera);
	UpdateCoinObjects(pCamera);
	for (int i = 0; i < 10; i++) if (m_ppCoinObjects[i]) m_ppCoinObjects[i]->Render(pd3dCommandList, pCamera);
}
