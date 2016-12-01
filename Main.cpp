#include "Engine.h"

using namespace std;

Application* pGame = 0;

int APIENTRY WinMain( HINSTANCE hInstance, HINSTANCE, LPSTR, int )
{
	MSG msg;

	if( !( pGame = createApplication( hInstance ) ) )
		return 1;

	DWORD lastTime = timeGetTime();
	DWORD currentTime = 0;
	DWORD deltaTime = 0;
	DWORD epsTime = 0;
	UINT frame = 0;

	PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE );
	while( msg.message != WM_QUIT )
	{
		currentTime = timeGetTime();
		deltaTime = currentTime - lastTime;

		if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
		}
		else
		{
			pGame->process( deltaTime );
			pGame->render( deltaTime );

			epsTime += deltaTime;
			if( epsTime >= 1000 )
			{
				epsTime %= 1000;
				pGame->setFPSValue( frame );
				frame = 0;
			}

			frame++;
			lastTime = currentTime;
		}

	}
	
	s_delete( pGame );

	return 0;
}

Texture* createTextureFromFile( Direct* pDirect, LPCTSTR pFileName )
{
	Texture* pResultTexture = new Texture;
	ZeroMemory( pResultTexture, sizeof( Texture ) );

	if( FAILED( D3DXCreateTextureFromFile( pDirect->pDevice, pFileName, &pResultTexture->pDirectTexture ) ) )
	{
		s_delete( pResultTexture );
		return 0;
	}

	if( FAILED( D3DXGetImageInfoFromFile( pFileName, &pResultTexture->textureInfo ) ) )
	{
		s_delete( pResultTexture );
		return 0;
	}

	return pResultTexture;
}

Background* createBackgroundFromFile( Direct* pDirect, LPCTSTR pFileName, ULONG displayWidth, ULONG displayHeight )
{
	Background* pResultBackground = new Background;

	if( FAILED( pDirect->pDevice->CreateOffscreenPlainSurface(
		displayWidth,
		displayHeight,
		pDirect->dispalyMode.Format,
		D3DPOOL_SYSTEMMEM,
		&pResultBackground->pBackgroundSurface,
		0 ) ) )
	{
		s_delete( pResultBackground );
		return 0;
	}

	if( FAILED( D3DXLoadSurfaceFromFile( pResultBackground->pBackgroundSurface, NULL, NULL, pFileName, NULL, D3DX_FILTER_BOX, 0, NULL ) ) )
	{
		s_delete( pResultBackground );
		return 0;
	}

	return pResultBackground;
}

Application* createApplication( HINSTANCE hInstance )
{
	Application* pApp = new Application( hInstance );

	if( pApp->init() )
		s_delete( pApp );

	return pApp;
}

LRESULT APIENTRY inputWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	LRESULT result = 0;

	switch( message )
	{
		case WM_DESTROY:
			PostQuitMessage( 0 );
			break;
		default:
			if( !pGame || !( result = pGame->inputProc( hWnd, message, wParam, lParam ) ) )
				result = DefWindowProc( hWnd, message, wParam, lParam );
			break;
	}
	
	return result;
}

POINTFLOAT viewToWorld( POINT displaySize, POINTFLOAT camera, POINT pt )
{
	LONG viewRelX = pt.x - displaySize.x / 2;
	LONG viewRelY = pt.y - displaySize.y / 2;

	POINTFLOAT result;

	result.x = camera.x + (float) viewRelX / (float) VIEW_FIELD_WIDTH + (float) viewRelY / (float) VIEW_FIELD_HEIGHT;
	result.y = camera.y - (float) viewRelX / (float) VIEW_FIELD_WIDTH + (float) viewRelY / (float) VIEW_FIELD_HEIGHT;

	return result;
}

POINT worldToView( POINT displaySize, POINTFLOAT camera, POINTFLOAT ptf )
{
	float worldRelX = ptf.x - camera.x;
	float worldRelY = ptf.y - camera.y;

	POINT result;

	result.x = displaySize.x / 2 + worldRelX * (float) VIEW_FIELD_WIDTH / 2.0f - worldRelY * (float) VIEW_FIELD_WIDTH / 2.0f;
	result.y = displaySize.y / 2 + worldRelX * (float) VIEW_FIELD_HEIGHT / 2.0f + worldRelY * (float) VIEW_FIELD_HEIGHT / 2.0f;

	return result;
}

float northAngle( POINTFLOAT vector )
{
	return D3DX_PI + atan2( -vector.x, vector.y );
}

bool sortWObjects( WObject* pWObj1, WObject* pWObj2 )
{
	if( !pWObj1 )
		return false;

	if( !pWObj2 )
		return true;

	POINTFLOAT pf1 = pWObj1->getPosition();
	POINTFLOAT pf2 = pWObj2->getPosition();

	if( pf1.x + pf1.y < pf2.x + pf2.y )
		return true;
	else if( ( pf1.x + pf1.y == pf2.x + pf2.y ) && ( pf1.x - pf1.y < pf2.x - pf2.y ) )
		return true;

	return false;
}