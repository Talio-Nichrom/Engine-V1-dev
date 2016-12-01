#include "Engine.h"

using namespace std;

Character::Character()
{
}

Character::~Character()
{

}


void Character::moveToPosition( float x, float y )
{
	if( mpManagedObject )
		mpManagedObject->moveToPosition( { x, y } );
}

void Character::setPosition( float x, float y )
{
	if( mpManagedObject )
		mpManagedObject->setPosition( x, y );
}

void Character::setManagedObject( WObject* pObject )
{
	mpManagedObject = pObject;
}


POINTFLOAT Character::getPosition()
{
	if( mpManagedObject )
		return mpManagedObject->getPosition();
	return{ 0.0f, 0.0f };
}

void Character::test()
{
	if( mpManagedObject )
		mpManagedObject->startAnimation( _T( "Move" ) );
}