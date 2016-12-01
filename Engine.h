#pragma once

#include <Windows.h>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <tchar.h>
#include <locale>
#include <fstream>
#include <codecvt>

#include <d3d9.h>
#include <d3dx9.h>

#pragma comment( lib, "d3d9.lib" )
#pragma comment( lib, "d3dx9.lib" )
#pragma comment ( lib, "winmm.lib" )

#define s_delete(p) { if(p) { delete(p); (p) = 0; } }
#define s_release(p) { if(p) { (p)->Release(); (p) = 0; } }
#define s_deleteA(p) { if(p) { delete[](p); (p) = 0; } }

typedef std::wstring tstring;
#define to_tstring( x )	std::to_wstring( x )
typedef std::wifstream tifstream;
typedef std::codecvt_utf8<wchar_t> convert_type;

#define VIEW_MAP_SIZE	41

#define VIEW_FIELD_WIDTH	200
#define VIEW_FIELD_HEIGHT	100

#define WM_BUTTONCLICK	WM_USER + 100

//Структура данных для отображения графики
struct Direct
{
	LPDIRECT3D9 pDirect;
	LPDIRECT3DDEVICE9 pDevice;
	D3DPRESENT_PARAMETERS presentParameters;
	D3DDISPLAYMODE dispalyMode;
	LPD3DXSPRITE pSprite;
	LPDIRECT3DSURFACE9 pBackBuffer;
	ULONG width, height;	//Текущее разрешение экрана
	LPD3DXFONT pFontItems; //Шрифт подписи предмета
	LPD3DXFONT pFontLocations; //Шрифт подписи локации
	LPD3DXFONT pFontInterface; //Шрифт интерфейса
};

//Структура информации о текстуре
struct Texture
{
	~Texture()
	{
		s_release( pDirectTexture );
	}

	LPDIRECT3DTEXTURE9 pDirectTexture;
	D3DXIMAGE_INFO textureInfo;
};

Texture* createTextureFromFile( Direct* pDirect, LPCTSTR pFileName );

POINTFLOAT viewToWorld( POINT displaySize, POINTFLOAT camera, POINT pt );

POINT worldToView( POINT displaySize, POINTFLOAT camera, POINTFLOAT ptf );

float northAngle( POINTFLOAT vector );

//Структура плоского объекта
struct Graf
{
	Graf() :
		pTexture( 0 ),
		textureRect( { 0 } ),
		textureColor( 0xffffffff ),
		maskRect( { 0 } ),
		maskColor( 0xffffffff )
	{

	}

	Texture* pTexture;
	RECT textureRect;
	DWORD textureColor;
	RECT maskRect;
	DWORD maskColor;

	int draw( Direct* pDirect, D3DXVECTOR3 position, ULONG width, ULONG height );
};

//Структура фонового изображения
struct Background
{
	~Background()
	{
		s_release( pBackgroundSurface );
	}

	LPDIRECT3DSURFACE9 pBackgroundSurface;

	int draw( Direct* pDirect );
};

Background* createBackgroundFromFile( Direct* pDirect, LPCTSTR pFileName, ULONG displayWidth, ULONG displayHeight );

//Структура площадки
struct Field
{
	~Field()
	{
		s_delete( pTexture );
	}

	Texture* pTexture;
	Graf graf;
	float move;
};

//Структура точки спауна
struct SpawnPoint
{
	UINT objectId;
	POINTFLOAT position;
	UINT taskId;
};

//Структура локации
struct Location
{
	UINT** map;
	tstring name;
	ULONG startX;
	ULONG startY;
	ULONG width;
	ULONG height;
	std::vector< SpawnPoint > spawnPoints;
};

class Animation
{
	public:
		Animation();
		Animation( LPCTSTR pName, Texture* pTexture, UINT frameSize, UINT orientationType, UINT topOffset );

		void addState( UINT animKey );

		void setCurrentState( UINT id );
		void setOrientation( float x, float y );
		void setOrientation( POINTFLOAT ptf );
		void setOrientation( float angle );

		Graf* getCurrentState();
		UINT getStateCount();
		UINT getFrameSize();
		LPCTSTR getName();

		void start();
		void process( float x, float y );
		void nextState();

