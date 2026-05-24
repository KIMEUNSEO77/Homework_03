//-----------------------------------------------------------------------------
// File: Scene.h
//-----------------------------------------------------------------------------

#pragma once

#include "Shader.h"
#include "Player.h"

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

	bool ProcessInput(UCHAR* pKeysBuffer);
	void AnimateObjects(float fTimeElapsed);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

	void ReleaseUploadBuffers();

	void FireBomb();
	void RespawnHouse(int nIndex);
	void MakeExplosion(XMFLOAT3 xmf3Position);
	void UpdateCoinObjects(CCamera* pCamera);
	CGameObject* CreateColorCube(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4 xmf4Color, float fSize);

	CPlayer*					m_pPlayer = NULL;
	CHeightMapTerrain*			m_pTerrain = NULL;

public:
	ID3D12RootSignature*		m_pd3dGraphicsRootSignature = NULL;

	CGameObject**				m_ppGameObjects = NULL;
	int							m_nGameObjects = 0;
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
