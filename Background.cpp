#include "Engine.h"

using namespace std;

int Background::draw( Direct* pDirect )
{
	if( FAILED( pDirect->pDevice->UpdateSurface( pBackgroundSurface, NULL, pDirect->pBackBuffer, NULL ) ) )
		return 1;

	return 0;
}