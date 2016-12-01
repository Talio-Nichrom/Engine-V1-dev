#include "Engine.h"

using namespace std;

Button::Button( Direct* pDirect, ULONG id ) :
	Sensor( pDirect, id )
{

}

Button::Button( Direct* pDirect, ULONG id, LONG x, LONG y, LONG width, LONG height, Texture* pTexture ) :
	Sensor( pDirect, id, x, y, width, height, pTexture )
{

}


void Button::onMouseEnter()
{
	mbActive = true;
}

void Button::onMouseLeave()
{
	mbActive = false;
}

void Button::onEvent( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		case WM_LBUTTONDOWN:
		case WM_LBUTTONDBLCLK:
		{
			mbPushed = true;
		}
		break;
		case WM_LBUTTONUP:
		{
			if( mbPushed )
			{
				SendMessage( hWnd, WM_BUTTONCLICK, mId, 0 );
				mbPushed = false;
			}
		}
		break;
	}
}


void Button::draw( ULONG deltaTime )
{
	if( mGraf.pTexture )
	{
		mGraf.textureRect.left = 0;
		mGraf.textureRect.right = mGraf.pTexture->textureInfo.Width;

		if( !mbEnabled )
		{
			mGraf.textureRect.top = mGraf.pTexture->textureInfo.Height * 3 / 4;
			mGraf.textureRect.bottom = mGraf.pTexture->textureInfo.Height;
		}
		else if( mbPushed )
		{
			mGraf.textureRect.top = mGraf.pTexture->textureInfo.Height / 2;
			mGraf.textureRect.bottom = mGraf.pTexture->textureInfo.Height * 3 / 4;
		}
		else if( mbActive )
		{
			mGraf.textureRect.top = mGraf.pTexture->textureInfo.Height / 4;
			mGraf.textureRect.bottom = mGraf.pTexture->textureInfo.Height / 2;
		}
		else
		{
			mGraf.textureRect.top = 0;
			mGraf.textureRect.bottom = mGraf.pTexture->textureInfo.Height / 4;
		}
	}

	Sensor::draw( deltaTime );
}