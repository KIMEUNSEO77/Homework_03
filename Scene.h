//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Shader.h"
#include "Player.h"

enum GAME_SCENE_ID
{
	GAME_SCENE_TITLE = 0,
	GAME_SCENE_MENU = 1,
	GAME_SCENE_LEVEL1 = 2
};

struct GAME_STATE
{
	int m_nScene = GAME_SCENE_TITLE;
	bool m_bMouseDown = false;
};
class CScene
{
public:
	CScene();
	~CScene();

	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseObjects();

	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }
	CHeightMapTerrain* GetTerrain() { return(m_pTerrain); }
	bool IsLevelScene() { return(m_GameState.m_nScene == GAME_SCENE_LEVEL1); }

	bool ProcessInput(UCHAR* pKeysBuffer);
	void AnimateObjects(float fTimeElapsed);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

	void ReleaseUploadBuffers();

	void FireBomb();
	void RespawnHouse(int nIndex);
	void MakeExplosion(XMFLOAT3 xmf3Position);
	void UpdateCoinObjects(CCamera* pCamera);
	CGameObject* CreateColorCube(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4 xmf4Color, float fSize);
	void BuildTitleObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void BuildMenuObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseSceneObjects(CGameObject** ppObjects, int nObjects);
	void RenderSceneObjects(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera, CGameObject** ppObjects, int nObjects);
	bool IsTitleNameClicked(HWND hWnd, LPARAM lParam);
	bool IsMenuStartClicked(HWND hWnd, LPARAM lParam);
	void StartTitleNameExplosion();

	CPlayer*					m_pPlayer = NULL;
	CHeightMapTerrain*			m_pTerrain = NULL;

public:
	ID3D12RootSignature*		m_pd3dGraphicsRootSignature = NULL;
	GAME_STATE					 m_GameState;

	CGameObject**				m_ppGameObjects = NULL;
	int							m_nGameObjects = 0;
	CGameObject**				 m_ppTitleObjects = NULL;
	int								 m_nTitleObjects = 0;
	int							 m_nTitleNameStart = 0;
	int							 m_nTitleNameObjects = 0;
	bool							 m_bTitleNameExploding = false;
	float							 m_fTitleExplosionTimer = 0.0f;
	XMFLOAT3*				 m_pxmf3TitleObjectVelocity = NULL;
	CGameObject**				 m_ppMenuObjects = NULL;
	int								 m_nMenuObjects = 0;
	bool								m_bHouseActive[16] = { true, true, true, true };
	bool								m_bBombActive = false;
	bool								m_bFireKeyDown = false;
	bool								m_bGameClear = false;
	bool								m_bGameOver = false;
	int									m_nCoins = 0;
	float								m_fHouseRespawnTimer = 0.0f;
	float								m_fGameEndBlink = 0.0f;

	CGameObject*					m_pBomb = NULL;
	CGameObject*					m_ppCoinObjects[10] = { NULL };
	CGameObject*					m_ppExplosionObjects[16] = { NULL };
	XMFLOAT3						m_pxmf3ExplosionVelocity[16];
	float								m_pfExplosionTime[16] = { 0.0f };
};
