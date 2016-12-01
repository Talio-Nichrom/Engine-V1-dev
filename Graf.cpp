#include "Engine.h"

using namespace std;

int Graf::draw( Direct* pDirect, D3DXVECTOR3 position, ULONG width, ULONG height )
{
	pDirect->pSprite->Begin( 0 );

	D3DXMATRIX tramsformMatrix;
	float kX = (float) ( width ) / (float) ( textureRect.right - textureRect.left );
	float kY = (float) ( height ) / (float) ( textureRect.bottom - textureRect.top );
	float kZ = 1.0f;

	D3DXMatrixScaling( &tramsformMatrix, kX, kY, 1.0f );
	D3DXVECTOR3 vec( position.x / kX, position.y / kY, position.z / kZ );

	if( FAILED( pDirect->pSprite->SetTransform( &tramsformMatrix ) ) )
		return 1;

	if( FAILED( pDirect->pSprite->Draw( pTexture->pDirectTexture, &maskRect, NULL, &vec, maskColor ) ) )
		return 2;

	if( FAILED( pDirect->pSprite->Draw( pTexture->pDirectTexture, &textureRect, NULL, &vec, textureColor ) ) )
		return 3;

	pDirect->pSprite->End();

	return 0;
}