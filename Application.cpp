#include "Engine.h"

using namespace std;

Application::Application( HINSTANCE hInstance ) :
	mhInstance( hInstance ),
	mWinClassName( _T( "EngineClassName" ) ),
	mhWnd( 0 ),
	mFPS( 0 ),
	mbEnable( true ),
	mpWorld( 0 )
{
}

Application::~Application()
{
	unloadTextures();

	UnregisterClass( mWinClassName.c_str(), mhInstance );
	s_release( mpDirectStruct->pBackBuffer );
	s_release( mpDirectStruct->pFontLocations );
	s_release( mpDirectStruct->pFontItems );
	s_release( mpDirectStruct->pSprite );
	s_release( mpDirectStruct->pDevice );
	s_release( mpDirectStruct->pDirect );
}


bool Application::init()
{
	int result = createWindow() || createDirect() || createInterface();
	mpWorld->loadWorldObjects();
	return result ? true : false;
}


void Application::setFPSValue( UINT value )
{
	mFPS = value;
}

void Application::setStage( UINT id )
{
	mpCurrentStage = mStages[ id ];
}


LRESULT Application::inputProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	POINT cursor;
	GetCursorPos( &cursor );

	if( mpCurrentStage )
	{
		Sensor* pTargetSensor = 0;
		for( auto elem = mpCurrentStage->interfaceObjects.rbegin(); elem != mpCurrentStage->interfaceObjects.rend(); elem++ )
		{
			if( ( *elem )->ptInSensor( cursor ) )
			{
				pTargetSensor = *elem;
				break;
			}
		}
		setTargetSensor( pTargetSensor );
	}

	switch( message )
	{
		case WM_KEYDOWN:
		{
			if( wParam == 27 && mpCurrentStage && mpCurrentStage->id == 3 )
			{
				setStage( 4 );
			}
		}
		break;
		case WM_LBUTTONDOWN:
		{
			if( mpTargetSensor )
			{
				mpTargetSensor->onEvent( hWnd, message, wParam, lParam );
			}
			else
			{
				POINT displaySize = { mpDirectStruct->width, mpDirectStruct->height };
				POINTFLOAT target = viewToWorld( displaySize, mpWorld->getCameraPosition(), cursor );
				mCharacter.moveToPosition( target.x, target.y );
			}
		}
		break;
		case WM_BUTTONCLICK:
		{
			if( wParam == 1003 )
			{
				SendMessage( hWnd, WM_DESTROY, 0, 0 );
			}
			else if( wParam == 1001 )
			{
				setStage( 1 );
			}
			else if( wParam == 2001 )
			{
				setStage( 2 );
			}
			else if( wParam == 2002 )
			{
				setStage( 0 );
			}
			else if( wParam == 5002 )
			{
				setStage( 3 );
			}
			else if( wParam == 5003 )
			{
				mpWorld->clear();
				setStage( 0 );
			}
		}
		break;
		default:
		{
			if( mpTargetSensor )
				mpTargetSensor->onEvent( hWnd, message, wParam, lParam );
		}
		break;
	}
	return 0;
}

void Application::process( ULONG deltaTime )
{
	if( !mbEnable )
		return;

	if( mpCurrentStage && mpCurrentStage->id == 2 )
	{
		StatusBar* pstBar = static_cast<StatusBar*>( mpCurrentStage->interfaceObjects[ 0 ] );
		if( pstBar->getPercentValue() == 1.0f )
		{
			loadWorld( _T( "Data/Level1.txt" ) );
			setStage( 3 );
		}
		else
		{
			pstBar->setPercentValue( pstBar->getPercentValue() + (float) deltaTime / 1500.0f );
		}
	}

	mpWorld->setCameraPosition( mCharacter.getPosition() );
	mpWorld->process( deltaTime );

}


