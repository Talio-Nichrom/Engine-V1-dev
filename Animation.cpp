#include "Engine.h"

using namespace std;

Animation::Animation()
{
	mName = _T( "Empty" );
	mGraf.pTexture = 0;
	mGraf.textureRect.left = 0;
	mGraf.textureRect.right = 0;
	mGraf.textureRect.top = 0;
	mGraf.textureRect.bottom = 0;
	mGraf.textureColor = 0xffffffff;
	mFrameSize = 0;
	mOrientationType = 0;
	mTopOffset = 0;
}

Animation::Animation( LPCTSTR pName, Texture* pTexture, UINT frameSize, UINT orientationType, UINT topOffset )
{
	mName = pName;
	mGraf.pTexture = pTexture;
	mGraf.textureRect.left = 0;
	mGraf.textureRect.right = frameSize;
	mGraf.textureRect.top = 0;
	mGraf.textureRect.bottom = frameSize;
	mGraf.textureColor = 0xffffffff;
	mFrameSize = frameSize;
	mOrientationType = orientationType;
	mTopOffset = topOffset;
}


void Animation::addState( UINT animKey )
{
	mStates.push_back( animKey );
}


void Animation::setCurrentState( UINT id )
{
	mCurrentState = id;
	UINT state = mStates[ mCurrentState ];
	mGraf.textureRect.left = mFrameSize * state;
	mGraf.textureRect.right = mGraf.textureRect.left + mFrameSize;
}

void Animation::setOrientation( float x, float y )
{
	setOrientation( northAngle( { x, y } ) );
}

void Animation::setOrientation( POINTFLOAT ptf )
{
	setOrientation( ptf.x, ptf.y );
}

void Animation::setOrientation( float angle )
{
	int zone = (int)( D3DXToDegree( angle ) / 22.5f );
	int pos = mTopOffset;

	switch( mOrientationType )
	{
		case 2:
			{
				if( ( zone >= 2 && zone <= 5 ) || ( zone >= 10 && zone <= 13 ) )
				{
					pos += 1;
				}
			}
			break;
		case 3:
			{
				if( zone >= 2 && zone <= 5 )
				{
					pos += 1;
				}
				else if( zone >= 6 && zone <= 9 )
				{
					pos += 2;
				}
				else if( zone >= 10 && zone <= 13 )
				{
					pos += 3;
				}
			}
			break;
		case 4:
			{
				if( zone == 1 || zone == 2 || zone == 9 || zone == 10 )
				{
					pos += 1;
				}
				else if( zone == 3 || zone == 4 || zone == 11 || zone == 12 )
				{
					pos += 2;
				}
				else if( zone == 5 || zone == 6 || zone == 13 || zone == 14 )
				{
					pos += 3;
				}
			}
			break;
		case 5:
			{
				if( zone >= 1 && zone <= 14 )
				{
					pos += ( zone + 1 ) / 2;
				}
			}
			break;
		default:
			break;
	}

	mGraf.textureRect.top = mFrameSize * pos;
	mGraf.textureRect.bottom = mGraf.textureRect.top + mFrameSize;
}


Graf* Animation::getCurrentState()
{
	return &mGraf;
}

UINT Animation::getStateCount()
{
	return mStates.size();
}

UINT Animation::getFrameSize()
{
	return mFrameSize;
}

LPCTSTR Animation::getName()
{
	return mName.c_str();
}


void Animation::start()
{
	setCurrentState( 0 );
}

void Animation::process( float x, float y )
{

}

void Animation::nextState()
{
	mCurrentState++;
	if( mCurrentState >= mStates.size() )
		mCurrentState = 0;
	setCurrentState( mCurrentState );
}