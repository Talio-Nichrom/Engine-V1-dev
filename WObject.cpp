#include "Engine.h"

using namespace std;

WObject::WObject( Direct* pDirect, World* pWorld ) :
	mpDirect( pDirect ),
	mpWorld( pWorld ),
	mpTask( 0 )
{
	mBaseRatio = 1.0f;
	mSpeed = 1.0f;
	mPosition = { 0.0f, 0.0f };
	mMoveVector = { 0.0f, 0.0f };
	mOrientationAngle = 0.0f;
}

WObject::WObject( const WObject& root )
{
	mpDirect = root.mpDirect;
	mpWorld = root.mpWorld;
	mName = root.mName;
	mPosition = { 0.0f, 0.0f };
	mBaseRatio = root.mBaseRatio;
	mSpeed = root.mSpeed;
	mAnimations = root.mAnimations;
	mColor = root.mColor;
	mMoveVector = { 0.0f, 0.0f };
	mOrientationAngle = 0.0f;
	if( root.mpTask )
		mpTask = root.mpTask->clone();
	startAnimation( _T( "Stay" ) );
}

WObject& WObject::operator=( const WObject& root )
{
	mpDirect = root.mpDirect;
	mpWorld = mpWorld;
	mName = root.mName;
	mPosition = { 0.0f, 0.0f };
	mBaseRatio = root.mBaseRatio;
	mSpeed = root.mSpeed;
	mAnimations = root.mAnimations;
	mColor = root.mColor;
	mMoveVector = { 0.0f, 0.0f };
	mOrientationAngle = 0.0f;
	if( root.mpTask )
		mpTask = root.mpTask->clone();
	startAnimation( _T( "Stay" ) );

	return *this;
}

WObject::~WObject()
{
	mAnimations.clear();
	s_delete( mpTask );
}


void WObject::setName( LPCTSTR pName )
{
	mName = pName;
}

void WObject::setBaseRatio( float baseRatio )
{
	mBaseRatio = baseRatio;
}

void WObject::setSpeed( float speed )
{
	mSpeed = speed;
}

void WObject::setAnimation( LPCTSTR pKey, const Animation& anim )
{
	mAnimations[ pKey ] = anim;
}

void WObject::setPosition( float x, float y )
{
	mPosition.x = x;
	mPosition.y = y;
}

void WObject::setPositionRel( float x, float y )
{
	mPosition.x += x;
	mPosition.y += y;
}

void WObject::setColor( DWORD color )
{
	mColor = color;
}

void WObject::setTask( Task* pTask )
{
	mpTask = pTask;
	if( mpTask )
		mpTask->setObject( this );
}


void WObject::startAnimation( LPCTSTR pKey )
{
	mCurrentAnimation = &mAnimations[ pKey ];
	mCurrentAnimation->setOrientation( mOrientationAngle );
	mCurrentAnimation->start();
	mTime = 0;
}


LPCTSTR WObject::getName()
{
	return mName.c_str();
}

POINTFLOAT WObject::getPosition()
{
	return mPosition;
}

float WObject::getBaseRatio()
{
	return mBaseRatio;
}

float WObject::getSpeed()
{
	return mSpeed;
}

bool WObject::isMoving()
{
	if( mMoveVector.x == 0.0f && mMoveVector.y == 0.0f )
		return false;
	return true;
}


void WObject::process( ULONG deltaTime )
{
	if( mpTask )
		mpTask->process( deltaTime );

	mTime += deltaTime;

	if( mTime >= 200.0f / getSpeed() )
	{
		mTime = 0;
		mCurrentAnimation->nextState();
	}

	if( mMoveVector.x != 0.0f && mMoveVector.y != 0.0f )
	{
		POINTFLOAT newPosition;
		newPosition.x = 0.01f * getSpeed() * sin( mOrientationAngle );
		newPosition.y = - 0.01f * getSpeed() * cos( mOrientationAngle );

		if( abs( newPosition.x ) >= abs( mMoveVector.x ) )
		{
			newPosition.x = mMoveVector.x;
			mMoveVector.x = 0.0f;
		}
		else
		{
			mMoveVector.x -= newPosition.x;
		}

		if( abs( newPosition.y ) >= abs( mMoveVector.y ) )
		{
			newPosition.x = mMoveVector.y;
			mMoveVector.y = 0.0f;
		}
		else
		{
			mMoveVector.y -= newPosition.y;
		}

		mpWorld->testMoving( mPosition, mBaseRatio, newPosition );
		setPositionRel( newPosition.x, newPosition.y );

		if( mMoveVector.x == 0.0f && mMoveVector.y == 0.0f )
		{
			startAnimation( _T( "Stay" ) );
		}
	}
}

void WObject::draw( POINTFLOAT camera )
{
	if( !mCurrentAnimation )
		return;

	int widthRel = (int)( VIEW_FIELD_WIDTH / 2 * mBaseRatio );
	int heightRel = (int) ( 3 * VIEW_FIELD_WIDTH / 4 * mBaseRatio );

	POINT displaySize = { mpDirect->width, mpDirect->height };

	POINT pos = worldToView( displaySize, camera, mPosition );

	float x = pos.x - widthRel;
	float y = pos.y - heightRel;

	D3DXVECTOR3 position( x, y, 0.0f );

	Graf* pGraf = mCurrentAnimation->getCurrentState();
	pGraf->textureColor = mColor;
	mCurrentAnimation->getCurrentState()->draw( mpDirect, position, (ULONG) ( VIEW_FIELD_WIDTH * mBaseRatio ), (ULONG) ( VIEW_FIELD_WIDTH * mBaseRatio ) );

	RECT fontRect = { pos.x - 200, y, pos.x + 200, y + 20 };
	mpDirect->pFontItems->DrawText( 0, mName.c_str(), -1, &fontRect, DT_CENTER, 0xffffffff );

}


void WObject::moveTo( POINTFLOAT vector )
{
	if( abs( vector.x ) < 0.001f && abs( vector.y ) < 0.001f )
	{
		vector.x = -0.001f;
		vector.y = -0.001f;
	}

	mMoveVector = vector;
	if( mCurrentAnimation && tstring( mCurrentAnimation->getName() ) != _T( "Move" ) )
	{
		startAnimation( _T( "Move" ) );
	}

	mOrientationAngle = northAngle( vector );
	mCurrentAnimation->setOrientation( mOrientationAngle );
}

void WObject::moveToPosition( POINTFLOAT target )
{
	moveTo( { target.x - mPosition.x, target.y - mPosition.y } );
}