void Application::render( ULONG deltaTime )
{
	if( testDevice() )
	{
		return;
	}

	LPDIRECT3DDEVICE9 pDevice = mpDirectStruct->pDevice;

	pDevice->Clear( 0, NULL, D3DCLEAR_TARGET, 0xff9999aa, 1.0f, 0 );

	if( SUCCEEDED( pDevice->BeginScene() ) )
	{
		pDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, true );
		pDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
		pDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

		if( mpCurrentStage )
		{
			auto pBackground = getBackground( mpCurrentStage->backgroundId );
			if( pBackground )
			{
				pBackground->draw( mpDirectStruct );
			}

			if( mpCurrentStage->bDrawingWolrd )
			{
				if( mpWorld )
					mpWorld->draw( deltaTime );
			}

			if( mpCurrentStage->id == 3 )
			{
				RECT rect = { 0, 0, mpDirectStruct->width, 70 };
				mpDirectStruct->pFontLocations->DrawText( NULL, mpWorld->getCurrenLocationName(), -1, &rect, DT_CENTER | DT_TOP, 0xffffffff );
			}

			for( auto elem = mpCurrentStage->interfaceObjects.begin(); elem != mpCurrentStage->interfaceObjects.end(); elem++ )
			{
				( *elem )->draw( deltaTime );
			}
		}
	}

	pDevice->EndScene();
	pDevice->Present( 0, 0, 0, 0 );

}


Texture* Application::getTexture( UINT id )
{
	return mTextures[ id ];
}

Background* Application::getBackground( UINT id )
{
	return mBackgrounds[ id ];
}


int Application::createWindow()
{
	WNDCLASS wc;

	wc.hInstance = mhInstance;
	wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	wc.hCursor = LoadCursor( NULL, IDC_ARROW );
	wc.hbrBackground = (HBRUSH) ( CreateSolidBrush( RGB( 0, 0, 0 ) ) );
	wc.lpfnWndProc = inputWndProc;
	wc.lpszClassName = mWinClassName.c_str();
	wc.lpszMenuName = NULL;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.style = CS_HREDRAW | CS_VREDRAW;

	ATOM WNDClass = RegisterClass( &wc );
	if( !WNDClass )
		return 1;

	//Создание экземплра окна
	mhWnd = CreateWindowEx( WS_EX_TOPMOST, mWinClassName.c_str(), _T( "CRYS" ), WS_POPUP | WS_VISIBLE, 0, 0, GetSystemMetrics( SM_CXSCREEN ), GetSystemMetrics( SM_CYSCREEN ), NULL, NULL, mhInstance, NULL );

	if( !mhWnd )
		return 2;

	ShowWindow( mhWnd, 1 );
	UpdateWindow( mhWnd );

	return 0;
}

int Application::createDirect()
{
	mpDirectStruct = new Direct;
	ZeroMemory( mpDirectStruct, sizeof( Direct ) );

	if( !( mpDirectStruct->pDirect = Direct3DCreate9( D3D_SDK_VERSION ) ) )
		return 1;

	if( FAILED( mpDirectStruct->pDirect->GetAdapterDisplayMode( D3DADAPTER_DEFAULT, &mpDirectStruct->dispalyMode ) ) )
		return 2;

	mpDirectStruct->width = GetSystemMetrics( SM_CXSCREEN );
	mpDirectStruct->height = GetSystemMetrics( SM_CYSCREEN );

	mpDirectStruct->presentParameters.BackBufferWidth = mpDirectStruct->width;
	mpDirectStruct->presentParameters.BackBufferHeight = mpDirectStruct->height;
	mpDirectStruct->presentParameters.BackBufferCount = 2;
	mpDirectStruct->presentParameters.BackBufferFormat = mpDirectStruct->dispalyMode.Format;
	mpDirectStruct->presentParameters.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	mpDirectStruct->presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
	mpDirectStruct->presentParameters.Windowed = false;
	mpDirectStruct->presentParameters.MultiSampleType = D3DMULTISAMPLE_NONE;
	mpDirectStruct->presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	mpDirectStruct->presentParameters.EnableAutoDepthStencil = true;
	mpDirectStruct->presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	mpDirectStruct->presentParameters.hDeviceWindow = mhWnd;

	if( FAILED( mpDirectStruct->pDirect->CreateDevice( 
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		mhWnd,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&mpDirectStruct->presentParameters,
		&mpDirectStruct->pDevice ) ) )
		return 3;

	if( FAILED( D3DXCreateSprite( mpDirectStruct->pDevice, &mpDirectStruct->pSprite ) ) )
		return 4;

	if( FAILED( D3DXCreateFont( mpDirectStruct->pDevice, 14, 6, 0, NULL, NULL, NULL, NULL, NULL, DEFAULT_PITCH | FF_MODERN, _T( "Arial" ), &mpDirectStruct->pFontItems ) ) )
		return 5;

	if( FAILED( D3DXCreateFont( mpDirectStruct->pDevice, 32, 10, 0, NULL, NULL, NULL, NULL, NULL, DEFAULT_PITCH | FF_MODERN, _T( "Arial" ), &mpDirectStruct->pFontLocations ) ) )
		return 6;

	if( FAILED( D3DXCreateFont( mpDirectStruct->pDevice, 16, 10, 0, NULL, NULL, NULL, NULL, NULL, DEFAULT_PITCH | FF_MODERN, _T( "Arial" ), &mpDirectStruct->pFontInterface ) ) )
		return 7;

	if( FAILED( mpDirectStruct->pDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &mpDirectStruct->pBackBuffer ) ) )
		return 8;

	return 0;

}

