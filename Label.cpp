#include "Engine.h"

using namespace std;

Label::Label( Direct* pDirect, ULONG id, LPCTSTR pText, DWORD fontColor ) :
	Sensor( pDirect, id ),
	mText( pText ),
	mFontColor( fontColor ),
	mFontStyle( DT_LEFT | DT_TOP )
{

}

Label::Label( Direct* pDirect, ULONG id, LPCTSTR pText, LONG x, LONG y, LONG width, LONG height, DWORD fontColor, Texture* pTexture ) :
	Sensor( pDirect, id, x, y, width, height, pTexture ),
	mText( pText ),
	mFontColor( fontColor ),
	mFontStyle( DT_LEFT | DT_TOP )
{

}


void Label::setText( LPCTSTR pText )
{
	mText = pText;
}

void Label::setFontColor( DWORD color )
{
	mFontColor = color;
}

void Label::setFontStyle( DWORD style )
{
	mFontStyle = style;
}


LPCTSTR Label::getText()
{
	return mText.c_str();
}

DWORD Label::getFontColor()
{
	return mFontColor;
}


void Label::draw( ULONG deltaTime )
{
	if( mGraf.pTexture )
	{
		mGraf.textureRect.left = 0;
		mGraf.textureRect.top = 0;
		mGraf.textureRect.right = mGraf.pTexture->textureInfo.Width;
		mGraf.textureRect.bottom = mGraf.pTexture->textureInfo.Height;
	}
	Sensor::draw( deltaTime );
	mpDirect->pFontInterface->DrawText( NULL, mText.c_str(), -1, &mWorkRect, mFontStyle, mFontColor );
}