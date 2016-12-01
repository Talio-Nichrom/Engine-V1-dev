#include "Engine.h"

using namespace std;

StatusBar::StatusBar( Direct* pDirect, ULONG id ) :
	Sensor( pDirect, id )
{

}

StatusBar::StatusBar( Direct* pDirect, ULONG id, LONG x, LONG y, LONG width, LONG height, Texture* pTexture ) :
	Sensor( pDirect, id, x, y, width, height, pTexture )
{

}


void StatusBar::setPercentValue( float value )
{
	if( value < 0.0f )
		value = 0.0f;
	else if( value > 1.0f )
		value = 1.0f;

	mPercentValue = value;
}


float StatusBar::getPercentValue()
{
	return mPercentValue;
}


void StatusBar::draw( ULONG deltaTime )
{
	if( mGraf.pTexture )
	{
		mGraf.textureRect.left = 0;
		mGraf.textureRect.right = mGraf.pTexture->textureInfo.Width;
		mGraf.textureRect.top = 0;
		mGraf.textureRect.bottom = mGraf.pTexture->textureInfo.Height / 2;

		mGraf.maskRect.left = 0;
		mGraf.maskRect.right = (LONG)( mGraf.pTexture->textureInfo.Width * mPercentValue );
		mGraf.maskRect.top = mGraf.pTexture->textureInfo.Height / 2;
		mGraf.maskRect.bottom = mGraf.pTexture->textureInfo.Height;
	}

	Sensor::draw( deltaTime );
}