int Application::createInterface()
{
	loadTextures( _T( "Data/Textures.txt" ) );

	//MainMenu
	AppStage* stage = new AppStage;
	stage->id = 0;
	stage->backgroundId = 1;
	stage->bDrawingWolrd = false;
	Button* button = new Button( mpDirectStruct, 1001, mpDirectStruct->width - 400, 200, 300, 75, getTexture( 1 ) );
	stage->interfaceObjects.push_back( button );
	button = new Button( mpDirectStruct, 1002, mpDirectStruct->width - 400, 350, 300, 75, getTexture( 2 ) );
	button->setEnabled( false );
	stage->interfaceObjects.push_back( button );
	button = new Button( mpDirectStruct, 1003, mpDirectStruct->width - 400, 500, 300, 75, getTexture( 3 ) );
	stage->interfaceObjects.push_back( button );
	mStages.emplace( stage->id, stage );

	//StartMenu
	stage = new AppStage;
	stage->id = 1;
	stage->backgroundId = 1;
	stage->bDrawingWolrd = false;
	button = new Button( mpDirectStruct, 2001, mpDirectStruct->width - 400, mpDirectStruct->height - 150, 300, 75, getTexture( 4 ) );
	stage->interfaceObjects.push_back( button );
	button = new Button( mpDirectStruct, 2002, 100, mpDirectStruct->height - 150, 300, 75, getTexture( 5 ) );
	stage->interfaceObjects.push_back( button );
	Label* label = new Label( mpDirectStruct, 2003, _T( "**Engine V1**\nТестовый движок\nИзображения в изометрии\nУправление щелчком мыши\nEsc - выход" ), 100, 100, 700, 700, 0xff000000, getTexture( 6 ) );
	label->setFontStyle( DT_CENTER | DT_VCENTER );
	stage->interfaceObjects.push_back( label );
	mStages.emplace( stage->id, stage );

	//LoadMenu
	stage = new AppStage;
	stage->id = 2;
	stage->backgroundId = 2;
	stage->bDrawingWolrd = false;
	StatusBar* stBar = new StatusBar( mpDirectStruct, 3001, mpDirectStruct->width / 2 - 300, mpDirectStruct->height / 2 - 50, 600, 100, getTexture( 7 ) );
	stage->interfaceObjects.push_back( stBar );
	mStages.emplace( stage->id, stage );

	//Game
	stage = new AppStage;
	stage->id = 3;
	stage->backgroundId = 3;
	stage->bDrawingWolrd = true;
	label = new Label( mpDirectStruct, 3001, _T( "**Demo Engine V1**\nДвижение - ЛКМ\nEsc - выход" ), 0, 0, 250, 100, 0xff000000, getTexture( 8 ) );
	label->setFontColor( 0xffffffff );
	stage->interfaceObjects.push_back( label );
	mStages.emplace( stage->id, stage );

	//GameMenu
	stage = new AppStage;
	stage->id = 4;
	stage->backgroundId = 3;
	stage->bDrawingWolrd = true;
	label = new Label( mpDirectStruct, 5001, _T( "" ), 0, 0, mpDirectStruct->width, mpDirectStruct->height, 0xff000000, getTexture( 8 ) );
	stage->interfaceObjects.push_back( label );
	button = new Button( mpDirectStruct, 5002, mpDirectStruct->width / 2 - 150, mpDirectStruct->height / 2 - 150, 300, 75, getTexture( 9 ) );
	stage->interfaceObjects.push_back( button );
	button = new Button( mpDirectStruct, 5003, mpDirectStruct->width / 2 - 150, mpDirectStruct->height / 2 + 75, 300, 75, getTexture( 10 ) );
	stage->interfaceObjects.push_back( button );
	mStages.emplace( stage->id, stage );

	mpWorld = new World( mpDirectStruct );
	
	setStage( 0 );

	return 0;
}


