#include "IDirect3DDevice9Proxy.h"
#include "LolKnow.h"
#include <thread>

IDirect3DDevice9Proxy *IDirect3DDevice9Proxy::lastDevice = NULL;

void* IDirect3DDevice9Proxy::callbacks[D3D9_DEVICE_FUNC_COUNT] = {NULL};

extern "C" UINT WINAPI D3D9DeviceFuncHook(UINT funcId, void* funcRef){
	//Check existence of device
	if (!IDirect3DDevice9Proxy::lastDevice)
		return D3D_DEVICE_PROXY_STATUS_NOTREADY;
	if(funcId>(D3D9_DEVICE_FUNC_COUNT-1))
		return D3D_DEVICE_PROXY_STATUS_WRONG_FUNC_ID;
	if(!funcRef)
		return D3D_DEVICE_PROXY_STATUS_WRONG_FUNC;
	//Mkay, set it up
	IDirect3DDevice9Proxy::callbacks[funcId] = funcRef;
	return 1;
}

extern "C" UINT WINAPI D3D9DeviceFuncUnHook(UINT funcId){
	//Check existence of device
	if (!IDirect3DDevice9Proxy::lastDevice)
		return D3D_DEVICE_PROXY_STATUS_NOTREADY;
	if(funcId>(D3D9_DEVICE_FUNC_COUNT-1))
		return D3D_DEVICE_PROXY_STATUS_WRONG_FUNC_ID;
	//Mkay, set it up
	IDirect3DDevice9Proxy::callbacks[funcId] = NULL;
	return 1;
}

IDirect3DDevice9Proxy::IDirect3DDevice9Proxy(IDirect3DDevice9* pOriginal){
	origIDirect3DDevice9 = pOriginal; // store the pointer to original object
	lastDevice = this;
	fontCreated = false;
}

IDirect3DDevice9Proxy::~IDirect3DDevice9Proxy(void){
	lastDevice = NULL;
}

HRESULT IDirect3DDevice9Proxy::QueryInterface(REFIID riid, void** ppvObj){
	// check if original dll can provide interface. then send *our* address
	*ppvObj = NULL;
	HRESULT hRes = origIDirect3DDevice9->QueryInterface(riid, ppvObj); 
	if (hRes == NOERROR)
		*ppvObj = this;
	return hRes;
}

ULONG IDirect3DDevice9Proxy::AddRef(void){
	return(origIDirect3DDevice9->AddRef());
}

ULONG IDirect3DDevice9Proxy::Release(void){
	// ATTENTION: This is a booby-trap ! Watch out !
	// If we create our own sprites, surfaces, etc. (thus increasing the ref counter
	// by external action), we need to delete that objects before calling the original
	// Release function	

	// release/delete own objects
	// .....

	// Calling original function now
	ULONG count = origIDirect3DDevice9->Release();
	// destructor will be called automatically
	if (count == 0){
		delete(this);
	}
	return (count);
}

HRESULT IDirect3DDevice9Proxy::TestCooperativeLevel(void){
	return(origIDirect3DDevice9->TestCooperativeLevel());
}

UINT IDirect3DDevice9Proxy::GetAvailableTextureMem(void){
	return(origIDirect3DDevice9->GetAvailableTextureMem());
}

HRESULT IDirect3DDevice9Proxy::EvictManagedResources(void){
	return(origIDirect3DDevice9->EvictManagedResources());
}

HRESULT IDirect3DDevice9Proxy::GetDirect3D(IDirect3D9** ppD3D9){
	return(origIDirect3DDevice9->GetDirect3D(ppD3D9));
}

HRESULT IDirect3DDevice9Proxy::GetDeviceCaps(D3DCAPS9* pCaps){
	return(origIDirect3DDevice9->GetDeviceCaps(pCaps));
}

HRESULT IDirect3DDevice9Proxy::GetDisplayMode(UINT iSwapChain, D3DDISPLAYMODE* pMode){
	return(origIDirect3DDevice9->GetDisplayMode(iSwapChain, pMode));
}