	private:
		tstring mName;
		UINT mFrameSize;
		UINT mOrientationType;
		UINT mTopOffset;
		Graf mGraf;
		UINT mCurrentState;
		std::vector< UINT > mStates;
};

class World;
class Task;

//Класс мирового объекта
class WObject
{
	public:
		WObject( Direct* pDirect, World* pWorld );
		WObject( const WObject& root );
		WObject& operator=( const WObject& root );
		~WObject();

		void setName( LPCTSTR pName );
		void setBaseRatio( float baseRatio );
		void setSpeed( float speed );
		void setAnimation( LPCTSTR pKey, const Animation& anim );
		void setPosition( float x, float y );
		void setPositionRel( float x, float y );
		void setColor( DWORD color );
		void setTask( Task* pTask );

		void startAnimation( LPCTSTR pKey );

		LPCTSTR getName();
		POINTFLOAT getPosition();
		float getBaseRatio();
		float getSpeed();
		bool isMoving();

		void process( ULONG deltaTime );
		void draw( POINTFLOAT camera );

		void moveTo( POINTFLOAT vector );
		void moveToPosition( POINTFLOAT target );

	private:
		Direct* mpDirect;
		World* mpWorld;
		tstring mName;
		POINTFLOAT mPosition;
		float mBaseRatio;
		float mSpeed;
		Animation* mCurrentAnimation;
		DWORD mColor;
		std::map < tstring, Animation > mAnimations;
		ULONG mTime;
		float mOrientationAngle;
		POINTFLOAT mMoveVector;
		Task* mpTask;
};

bool sortWObjects( WObject* pWObj1, WObject* pWObj2 );

//Задания для NPC
class Task
{
	public:
		Task():
			mpWObject( 0 ),
			mTime( 0 )
		{

		}

		void setObject( WObject* pWObject )
		{
			mpWObject = pWObject;
		}

		virtual void process( ULONG deltaTime ) = 0;
		virtual Task* clone() = 0;

	protected:
		WObject* mpWObject;
		ULONG mTime;
};

class MoveTask : public Task
{
	public:
		MoveTask( POINTFLOAT center );

		void setCenter( POINTFLOAT center );

		void process( ULONG deltaTime );

		MoveTask* clone();

	private:
		POINTFLOAT mCenter;
};

//Игровой уровень
class World
{
	public:
		World( Direct* pDirect );
		~World();

		void process( ULONG deltaTime );
		void draw( ULONG deltaTime );

		void loadWorldObjects();
		void loadWorld( LPCTSTR pFileName );
		void initMap();
		void spawnAll();
		WObject* spawn( UINT objectId, POINTFLOAT position, UINT taskId );
		WObject* spawn( SpawnPoint sPoint );

		WObject* spawnCharacter( POINTFLOAT position );

		void clear();

		void setCameraPosition( POINTFLOAT pf );
		void setCameraPositionRel( POINTFLOAT pf );

		Field* getMapField( POINT coords );
		Field* getMapField( POINTFLOAT coords );
		Location* getMapLocation( POINT coords );
		Location* getMapLocation( POINTFLOAT coords );
		POINTFLOAT getCameraPosition();
		LPCTSTR getCurrenLocationName();

		void testMoving( POINTFLOAT position, float size, POINTFLOAT& rel );
		bool testVisible( POINTFLOAT position );

	private:
		struct MapField
		{
			Field* pField;
			Location* pLocation;
		};

		Direct* mpDirect;
		std::map< UINT, Field* > mMemFields;
		std::map< UINT, Location* > mMemLocations;
		std::map< UINT, WObject* > mMemWObjects;
		std::map< tstring, Texture* > mMemTextures;
		std::vector< SpawnPoint > mSpawnPoints;
		SpawnPoint mCharacterSpawnPoint;
		std::vector< WObject* > mWorldObjects;
		std::vector< WObject* > mVisibleWObjects;
		ULONG mMapWidth;
		ULONG mMapHeight;
		MapField** mMap;
		POINTFLOAT mCameraPosition;
};

//Игровой персонаж
class Character
{
	public:
		Character();
		~Character();

		void moveToPosition( float x, float y );
		void setPosition( float x, float y );
		void setManagedObject( WObject* pObject );

		POINTFLOAT getPosition();

		void test();

	private:
		WObject* mpManagedObject;
};

