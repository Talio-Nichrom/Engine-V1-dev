#include "Engine.h"

using namespace std;

MoveTask::MoveTask( POINTFLOAT center ) :
	mCenter( center )
{

}

void MoveTask::setCenter( POINTFLOAT center )
{
	mCenter = center;
}

void MoveTask::process( ULONG deltaTime )
{
	if( !mpWObject )
		return;

	if( !mpWObject->isMoving() )
	{
		if( mTime == 0 )
		{
			srand( timeGetTime() + rand() );

			float angle = D3DXToRadian( float( rand() % 720 ) + float( rand() % ( rand() % 125233 ) ) );
			float radius = 2.0f;
			float S = ( float( rand() % int( 1000.0f * radius ) + 100.0f ) / 1000.0f );

			mpWObject->moveToPosition( { mCenter.x + S * cos( angle ), mCenter.y + S * sin( angle ) } );
			mTime = (ULONG) ( ( rand() % 1000 + 500 ) / mpWObject->getSpeed() );
		}
		else
		{
			if( deltaTime < mTime )
				mTime -= deltaTime;
			else
				mTime = 0;	
		}
	}
}

MoveTask* MoveTask::clone()
{
	MoveTask* pResult = new MoveTask( mCenter );

	return pResult;
}