HRESULT IDirect3DDevice9Proxy::GetCreationParameters(D3DDEVICE_CREATION_PARAMETERS *pParameters){
	return(origIDirect3DDevice9->GetCreationParameters(pParameters));
}

HRESULT IDirect3DDevice9Proxy::SetCursorProperties(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap){
	return(origIDirect3DDevice9->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap));
}

void IDirect3DDevice9Proxy::SetCursorPosition(int X, int Y, DWORD Flags){
	return(origIDirect3DDevice9->SetCursorPosition(X, Y, Flags));
}

BOOL IDirect3DDevice9Proxy::ShowCursor(BOOL bShow){
	return(origIDirect3DDevice9->ShowCursor(bShow));
}

HRESULT IDirect3DDevice9Proxy::CreateAdditionalSwapChain(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** pSwapChain){
	return(origIDirect3DDevice9->CreateAdditionalSwapChain(pPresentationParameters, pSwapChain));
}

HRESULT IDirect3DDevice9Proxy::GetSwapChain(UINT iSwapChain, IDirect3DSwapChain9** pSwapChain){
	return(origIDirect3DDevice9->GetSwapChain(iSwapChain, pSwapChain));
}

UINT IDirect3DDevice9Proxy::GetNumberOfSwapChains(void){
	return(origIDirect3DDevice9->GetNumberOfSwapChains());
}

HRESULT IDirect3DDevice9Proxy::Reset(D3DPRESENT_PARAMETERS* pPresentationParameters){
	if (callbacks[PRERESET])
		((D3D9DevicePreResetFunc)callbacks[PRERESET])();
	HRESULT res = (origIDirect3DDevice9->Reset(pPresentationParameters));
	if (callbacks[POSTRESET])
		((D3D9DevicePostResetFunc)callbacks[POSTRESET])(this, res);
	return res;
}

HRESULT IDirect3DDevice9Proxy::Present(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion){
	
	if(origIDirect3DDevice9->TestCooperativeLevel() == D3D_OK)
	{
		DisplayText(D3DCOLOR_ARGB(255,255,0,0), 850, 0, 220, 25, "LolKnow Development Build");
		if(!LolKnow::hasCreatedThread)
		{
			std::thread t1(LolKnow::timerCheckForData, 250);
			t1.detach();
			LolKnow::hasCreatedThread = true;
		}
		
		if(LolKnow::completedDataTransfer)
		{
			//Display data
			stringstream ss;

			IDirect3DSurface9* m_surface;
			origIDirect3DDevice9->GetBackBuffer(0,0,D3DBACKBUFFER_TYPE_MONO,&m_surface);
			HDC hdc;
			(*m_surface).GetDC(&hdc);
			
			bool isIdentical = true;

			ss << &hdc;
			DisplayText(D3DCOLOR_ARGB(255,255,255,255), 0, 0, 200, 25, ss.str());

			for(int x = 0; x < m_hdc.size(); x++)
			{
				ss = stringstream();
				ss << m_hdc.at(x);
				DisplayText(D3DCOLOR_ARGB(255,255,255,255), 0, 50+(25*x), 200, 25, ss.str());

				if(&hdc != m_hdc.at(x))
				{
					isIdentical = false;
				}
			}

			if(!isIdentical)
			{
				for(int x = 0; x < LolKnow::teamOne.size(); x++)
					DisplayData(1,x,(1720/(LolKnow::teamOne.size()+1))*(x+1),35);
				for(int x = 0; x < LolKnow::teamTwo.size(); x++)
					DisplayData(2,x,(1720/(LolKnow::teamOne.size()+1))*(x+1),955);
			}
			if(m_hdc.size() < 10)
			{
				m_hdc.push_back(&hdc);
			}
			else
			{
				m_hdc.insert(m_hdc.begin(), &hdc);
				m_hdc.pop_back();
			}
			m_surface->Release();
		}
	}
	
	HRESULT res = (origIDirect3DDevice9->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion));

	if (callbacks[POSTPRESENT])
		((D3D9DevicePostPresentFunc)callbacks[POSTPRESENT])(this, res);

	return res;
}