class Sensor
{
	public:
		Sensor( Direct* pDirect, ULONG id );
		Sensor( Direct* pDirect, ULONG id, LONG x, LONG y, LONG width, LONG height, Texture* pTexture = 0 );

		void setTexture( Texture* pTexture );
		void setPosition( LONG x, LONG y );
		void setSize( LONG width, LONG height );
		void setWorkRect( RECT rect );
		void setVisible( bool bVisible );
		void setEnabled( bool bEnabled );

		bool isVisible();
		bool isEnable();

		bool ptInSensor( POINT pt );

		virtual void onMouseEnter();
		virtual void onMouseLeave();
		virtual void onEvent( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

		virtual void draw( ULONG deltaTime );

	protected:
		Direct* mpDirect;
		ULONG mId;
		Graf mGraf;
		RECT mWorkRect;
		bool mbVisible;
		bool mbEnabled;
};

class Label : public Sensor
{
	public:
		Label( Direct* pDirect, ULONG id, LPCTSTR pText, DWORD fontColor = 0xff000000 );
		Label( Direct* pDirect, ULONG id, LPCTSTR pText, LONG x, LONG y, LONG width, LONG height, DWORD fontColor = 0xff000000, Texture* pTexture = 0 );

		void setText( LPCTSTR pText );
		void setFontColor( DWORD color );
		void setFontStyle( DWORD style );

		LPCTSTR getText();
		DWORD getFontColor();

		void draw( ULONG deltaTime );

	protected:
		tstring mText;
		DWORD mFontColor;
		DWORD mFontStyle;
};

class Button : public Sensor
{
	public:
		Button( Direct* pDirect, ULONG id );
		Button( Direct* pDirect, ULONG id, LONG x, LONG y, LONG width, LONG height, Texture* pTexture = 0 );

		void onMouseEnter();
		void onMouseLeave();
		void onEvent( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

		void draw( ULONG deltaTime );

	protected:
		bool mbActive;
		bool mbPushed;
};

class StatusBar : public Sensor
{
	public:
		StatusBar( Direct* pDirect, ULONG id );
		StatusBar( Direct* pDirect, ULONG id, LONG x, LONG y, LONG width, LONG height, Texture* pTexture = 0 );

		void setPercentValue( float value );

		float getPercentValue();

		void draw( ULONG deltaTime );

	protected:
		float mPercentValue;
};

struct AppStage
{
	~AppStage()
	{
		for( auto elem = interfaceObjects.begin(); elem != interfaceObjects.end(); elem++ )
		{
			s_delete( *elem );
		}
		interfaceObjects.clear();
	}

	UINT id;
	UINT backgroundId;
	std::vector< Sensor* > interfaceObjects;
	bool bDrawingWolrd;
};

//Класс приложения
class Application
{
	public:
		Application( HINSTANCE hInstance );
		~Application();

		bool init();

		void setFPSValue( UINT value );
		void setStage( UINT id );

		//Методы циклической обработки
		LRESULT inputProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		void process( ULONG deltaTime );
		void render( ULONG deltaTime );

		Texture* getTexture( UINT id );
		Background* getBackground( UINT id );

	private:
		UINT mFPS;
		HINSTANCE mhInstance;
		tstring mWinClassName;
		HWND mhWnd;
		Direct* mpDirectStruct;
		bool mbEnable;
		std::map< UINT, Texture* > mTextures;
		std::map< UINT, Background* > mBackgrounds;
		World* mpWorld;
		Character mCharacter;
		AppStage* mpCurrentStage;
		Sensor* mpTargetSensor;
		std::map< UINT, AppStage* > mStages;

		int createWindow();
		int createDirect();
		int createInterface();

		int testDevice();
		int reset();

		Texture* loadTextureFromFile( LPCTSTR pFileName, UINT id );
		Background* loadBackgroundFromFile( LPCTSTR pFileName, ULONG displayWidth, ULONG displayHeight, UINT id );

		int loadTextures( LPCTSTR pSourceFileName );

		void unloadTextures();

		void loadWorld( LPCTSTR pFileName );
		void unloadWold();

		void setTargetSensor( Sensor* pSensor );
};

Application* createApplication( HINSTANCE hInstance );

LRESULT APIENTRY inputWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );