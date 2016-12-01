#include "Engine.h"

using namespace std;

Sensor::Sensor( Direct* pDirect, ULONG id ) :
	Sensor( pDirect, id, 0, 0, 0, 0, 0 )
{

}

Sensor::Sensor( Direct* pDirect, ULONG id, LONG x, LONG y, LONG width, LONG height, Texture* pTexture ) :
	mpDirect( pDirect ),
	mId( id ),
	mWorkRect( { x, y, x + width, y + height } ),
	mbEnabled( true ),
	mbVisible( true )
{
	setTexture( pTexture );
}


void Sensor::setTexture( Texture* pTexture )
{
	mGraf.pTexture = pTexture;
}

void Sensor::setPosition( LONG x, LONG y )
{
	LONG width = mWorkRect.right - mWorkRect.left;
	LONG height = mWorkRect.bottom - mWorkRect.top;

	mWorkRect.left = x;
	mWorkRect.right = x + width;
	mWorkRect.top = y;
	mWorkRect.right = y + height;
}

void Sensor::setSize( LONG width, LONG height )
{
	mWorkRect.right = mWorkRect.left + width;
	mWorkRect.bottom = mWorkRect.top + height;
}

void Sensor::setWorkRect( RECT rect )
{
	mWorkRect = rect;
}

void Sensor::setVisible( bool bVisible )
{
	mbVisible = bVisible;
}

void Sensor::setEnabled( bool bEnabled )
{
	mbEnabled = bEnabled;
}


bool Sensor::isVisible()
{
	return mbVisible;
}

bool Sensor::isEnable()
{
	return mbEnabled;
}


bool Sensor::ptInSensor( POINT pt )
{
	if( mbVisible )
		return PtInRect( &mWorkRect, pt );

	return false;
}


void Sensor::onMouseEnter()
{

}

void Sensor::onMouseLeave()
{

}

void Sensor::onEvent( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{

}


void Sensor::draw( ULONG deltaTime )
{
	if( mbVisible )
		mGraf.draw( mpDirect, { (float)mWorkRect.left, (float)mWorkRect.top, 0.0f }, mWorkRect.right - mWorkRect.left, mWorkRect.bottom - mWorkRect.top );
}