HRESULT IDirect3DDevice9Proxy::GetBackBuffer(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer){
	return(origIDirect3DDevice9->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer));
}

HRESULT IDirect3DDevice9Proxy::GetRasterStatus(UINT iSwapChain, D3DRASTER_STATUS* pRasterStatus){
	return(origIDirect3DDevice9->GetRasterStatus(iSwapChain, pRasterStatus));
}

HRESULT IDirect3DDevice9Proxy::SetDialogBoxMode(BOOL bEnableDialogs){
	return(origIDirect3DDevice9->SetDialogBoxMode(bEnableDialogs));
}

void IDirect3DDevice9Proxy::SetGammaRamp(UINT iSwapChain, DWORD Flags, CONST D3DGAMMARAMP* pRamp){
	return(origIDirect3DDevice9->SetGammaRamp(iSwapChain, Flags, pRamp));
}

void IDirect3DDevice9Proxy::GetGammaRamp(UINT iSwapChain, D3DGAMMARAMP* pRamp){
	return(origIDirect3DDevice9->GetGammaRamp(iSwapChain, pRamp));
}

HRESULT IDirect3DDevice9Proxy::CreateTexture(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle){
	return(origIDirect3DDevice9->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle));
}

HRESULT IDirect3DDevice9Proxy::CreateVolumeTexture(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle){
	return(origIDirect3DDevice9->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle));
}

HRESULT IDirect3DDevice9Proxy::CreateCubeTexture(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle){
	return(origIDirect3DDevice9->CreateCubeTexture(EdgeLength, Levels, Usage,Format, Pool, ppCubeTexture, pSharedHandle));
}

HRESULT IDirect3DDevice9Proxy::CreateVertexBuffer(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle){
	return(origIDirect3DDevice9->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle));
}

HRESULT IDirect3DDevice9Proxy::CreateIndexBuffer(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle){
	return(origIDirect3DDevice9->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle));
}

HRESULT IDirect3DDevice9Proxy::CreateRenderTarget(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle){
	return(origIDirect3DDevice9->CreateRenderTarget(Width,Height,Format,MultiSample,MultisampleQuality,Lockable,ppSurface,pSharedHandle));
}

HRESULT IDirect3DDevice9Proxy::CreateDepthStencilSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle){
	return(origIDirect3DDevice9->CreateDepthStencilSurface(Width,Height,Format,MultiSample,MultisampleQuality,Discard,ppSurface,pSharedHandle));
}

HRESULT IDirect3DDevice9Proxy::UpdateSurface(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint){
	return(origIDirect3DDevice9->UpdateSurface(pSourceSurface,pSourceRect,pDestinationSurface,pDestPoint));
}

HRESULT IDirect3DDevice9Proxy::UpdateTexture(IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture){
	return(origIDirect3DDevice9->UpdateTexture(pSourceTexture,pDestinationTexture));
}

HRESULT IDirect3DDevice9Proxy::GetRenderTargetData(IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface){
	return(origIDirect3DDevice9->GetRenderTargetData(pRenderTarget,pDestSurface));
}

HRESULT IDirect3DDevice9Proxy::GetFrontBufferData(UINT iSwapChain,IDirect3DSurface9* pDestSurface){
	return(origIDirect3DDevice9->GetFrontBufferData(iSwapChain,pDestSurface));
}

HRESULT IDirect3DDevice9Proxy::StretchRect(IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter){
	return(origIDirect3DDevice9->StretchRect(pSourceSurface,pSourceRect,pDestSurface,pDestRect,Filter));
}

HRESULT IDirect3DDevice9Proxy::ColorFill(IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color){
	return(origIDirect3DDevice9->ColorFill(pSurface,pRect,color));
}

HRESULT IDirect3DDevice9Proxy::CreateOffscreenPlainSurface(UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle){
	return(origIDirect3DDevice9->CreateOffscreenPlainSurface(Width,Height,Format,Pool,ppSurface,pSharedHandle));
}

HRESULT IDirect3DDevice9Proxy::SetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget){
	return(origIDirect3DDevice9->SetRenderTarget(RenderTargetIndex,pRenderTarget));
}

