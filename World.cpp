#include "Engine.h"

using namespace std;

World::World( Direct* pDirect ):
	mpDirect( pDirect ),
	mMapWidth( 0 ),
	mMapHeight( 0 ),
	mMap( 0 ),
	mCameraPosition( { 5.5f, 5.5f } )
{
	//loadWorldObjects();
}

World::~World()
{
	clear();

	for( auto elem = mMemWObjects.begin(); elem != mMemWObjects.end(); elem++ )
	{
		s_delete( elem->second );
	}
	mMemWObjects.clear();

	for( auto elem = mMemTextures.begin(); elem != mMemTextures.end(); elem++ )
	{
		s_delete( elem->second );
	}
	mMemTextures.clear();
}


void World::process( ULONG deltaTime )
{
	mVisibleWObjects.clear();

	for( auto elem = mWorldObjects.begin(); elem != mWorldObjects.end(); elem++ )
	{
		auto pWObject = ( *elem );
		if( testVisible( pWObject->getPosition() ) )
		{
			pWObject->process( deltaTime );
			mVisibleWObjects.push_back( pWObject );
		}
	}

	mVisibleWObjects.shrink_to_fit();
	sort( mVisibleWObjects.begin(), mVisibleWObjects.end(), sortWObjects );
}

void World::draw( ULONG deltaTime )
{
	int xRel = VIEW_FIELD_WIDTH / 2;

	int centerFieldX = (int) ( mCameraPosition.x );
	int centerFieldY = (int) ( mCameraPosition.y );

	//Отрисовка поверхности карты в диапазоне, заданном VIEW_MAP_SIZE
	for( int i = -VIEW_MAP_SIZE / 2; i <= VIEW_MAP_SIZE / 2; i++ )
	{
		for( int j = -VIEW_MAP_SIZE / 2; j <= VIEW_MAP_SIZE / 2; j++ )
		{
			Field* pField = getMapField( POINT{ j + centerFieldX , i + centerFieldY } );

			POINT displaySize = { mpDirect->width, mpDirect->height };
			POINT viewPos = worldToView( displaySize, mCameraPosition, POINTFLOAT{ (float)( j + centerFieldX ) , (float)( i + centerFieldY ) } );

			D3DXVECTOR3 position( viewPos.x - xRel, viewPos.y, 0.0f );
			pField->graf.draw( mpDirect, position, VIEW_FIELD_WIDTH, VIEW_FIELD_HEIGHT );
		}
	}

	for( auto elem = mVisibleWObjects.begin(); elem != mVisibleWObjects.end(); elem++ )
	{
		( *elem )->draw( mCameraPosition );
	}
}


void World::loadWorldObjects()
{
	convert_type* converter = new convert_type();
	locale utf8( locale::empty(), converter );

	tifstream file( _T( "Data/Objects.txt" ) );
	file.imbue( utf8 );

	tstring key;

	while( !file.eof() )
	{
		file >> key;
		if( key == _T( "<Object>" ) )
		{
			UINT id = 0;
			WObject* wObject = new WObject( mpDirect, this );
			DWORD color = 0xffffffff;

			do
			{
				file >> key;
				if( key == _T( "<Id>" ) )
				{
					file >> id;
				}
				else if( key == _T( "<Parent>" ) )
				{
					UINT parent;
					file >> parent;
					WObject* parentObject = mMemWObjects[ parent ];
					if( parentObject )
						*wObject = *mMemWObjects[ parent ];
				}
				else if( key == _T( "<Name>" ) )
				{
					tstring name;
					file.get();
					getline( file, name );
					wObject->setName( name.c_str() );
				}
				else if( key == _T( "<ColorARGB>" ) )
				{
					UINT a = 0, r = 0, g = 0, b = 0;
					file >> a;
					file >> r;
					file >> g;
					file >> b;
					color = D3DCOLOR_ARGB( a, r, g, b );
					wObject->setColor( color );
				}
				else if( key == _T( "<Animation>" ) )
				{
					tstring animName;
					file >> animName;
				
					tstring textureFileName;
					file >> textureFileName;
					Texture* pTexture = mMemTextures[ textureFileName ];
					if( !pTexture )
					{
						pTexture = createTextureFromFile( mpDirect, textureFileName.c_str() );
						mMemTextures.emplace( textureFileName, pTexture );
					}
					UINT frameSize;
					file >> frameSize;
					UINT orientationType;
					file >> orientationType;
					UINT topOffset;
					file >> topOffset;

					Animation anim( animName.c_str(), pTexture, frameSize, orientationType, topOffset );

					UINT keyCount;
					file >> keyCount;

					for( UINT i = 0; i < keyCount; i++ )
					{
						UINT animKey;
						file >> animKey;
						anim.addState( animKey );
					}

					wObject->setAnimation( animName.c_str(), anim );

				}
				else if( key == _T( "<WProp>" ) )
				{
					float wProp;
					file >> wProp;
					wObject->setBaseRatio( wProp );
				}
				else if( key == _T( "<Speed>" ) )
				{
					float speed;
					file >> speed;
					wObject->setSpeed( speed );
				}
			} while( key != _T( "</Object>" ) );

			mMemWObjects.emplace( id, wObject );
		}
	}

	file.close();
}