int Application::testDevice()
{
	HRESULT result = mpDirectStruct->pDevice->TestCooperativeLevel();

	if( result == D3D_OK )
		return 0;

	mbEnable = false;

	if( result == D3DERR_DEVICENOTRESET )
		return reset();

	return 2;
}

int Application::reset()
{
	s_release( mpDirectStruct->pBackBuffer );
	mpDirectStruct->pFontItems->OnLostDevice();
	mpDirectStruct->pFontLocations->OnLostDevice();
	mpDirectStruct->pFontInterface->OnLostDevice();
	mpDirectStruct->pSprite->OnLostDevice();

	if( FAILED( mpDirectStruct->pDevice->Reset( &mpDirectStruct->presentParameters ) ) )
		return 1;

	mpDirectStruct->pSprite->OnResetDevice();
	mpDirectStruct->pFontInterface->OnResetDevice();
	mpDirectStruct->pFontLocations->OnResetDevice();
	mpDirectStruct->pFontItems->OnResetDevice();
	mpDirectStruct->pDevice->GetBackBuffer( 0, 0, D3DBACKBUFFER_TYPE_MONO, &mpDirectStruct->pBackBuffer );

	mbEnable = true;

	return 0;
}


Texture* Application::loadTextureFromFile( LPCTSTR pFileName, UINT id )
{
	Texture* pResultTexture = createTextureFromFile( mpDirectStruct, pFileName );

	mTextures.emplace( id, pResultTexture );

	return pResultTexture;
}

Background* Application::loadBackgroundFromFile( LPCTSTR pFileName, ULONG displayWidth, ULONG displayHeight, UINT id )
{
	Background* pResultBackground = createBackgroundFromFile( mpDirectStruct, pFileName, displayWidth, displayHeight );

	mBackgrounds.emplace( id, pResultBackground );

	return pResultBackground;
}


int Application::loadTextures( LPCTSTR pSourceFileName )
{
	int result = 0;

	convert_type* converter = new convert_type();
	locale utf8( locale::empty(), converter );

	tifstream file( pSourceFileName );
	file.imbue( utf8 );

	tstring key;

	while( !file.eof() )
	{
		file >> key;
		if( key == _T( "<Textures>" ) )
		{
			file >> key;
			while( key != _T( "</Textures>" ) )
			{
				if( key == _T( "<bg>" ) )
				{
					tstring textureFileName;
					file >> textureFileName;
					UINT id;
					file >> id;
					file >> key;

					if( !( loadBackgroundFromFile( textureFileName.c_str(), mpDirectStruct->width, mpDirectStruct->height, id ) ) )
						result++;

				}
				else if( key == _T( "<it>" ) )
				{
					tstring textureFileName;
					file >> textureFileName;
					UINT id;
					file >> id;
					file >> key;

					if( !( loadTextureFromFile( textureFileName.c_str(), id ) ) )
						result++;
				}
				
			}
		}		
	}

	file.close();
	return result;
}


void Application::unloadTextures()
{
	for( auto elem = mTextures.begin(); elem != mTextures.end(); elem++ )
	{
		s_delete( elem->second );
	}
	mTextures.clear();

	for( auto elem = mBackgrounds.begin(); elem != mBackgrounds.end(); elem++ )
	{
		s_delete( elem->second );
	}
	mBackgrounds.clear();
}


void Application::loadWorld( LPCTSTR pFileName )
{
	mpWorld->loadWorld( pFileName );
	mCharacter.setManagedObject( mpWorld->spawnCharacter( { 2.0f, 2.0f } ) );
}

void Application::unloadWold()
{
	mpWorld->clear();
}


void Application::setTargetSensor( Sensor* pSensor )
{
	if( mpTargetSensor == pSensor )
		return;

	if( mpTargetSensor )
		mpTargetSensor->onMouseLeave();

	if( pSensor )
		pSensor->onMouseEnter();

	mpTargetSensor = pSensor;
}