HRESULT IDirect3DDevice9Proxy::GetRenderTarget(DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget){
	return(origIDirect3DDevice9->GetRenderTarget(RenderTargetIndex,ppRenderTarget));
}

HRESULT IDirect3DDevice9Proxy::SetDepthStencilSurface(IDirect3DSurface9* pNewZStencil){
	return(origIDirect3DDevice9->SetDepthStencilSurface(pNewZStencil));
}

HRESULT IDirect3DDevice9Proxy::GetDepthStencilSurface(IDirect3DSurface9** ppZStencilSurface){
	return(origIDirect3DDevice9->GetDepthStencilSurface(ppZStencilSurface));
}

HRESULT IDirect3DDevice9Proxy::BeginScene(void){
	return(origIDirect3DDevice9->BeginScene());
}

HRESULT IDirect3DDevice9Proxy::EndScene(void){
	if (callbacks[ENDSCENE])
		((D3D9DeviceEndSceneFunc)callbacks[ENDSCENE])(this);
	return(origIDirect3DDevice9->EndScene());
}

HRESULT IDirect3DDevice9Proxy::Clear(DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil){
	return(origIDirect3DDevice9->Clear(Count,pRects,Flags,Color,Z,Stencil));
}

HRESULT IDirect3DDevice9Proxy::SetTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix){
	return(origIDirect3DDevice9->SetTransform(State,pMatrix));
}

HRESULT IDirect3DDevice9Proxy::GetTransform(D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix){
	return(origIDirect3DDevice9->GetTransform(State,pMatrix));
}

HRESULT IDirect3DDevice9Proxy::MultiplyTransform(D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix){
	return(origIDirect3DDevice9->MultiplyTransform(State,pMatrix));
}

HRESULT IDirect3DDevice9Proxy::SetViewport(CONST D3DVIEWPORT9* pViewport){
	return(origIDirect3DDevice9->SetViewport(pViewport));
}

HRESULT IDirect3DDevice9Proxy::GetViewport(D3DVIEWPORT9* pViewport){
	return(origIDirect3DDevice9->GetViewport(pViewport));
}

HRESULT IDirect3DDevice9Proxy::SetMaterial(CONST D3DMATERIAL9* pMaterial){
	return(origIDirect3DDevice9->SetMaterial(pMaterial));
}

HRESULT IDirect3DDevice9Proxy::GetMaterial(D3DMATERIAL9* pMaterial){
	return(origIDirect3DDevice9->GetMaterial(pMaterial));
}

HRESULT IDirect3DDevice9Proxy::SetLight(DWORD Index,CONST D3DLIGHT9* pLight){
	return(origIDirect3DDevice9->SetLight(Index,pLight));
}

HRESULT IDirect3DDevice9Proxy::GetLight(DWORD Index,D3DLIGHT9* pLight){
	return(origIDirect3DDevice9->GetLight(Index,pLight));
}

HRESULT IDirect3DDevice9Proxy::LightEnable(DWORD Index,BOOL Enable){
	return(origIDirect3DDevice9->LightEnable(Index,Enable));
}

HRESULT IDirect3DDevice9Proxy::GetLightEnable(DWORD Index,BOOL* pEnable){
	return(origIDirect3DDevice9->GetLightEnable(Index, pEnable));
}

HRESULT IDirect3DDevice9Proxy::SetClipPlane(DWORD Index,CONST float* pPlane){
	return(origIDirect3DDevice9->SetClipPlane(Index, pPlane));
}

HRESULT IDirect3DDevice9Proxy::GetClipPlane(DWORD Index,float* pPlane){
	return(origIDirect3DDevice9->GetClipPlane(Index,pPlane));
}

HRESULT IDirect3DDevice9Proxy::SetRenderState(D3DRENDERSTATETYPE State,DWORD Value){
	return(origIDirect3DDevice9->SetRenderState(State, Value));
}

HRESULT IDirect3DDevice9Proxy::GetRenderState(D3DRENDERSTATETYPE State,DWORD* pValue){
	return(origIDirect3DDevice9->GetRenderState(State, pValue));
}

HRESULT IDirect3DDevice9Proxy::CreateStateBlock(D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB){
	return(origIDirect3DDevice9->CreateStateBlock(Type,ppSB));
}

HRESULT IDirect3DDevice9Proxy::BeginStateBlock(void){
	return(origIDirect3DDevice9->BeginStateBlock());
}

HRESULT IDirect3DDevice9Proxy::EndStateBlock(IDirect3DStateBlock9** ppSB){
	return(origIDirect3DDevice9->EndStateBlock(ppSB));
}

HRESULT IDirect3DDevice9Proxy::SetClipStatus(CONST D3DCLIPSTATUS9* pClipStatus){
	return(origIDirect3DDevice9->SetClipStatus(pClipStatus));
}

HRESULT IDirect3DDevice9Proxy::GetClipStatus(D3DCLIPSTATUS9* pClipStatus){
	return(origIDirect3DDevice9->GetClipStatus( pClipStatus));
}

HRESULT IDirect3DDevice9Proxy::GetTexture(DWORD Stage,IDirect3DBaseTexture9** ppTexture){
	return(origIDirect3DDevice9->GetTexture(Stage,ppTexture));
}

HRESULT IDirect3DDevice9Proxy::SetTexture(DWORD Stage,IDirect3DBaseTexture9* pTexture)
{
	return(origIDirect3DDevice9->SetTexture(Stage,pTexture));
}

HRESULT IDirect3DDevice9Proxy::GetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue)
{
	return(origIDirect3DDevice9->GetTextureStageState(Stage,Type, pValue));
}

HRESULT IDirect3DDevice9Proxy::SetTextureStageState(DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value)
{
	return(origIDirect3DDevice9->SetTextureStageState(Stage,Type,Value));
}

HRESULT IDirect3DDevice9Proxy::GetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue)
{
	return(origIDirect3DDevice9->GetSamplerState(Sampler,Type, pValue));
}

HRESULT IDirect3DDevice9Proxy::SetSamplerState(DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value)
{
	return(origIDirect3DDevice9->SetSamplerState(Sampler,Type,Value));
}

HRESULT IDirect3DDevice9Proxy::ValidateDevice(DWORD* pNumPasses)
{
	return(origIDirect3DDevice9->ValidateDevice( pNumPasses));
}

HRESULT IDirect3DDevice9Proxy::SetPaletteEntries(UINT PaletteNumber,CONST PALETTEENTRY* pEntries)
{
	return(origIDirect3DDevice9->SetPaletteEntries(PaletteNumber, pEntries));
}

HRESULT IDirect3DDevice9Proxy::GetPaletteEntries(UINT PaletteNumber,PALETTEENTRY* pEntries)
{
	return(origIDirect3DDevice9->GetPaletteEntries(PaletteNumber, pEntries));
}

HRESULT IDirect3DDevice9Proxy::SetCurrentTexturePalette(UINT PaletteNumber)
{
	return(origIDirect3DDevice9->SetCurrentTexturePalette(PaletteNumber));
}

HRESULT IDirect3DDevice9Proxy::GetCurrentTexturePalette(UINT *PaletteNumber)
{
	return(origIDirect3DDevice9->GetCurrentTexturePalette(PaletteNumber));
}

HRESULT IDirect3DDevice9Proxy::SetScissorRect(CONST RECT* pRect)
{
	return(origIDirect3DDevice9->SetScissorRect( pRect));
}

HRESULT IDirect3DDevice9Proxy::GetScissorRect( RECT* pRect)
{
	return(origIDirect3DDevice9->GetScissorRect( pRect));
}

HRESULT IDirect3DDevice9Proxy::SetSoftwareVertexProcessing(BOOL bSoftware)
{
	return(origIDirect3DDevice9->SetSoftwareVertexProcessing(bSoftware));
}

BOOL    IDirect3DDevice9Proxy::GetSoftwareVertexProcessing(void)
{
	return(origIDirect3DDevice9->GetSoftwareVertexProcessing());
}