void World::loadWorld( LPCTSTR pFileName )
{
	clear();

	convert_type* converter = new convert_type();
	locale utf8( locale::empty(), converter );

	tifstream file( pFileName );
	file.imbue( utf8 );

	tstring key;

	while( !file.eof() )
	{
		file >> key;
		if( key == _T( "<Field>" ) )
		{
			Field* pNewField = new Field;
			UINT id = 0;
			file >> id;
			file >> pNewField->move;
			tstring textureFile;
			file >> textureFile;
			pNewField->pTexture = createTextureFromFile( mpDirect, textureFile.c_str() );

			pNewField->graf.pTexture = pNewField->pTexture;
			pNewField->graf.textureRect.right = pNewField->pTexture->textureInfo.Width;
			pNewField->graf.textureRect.top = pNewField->pTexture->textureInfo.Height / 2;
			pNewField->graf.textureRect.bottom = pNewField->pTexture->textureInfo.Height;

			mMemFields.emplace( id, pNewField );
		}
		else if( key == _T( "<Location>" ) )
		{
			Location* pNewLocation = new Location;
			UINT id = 0;
			file >> id;
			getline( file, pNewLocation->name );
			getline( file, pNewLocation->name );
			file >> pNewLocation->startX;
			file >> pNewLocation->startY;
			file >> pNewLocation->width;
			file >> pNewLocation->height;

			pNewLocation->map = new UINT*[ pNewLocation->height ];
			for( ULONG i = 0; i < pNewLocation->height; i++ )
				pNewLocation->map[ i ] = new UINT[ pNewLocation->width ];

			for( ULONG i = 0; i < pNewLocation->height; i++ )
			{
				for( ULONG j = 0; j < pNewLocation->width; j++ )
				{
					file >> pNewLocation->map[ i ][ j ];
				}
			}

			mMemLocations.emplace( id, pNewLocation );

			if( mMapWidth < pNewLocation->startX + pNewLocation->width )
				mMapWidth = pNewLocation->startX + pNewLocation->width;
			if( mMapHeight < pNewLocation->startY + pNewLocation->height )
				mMapHeight = pNewLocation->startY + pNewLocation->height;

			while( key != _T( "</Location>" ) )
			{
				file >> key;
				if( key == _T( "<Spawn>" ) )
				{
					SpawnPoint sp;
					file >> sp.objectId;
					file >> sp.position.x;
					file >> sp.position.y;
					file >> sp.taskId;
					sp.position.x += (float)pNewLocation->startX;
					sp.position.y += (float)pNewLocation->startY;
					mSpawnPoints.push_back( sp );
				}
				else if( key == _T( "<Character>" ) )
				{
					file >> mCharacterSpawnPoint.objectId;
					file >> mCharacterSpawnPoint.position.x;
					file >> mCharacterSpawnPoint.position.y;
					file >> mCharacterSpawnPoint.taskId;
					mCharacterSpawnPoint.position.x += (float) pNewLocation->startX;
					mCharacterSpawnPoint.position.y += (float) pNewLocation->startY;
				}
			}
		}
	}

	file.close();

	initMap();
	spawnAll();
}

void World::initMap()
{
	mMap = new MapField*[ mMapHeight ];
	for( ULONG i = 0; i < mMapHeight; i++ )
	{
		mMap[ i ] = new MapField[ mMapWidth ];
		for( ULONG j = 0; j < mMapWidth; j++ )
		{
			mMap[ i ][ j ].pField = mMemFields[ 0 ];
			mMap[ i ][ j ].pLocation = 0;
		}
	}

	for( auto elem = mMemLocations.begin(); elem != mMemLocations.end(); elem++ )
	{
		Location* location = elem->second;
		for( ULONG i = 0; i < location->height; i++ )
		{
			for( ULONG j = 0; j < location->width; j++ )
			{
				mMap[ i + location->startY ][ j + location->startX ].pLocation = location;
				mMap[ i + location->startY ][ j + location->startX ].pField = mMemFields[ location->map[ i ][ j ] ];
			}
		}
	}
}

void World::spawnAll()
{
	for( auto elem = mSpawnPoints.begin(); elem != mSpawnPoints.end(); elem++ )
	{
		spawn( elem->objectId, elem->position, elem->taskId );
	}
}

WObject* World::spawn( UINT objectId, POINTFLOAT position, UINT taskId )
{
	WObject* pWObject = new WObject( *mMemWObjects[ objectId ] );
	pWObject->setPosition( position.x, position.y );

	if( taskId == 1 )
	{
		MoveTask* pTask = new MoveTask( position );
		pWObject->setTask( pTask );
	}

	mWorldObjects.push_back( pWObject );

	return pWObject;
}

WObject* World::spawn( SpawnPoint sPoint )
{
	return spawn( sPoint.objectId, sPoint.position, sPoint.taskId );
}


WObject* World::spawnCharacter( POINTFLOAT position )
{
	return spawn( mCharacterSpawnPoint );
}

void World::clear()
{
	for( auto elem = mMemFields.begin(); elem != mMemFields.end(); elem++ )
	{
		s_delete( elem->second );
	}
	mMemFields.clear();

	for( auto elem = mMemLocations.begin(); elem != mMemLocations.end(); elem++ )
	{
		s_delete( elem->second );
	}
	mMemLocations.clear();

	for( auto elem = mWorldObjects.begin(); elem != mWorldObjects.end(); elem++ )
	{
		s_delete( ( *elem ) );
	}
	mWorldObjects.clear();

	mSpawnPoints.clear();


	for( UINT i = 0; i < mMapHeight; i++ )
	{
		s_deleteA( mMap[i] );
	}
	s_deleteA( mMap );
	mMapHeight = 0;
	mMapWidth = 0;
}

void World::setCameraPosition( POINTFLOAT pf )
{
	mCameraPosition = pf;
}

void World::setCameraPositionRel( POINTFLOAT pf )
{
	mCameraPosition.x += pf.x;
	mCameraPosition.y += pf.y;
}


Field* World::getMapField( POINT coords )
{
	Field* pResult = 0;

	if( coords.x < 0 || coords.x >= (LONG)mMapWidth || coords.y < 0 || coords.y >= (LONG)mMapHeight )
		pResult =  mMemFields[ 0 ];
	else
		pResult = mMap[ coords.y ][ coords.x ].pField;

	return pResult;
}

Field* World::getMapField( POINTFLOAT coords )
{
	return getMapField( POINT { (int) ( coords.x ), (int) ( coords.y ) } );
}

Location* World::getMapLocation( POINT coords )
{
	Location* pResult = 0;

	if( coords.x < 0 || coords.x >= (LONG) mMapWidth || coords.y < 0 || coords.y >= (LONG) mMapHeight )
		pResult = 0;
	else
		pResult = mMap[ coords.y ][ coords.x ].pLocation;

	return pResult;
}

Location* World::getMapLocation( POINTFLOAT coords )
{
	return getMapLocation( POINT{ (int) ( coords.x ), (int) ( coords.y ) } );
}

POINTFLOAT World::getCameraPosition()
{
	return mCameraPosition;
}

LPCTSTR World::getCurrenLocationName()
{
	Location* pLocation = getMapLocation( mCameraPosition );

	if( pLocation )
		return pLocation->name.c_str();

	return _T( "" );
}


void World::testMoving( POINTFLOAT position, float size, POINTFLOAT& rel )
{
	//Возможен другой алгоритм
	POINTFLOAT newPosition = { position.x + rel.x, position.y + rel.y };

	POINTFLOAT cornerPoint;
	POINTFLOAT xPoint;
	POINTFLOAT yPoint;

	float xBorder, yBorder;

	if( rel.x >= 0.0f && rel.y >= 0.0f )
	{
		cornerPoint = { newPosition.x + size / 2.0f, newPosition.y + size / 2.0f };
		xPoint = { newPosition.x + size / 2.0f, newPosition.y - size / 2.0f };
		yPoint = { newPosition.x - size / 2.0f, newPosition.y + size / 2.0f };
		xBorder = (float)( (int)( position.x ) + 1 );
		yBorder = (float)( (int)( position.y ) + 1 );
	}
	else if( rel.x >= 0.0f && rel.y < 0.0f )
	{
		cornerPoint = { position.x + size / 2.0f, position.y - size / 2.0f };
		xPoint = { newPosition.x + size / 2.0f, newPosition.y + size / 2.0f };
		yPoint = { newPosition.x - size / 2.0f, newPosition.y - size / 2.0f };
		xBorder = (float)( (int)( position.x ) + 1 );
		yBorder = (float)( (int)( position.y ) );
	}
	else if( rel.x < 0.0f && rel.y >= 0.0f )
	{
		cornerPoint = { position.x - size / 2.0f, position.y + size / 2.0f };
		xPoint = { newPosition.x - size / 2.0f, newPosition.y - size / 2.0f };
		yPoint = { newPosition.x + size / 2.0f, newPosition.y + size / 2.0f };
		xBorder = (float)( (int)( position.x ) );
		yBorder = (float)( (int)( position.y ) + 1 );
	}
	else
	{
		cornerPoint = { position.x - size / 2.0f, position.y - size / 2.0f };
		xPoint = { newPosition.x - size / 2.0f, newPosition.y + size / 2.0f };
		yPoint = { newPosition.x + size / 2.0f, newPosition.y - size / 2.0f };
		xBorder = (float)( (int)( position.x ) );
		yBorder = (float)( (int)( position.y ) );
	}

	float cornerPointMove = getMapField( cornerPoint )->move;
	float xPointMove = getMapField( xPoint )->move;
	float yPointMove = getMapField( yPoint )->move;

	if( xPointMove == 0.0f || yPointMove == 0.0f )
	{
		if( xPointMove == 0.0f )
		{
			rel.x -= xPoint.x - xBorder;
		}
		
		if( yPointMove == 0.0f )
		{
			rel.y -= yPoint.y - yBorder;
		}
	}
	else if( cornerPointMove == 0.0f )
	{
		float xT = abs( ( xPoint.x - xBorder ) / rel.x );
		float yT = abs( ( yPoint.y - yBorder ) / rel.y );

		if( xT > yT )
		{
			rel.y -= yPoint.y - yBorder;
		}
		else if( xT < yT )
		{
			rel.x -= xPoint.x - xBorder;
		}
		else
		{
			rel.x -= xPoint.x - xBorder;
			rel.y -= yPoint.y - yBorder;
		}
	}

}

bool World::testVisible( POINTFLOAT position )
{
	if( position.x - mCameraPosition.x < (float) ( -VIEW_MAP_SIZE / 2 ) || position.x - mCameraPosition.x >( float ) ( VIEW_MAP_SIZE / 2 ) )
		return false;

	if( position.y - mCameraPosition.y < (float) ( -VIEW_MAP_SIZE / 2 ) || position.y - mCameraPosition.y >( float ) ( VIEW_MAP_SIZE / 2 ) )
		return false;

	return true;
}