HRESULT IDirect3DDevice9Proxy::SetNPatchMode(float nSegments)
{
	return(origIDirect3DDevice9->SetNPatchMode(nSegments));
}

float   IDirect3DDevice9Proxy::GetNPatchMode(void)
{
	return(origIDirect3DDevice9->GetNPatchMode());
}

HRESULT IDirect3DDevice9Proxy::DrawPrimitive(D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount)
{
	return(origIDirect3DDevice9->DrawPrimitive(PrimitiveType,StartVertex,PrimitiveCount));
}

HRESULT IDirect3DDevice9Proxy::DrawIndexedPrimitive(D3DPRIMITIVETYPE PrimitiveType,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount)
{
	return(origIDirect3DDevice9->DrawIndexedPrimitive(PrimitiveType,BaseVertexIndex,MinVertexIndex,NumVertices,startIndex,primCount));
}

HRESULT IDirect3DDevice9Proxy::DrawPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
	return(origIDirect3DDevice9->DrawPrimitiveUP(PrimitiveType,PrimitiveCount,pVertexStreamZeroData,VertexStreamZeroStride));
}

HRESULT IDirect3DDevice9Proxy::DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride)
{
	return(origIDirect3DDevice9->DrawIndexedPrimitiveUP(PrimitiveType,MinVertexIndex,NumVertices,PrimitiveCount, pIndexData, IndexDataFormat, pVertexStreamZeroData,VertexStreamZeroStride));
}

HRESULT IDirect3DDevice9Proxy::ProcessVertices(UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags)
{
	return(origIDirect3DDevice9->ProcessVertices( SrcStartIndex, DestIndex, VertexCount, pDestBuffer, pVertexDecl, Flags));
}

HRESULT IDirect3DDevice9Proxy::CreateVertexDeclaration(CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl)
{
	return(origIDirect3DDevice9->CreateVertexDeclaration( pVertexElements,ppDecl));
}

HRESULT IDirect3DDevice9Proxy::SetVertexDeclaration(IDirect3DVertexDeclaration9* pDecl)
{
	return(origIDirect3DDevice9->SetVertexDeclaration(pDecl));
}

HRESULT IDirect3DDevice9Proxy::GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl)
{
	return(origIDirect3DDevice9->GetVertexDeclaration(ppDecl));
}

HRESULT IDirect3DDevice9Proxy::SetFVF(DWORD FVF)
{
	return(origIDirect3DDevice9->SetFVF(FVF));
}

HRESULT IDirect3DDevice9Proxy::GetFVF(DWORD* pFVF)
{
	return(origIDirect3DDevice9->GetFVF(pFVF));
}

HRESULT IDirect3DDevice9Proxy::CreateVertexShader(CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader)
{
	return(origIDirect3DDevice9->CreateVertexShader(pFunction,ppShader));
}

HRESULT IDirect3DDevice9Proxy::SetVertexShader(IDirect3DVertexShader9* pShader)
{
	return(origIDirect3DDevice9->SetVertexShader(pShader));
}

HRESULT IDirect3DDevice9Proxy::GetVertexShader(IDirect3DVertexShader9** ppShader)
{
	return(origIDirect3DDevice9->GetVertexShader(ppShader));
}

HRESULT IDirect3DDevice9Proxy::SetVertexShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
	return(origIDirect3DDevice9->SetVertexShaderConstantF(StartRegister,pConstantData, Vector4fCount));
}

HRESULT IDirect3DDevice9Proxy::GetVertexShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{
	return(origIDirect3DDevice9->GetVertexShaderConstantF(StartRegister,pConstantData,Vector4fCount));
}

HRESULT IDirect3DDevice9Proxy::SetVertexShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
	return(origIDirect3DDevice9->SetVertexShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT IDirect3DDevice9Proxy::GetVertexShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{
	return(origIDirect3DDevice9->GetVertexShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT IDirect3DDevice9Proxy::SetVertexShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{
	return(origIDirect3DDevice9->SetVertexShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT IDirect3DDevice9Proxy::GetVertexShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{
	return(origIDirect3DDevice9->GetVertexShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT IDirect3DDevice9Proxy::SetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride)
{
	return(origIDirect3DDevice9->SetStreamSource(StreamNumber,pStreamData,OffsetInBytes,Stride));
}

HRESULT IDirect3DDevice9Proxy::GetStreamSource(UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* OffsetInBytes,UINT* pStride)
{
	return(origIDirect3DDevice9->GetStreamSource(StreamNumber,ppStreamData,OffsetInBytes,pStride));
}

HRESULT IDirect3DDevice9Proxy::SetStreamSourceFreq(UINT StreamNumber,UINT Divider)
{
	return(origIDirect3DDevice9->SetStreamSourceFreq(StreamNumber,Divider));
}

HRESULT IDirect3DDevice9Proxy::GetStreamSourceFreq(UINT StreamNumber,UINT* Divider)
{
	return(origIDirect3DDevice9->GetStreamSourceFreq(StreamNumber,Divider));
}

HRESULT IDirect3DDevice9Proxy::SetIndices(IDirect3DIndexBuffer9* pIndexData)
{
	return(origIDirect3DDevice9->SetIndices(pIndexData));
}

HRESULT IDirect3DDevice9Proxy::GetIndices(IDirect3DIndexBuffer9** ppIndexData)
{
	return(origIDirect3DDevice9->GetIndices(ppIndexData));
}

HRESULT IDirect3DDevice9Proxy::CreatePixelShader(CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader)
{
	return(origIDirect3DDevice9->CreatePixelShader(pFunction,ppShader));
}

HRESULT IDirect3DDevice9Proxy::SetPixelShader(IDirect3DPixelShader9* pShader)
{
	return(origIDirect3DDevice9->SetPixelShader(pShader));
}

HRESULT IDirect3DDevice9Proxy::GetPixelShader(IDirect3DPixelShader9** ppShader)
{
	return(origIDirect3DDevice9->GetPixelShader(ppShader));
}

HRESULT IDirect3DDevice9Proxy::SetPixelShaderConstantF(UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount)
{
	return(origIDirect3DDevice9->SetPixelShaderConstantF(StartRegister,pConstantData,Vector4fCount));
}

HRESULT IDirect3DDevice9Proxy::GetPixelShaderConstantF(UINT StartRegister,float* pConstantData,UINT Vector4fCount)
{
	return(origIDirect3DDevice9->GetPixelShaderConstantF(StartRegister,pConstantData,Vector4fCount));
}

HRESULT IDirect3DDevice9Proxy::SetPixelShaderConstantI(UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount)
{
	return(origIDirect3DDevice9->SetPixelShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT IDirect3DDevice9Proxy::GetPixelShaderConstantI(UINT StartRegister,int* pConstantData,UINT Vector4iCount)
{
	return(origIDirect3DDevice9->GetPixelShaderConstantI(StartRegister,pConstantData,Vector4iCount));
}

HRESULT IDirect3DDevice9Proxy::SetPixelShaderConstantB(UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount)
{
	return(origIDirect3DDevice9->SetPixelShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT IDirect3DDevice9Proxy::GetPixelShaderConstantB(UINT StartRegister,BOOL* pConstantData,UINT BoolCount)
{
	return(origIDirect3DDevice9->GetPixelShaderConstantB(StartRegister,pConstantData,BoolCount));
}

HRESULT IDirect3DDevice9Proxy::DrawRectPatch(UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo)
{
	return(origIDirect3DDevice9->DrawRectPatch(Handle,pNumSegs, pRectPatchInfo));
}

HRESULT IDirect3DDevice9Proxy::DrawTriPatch(UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo)
{
	return(origIDirect3DDevice9->DrawTriPatch(Handle, pNumSegs, pTriPatchInfo));
}

HRESULT IDirect3DDevice9Proxy::DeletePatch(UINT Handle)
{
	return(origIDirect3DDevice9->DeletePatch(Handle));
}

HRESULT IDirect3DDevice9Proxy::CreateQuery(D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery)
{
	return(origIDirect3DDevice9->CreateQuery(Type,ppQuery));
}

void IDirect3DDevice9Proxy::DisplayText(D3DCOLOR fontColor, int x, int y, int width, int height, string text)
{
	if(!fontCreated)
	{
		D3DXCreateFont( origIDirect3DDevice9, 20, 0, FW_BOLD, 0, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Arial"), &m_font );
		fontCreated = true;
	}
	// Create a rectangle to indicate where on the screen it should be drawn
	RECT rct;
	rct.left=x;
	rct.right=x+width;
	rct.top=y;
	rct.bottom=y+height;

	// Draw some text 
	m_font->DrawText(NULL, text.c_str(), -1, &rct, 0, fontColor );
}

void IDirect3DDevice9Proxy::DisplayData(int team, int player, int x, int y)
{
	Summoner s = ((team == 1) ? LolKnow::teamOne : LolKnow::teamTwo).at(player);

	//Champion that they're playing
	//DisplayText(D3DCOLOR_ARGB(255,255,255,255), x, y, 220, 25, s.champion);

	D3DCOLOR color = D3DCOLOR_ARGB(255,255,255,255);

	D3DCOLOR nameColor;
	//Changes color for queue groups, applies to names only
	switch(s.queueGroup) {
	case 0: nameColor = D3DCOLOR_ARGB(255,255,255,255); break;
	case 1: nameColor = D3DCOLOR_ARGB(255,255,0,0); break;
	case 2: nameColor = D3DCOLOR_ARGB(255,0,255,0); break;
	case 3: nameColor = D3DCOLOR_ARGB(255,0,0,255); break;
	case 4: nameColor = D3DCOLOR_ARGB(255,0,255,255); break;
	default: nameColor = D3DCOLOR_ARGB(255,255,255,255); break;
	}
	//Solo Queue Rank and tier
	DisplayText(nameColor, x, y, 220, 25, s.name); //TODO: color based on their rank

	D3DCOLOR rankColor;
	//Changes color for queue groups, applies to names only
	if(s.tier == "BRONZE")
	{
		rankColor = D3DCOLOR_ARGB(255,108,84,30);
	}
	else if(s.tier == "SILVER")
	{
		rankColor = D3DCOLOR_ARGB(255,192,192,192);
	}
	else if(s.tier == "GOLD")
	{
		rankColor = D3DCOLOR_ARGB(255,255,215,0);
	}
	else if(s.tier == "PLATINUM")
	{
		rankColor = D3DCOLOR_ARGB(255,229,228,226);
	}
	else if(s.tier == "DIAMOND")
	{
		rankColor = D3DCOLOR_ARGB(255,185,242,255);
	}
	else if(s.tier == "CHALLENGER")
	{
		rankColor = D3DCOLOR_ARGB(255,255,255,0);
	}
	else
	{
		rankColor = D3DCOLOR_ARGB(255,255,255,255);
	}

	//Solo Queue Rank and tier
	DisplayText(rankColor, x, y+25, 220, 25, s.tier + " " + s.rank);

	//General ranked KDA
	stringstream ss;
	if(s.kills != -1 || s.deaths != -1 || s.assists != -1)
	{
		ss << s.kills << "/" << s.deaths << "/" << s.assists;
		DisplayText(color, x, y+50, 220, 25, ss.str()); //TODO: color kills green, deaths red, assists blue
	}

	//General Ranked W/L
	if(s.wins != -1 || s.losses != -1)
	{
		ss = stringstream();
		ss << s.wins << "W/" << s.losses << "L";
		DisplayText(color, x, y+75, 220, 25, ss.str()); //TODO: color wins green, loses red
	}

	//Champion specific KDA and games played
	if(s.champKills != -1 || s.champDeaths != -1 || s.champAssists != -1 || s.champPlayed != -1)
	{
		ss = stringstream();
		ss << s.champKills << "/" << s.champDeaths << "/" << s.champAssists << " - " << s.champPlayed << "P";
		DisplayText(color, x, y+100, 220, 25, ss.str()); //TODO: color kills green, deaths red, assists blue
	}
}